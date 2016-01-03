/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gusb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define USBASYNC_MAX_DEVICES 256

#define USBASYNC_OUT_TIMEOUT 20 // milliseconds

#define USBASYNC_DEFAULT_TIMEOUT 1000 // milliseconds

#define IS_ENDPOINT_IN(endpoint) ((endpoint & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
#define IS_ENDPOINT_OUT(endpoint) ((endpoint & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT)
#define IS_ENDPOINT_INTERRUPT(endpoint) ((endpoint & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_INTERRUPT)
#define IS_ENDPOINT_BULK(endpoint) ((endpoint & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK)
#define IS_ENDPOINT_ISOCHRONOUS(endpoint) ((endpoint & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)

#define INVALID_ENDPOINT_INDEX 0xff

#define DEFAULT_STRING_BUFFER_SIZE 255

static struct {
  char * path;
  libusb_device_handle * devh;
  s_usb_descriptors descriptors;
  struct {
    struct {
      unsigned char type;
      unsigned short size;
    } in;
    struct {
      unsigned char type;
      unsigned short size;
    } out;
  } endpoints[LIBUSB_ENDPOINT_ADDRESS_MASK];
  struct {
    int user;
    USBASYNC_READ_CALLBACK fp_read;
    USBASYNC_WRITE_CALLBACK fp_write;
    USBASYNC_CLOSE_CALLBACK fp_close;
  } callback;
  int pending_transfers;
  int closing;
} usbdevices[USBASYNC_MAX_DEVICES] = { };

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
static const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

static void print_error_libusb(const char * file, int line, const char * func, const char * libusbfunc, int ret) {

  fprintf(stderr, "%s:%d %s: %s failed with error: %s\n", file, line, func, libusbfunc, libusb_strerror(ret));
}
#define PRINT_ERROR_LIBUSB(libusbfunc,ret) print_error_libusb(__FILE__, __LINE__, __func__, libusbfunc, ret);

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_INVALID_ENDPOINT(msg, endpoint) fprintf(stderr, "%s:%d %s: %s: 0x%02x\n", __FILE__, __LINE__, __func__, msg, endpoint);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_TRANSFER_ERROR(transfer) fprintf(stderr, "libusb_transfer failed with status %s (endpoint=0x%02x)\n", libusb_error_name(transfer->status), transfer->endpoint);

static libusb_context* ctx = NULL;

static struct libusb_transfer ** transfers = NULL;
static unsigned int transfers_nb = 0;

static int add_transfer(struct libusb_transfer * transfer) {
  unsigned int i;
  for (i = 0; i < transfers_nb; ++i) {
    if (transfers[i] == transfer) {
      return 0;
    }
  }
  void * ptr = realloc(transfers, (transfers_nb + 1) * sizeof(*transfers));
  if (ptr) {
    transfers = ptr;
    transfers[transfers_nb] = transfer;
    transfers_nb++;
    usbdevices[(unsigned long) transfer->user_data].pending_transfers++;
    return 0;
  } else {
    PRINT_ERROR_ALLOC_FAILED("realloc")
    return -1;
  }
}

static void remove_transfer(struct libusb_transfer * transfer) {
  unsigned int i;
  for (i = 0; i < transfers_nb; ++i) {
    if (transfers[i] == transfer) {
      memmove(transfers + i, transfers + i + 1, (transfers_nb - i - 1) * sizeof(*transfers));
      transfers_nb--;
      void * ptr = realloc(transfers, transfers_nb * sizeof(*transfers));
      if (ptr || !transfers_nb) {
        transfers = ptr;
      } else {
        PRINT_ERROR_ALLOC_FAILED("realloc")
      }
      usbdevices[(unsigned long) transfer->user_data].pending_transfers--;
      free(transfer->buffer);
      libusb_free_transfer(transfer);
      break;
    }
  }
}

void usbasync_init(void) __attribute__((constructor (101)));
void usbasync_init(void) {
  int ret = libusb_init(&ctx);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_init", ret)
    exit(-1);
  }
}

void usbasync_clean(void) __attribute__((destructor (101)));
void usbasync_clean(void) {
  int i;
  for (i = 0; i < USBASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].devh != NULL) {
      gusb_close(i);
    }
  }
  libusb_exit(ctx);
}

static inline int usbasync_check_device(int device, const char * file, unsigned int line, const char * func) {
  if (device < 0 || device >= USBASYNC_MAX_DEVICES) {
    fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
    return -1;
  }
  if (usbdevices[device].devh == NULL) {
    fprintf(stderr, "%s:%d %s: no such device\n", file, line, func);
    return -1;
  }
  return 0;
}
#define USBASYNC_CHECK_DEVICE(device,retValue) \
  if(usbasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static inline unsigned char get_endpoint(int device, unsigned char endpoint, unsigned char direction, unsigned int count,
    const char * file, unsigned int line, const char * func) {

  if ((endpoint & LIBUSB_ENDPOINT_DIR_MASK) != direction) {

    PRINT_ERROR_INVALID_ENDPOINT("wrong direction for endpoint", endpoint)
    return INVALID_ENDPOINT_INDEX;
  }
  
  unsigned char endpointIndex = (endpoint & LIBUSB_ENDPOINT_ADDRESS_MASK);
    
  if (endpointIndex == 0) {

    PRINT_ERROR_INVALID_ENDPOINT("invalid endpoint", endpoint)
    return INVALID_ENDPOINT_INDEX;
  }
  
  --endpointIndex;

  if (IS_ENDPOINT_IN(endpoint)) {

    if(usbdevices[device].endpoints[endpointIndex].in.type == 0) {
      PRINT_ERROR_INVALID_ENDPOINT("no such endpoint", endpoint)
      return INVALID_ENDPOINT_INDEX;
    }
    if (count > usbdevices[device].endpoints[endpointIndex].in.size) {

      PRINT_ERROR_OTHER("incorrect transfer size")
      return INVALID_ENDPOINT_INDEX;
    }
  } else {

    if(usbdevices[device].endpoints[endpointIndex].out.type == 0) {
      PRINT_ERROR_INVALID_ENDPOINT("no such endpoint", endpoint)
      return INVALID_ENDPOINT_INDEX;
    }
    if (count > usbdevices[device].endpoints[endpointIndex].out.size) {

      PRINT_ERROR_OTHER("incorrect transfer size")
      return INVALID_ENDPOINT_INDEX;
    }
  }
  
  return endpointIndex;
}
#define GET_ENDPOINT(device,endpoint,direction,count) \
        get_endpoint(device, endpoint, direction, count, __FILE__, __LINE__, __func__);

static char * make_path(libusb_device * dev) {
  uint8_t path[1 + 7] = { };
  int pathLen = sizeof(path) / sizeof(*path);
  static char str[sizeof(path) / sizeof(*path) * 3];
  path[0] = libusb_get_bus_number(dev);
  int ret = libusb_get_port_numbers(dev, path + 1, pathLen - 1);
  if (ret < 0) {
    PRINT_ERROR_LIBUSB("libusb_get_port_numbers", ret)
    return NULL;
  }
  int i;
  for (i = 0; i < ret + 1; ++i) {
    snprintf(str + i * 3, sizeof(str) - i * 3, "%02x:", path[i]);
  }
  str[(ret + 1) * 3 - 1] = '\0';
  return str;
}

static int add_device(const char * path, int print) {
  int i;
  for (i = 0; i < USBASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].path && !strcmp(usbdevices[i].path, path)) {
      if (print) {
        PRINT_ERROR_OTHER("device already opened")
      }
      return -1;
    }
  }
  for (i = 0; i < USBASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].devh == NULL) {
      usbdevices[i].path = strdup(path);
      if (usbdevices[i].path != NULL) {
        return i;
      } else {
        PRINT_ERROR_OTHER("can't duplicate path")
        return -1;
      }
    }
  }
  return -1;
}

static int submit_transfer(struct libusb_transfer * transfer) {
  /*
   * Don't submit the transfer if it can't be added in the 'transfers' table.
   * Otherwise it would not be possible to cleanly cancel it.
   */
  int ret = add_transfer(transfer);

  if (ret != -1) {
    ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS) {
      PRINT_ERROR_LIBUSB("libusb_submit_transfer", ret)
      remove_transfer(transfer);
      return -1;
    }
  }
  return ret;
}

static void usb_callback(struct libusb_transfer* transfer) {

  int device = (unsigned long) transfer->user_data;

  //make sure the device still exists, in case something went wrong
  if(usbasync_check_device(device, __FILE__, __LINE__, __func__) < 0) {
    remove_transfer(transfer);
    return;
  }

  int status;
  switch (transfer->status) {
  case LIBUSB_TRANSFER_COMPLETED:
    status = transfer->actual_length;
    break;
  case LIBUSB_TRANSFER_TIMED_OUT:
    status = E_TRANSFER_TIMED_OUT;
    break;
  case LIBUSB_TRANSFER_STALL:
    status = E_TRANSFER_STALL;
    break;
  case LIBUSB_TRANSFER_CANCELLED:
    break;
  default:
    status = E_TRANSFER_ERROR;
    PRINT_TRANSFER_ERROR(transfer)
    break;
  }
  if (transfer->status != LIBUSB_TRANSFER_CANCELLED) {
    if (transfer->type == LIBUSB_TRANSFER_TYPE_CONTROL) {
      struct libusb_control_setup * setup = libusb_control_transfer_get_setup(transfer);
      if(setup->bmRequestType & LIBUSB_ENDPOINT_IN) {
        unsigned char * data = libusb_control_transfer_get_data(transfer);
        usbdevices[device].callback.fp_read(usbdevices[device].callback.user, transfer->endpoint, data, status);
      } else {
        usbdevices[device].callback.fp_write(usbdevices[device].callback.user, transfer->endpoint, status);
      }
    } else {
      if (IS_ENDPOINT_OUT(transfer->endpoint)) {
        usbdevices[device].callback.fp_write(usbdevices[device].callback.user, transfer->endpoint, status);
      } else {
        usbdevices[device].callback.fp_read(usbdevices[device].callback.user, transfer->endpoint, transfer->buffer, status);
      }
    }
  }

  remove_transfer(transfer);
}

int gusb_poll(int device, unsigned char endpoint) {

  USBASYNC_CHECK_DEVICE(device, -1)

  unsigned char endpointIndex = GET_ENDPOINT(device, endpoint, LIBUSB_ENDPOINT_IN, 0)
  if(endpointIndex == INVALID_ENDPOINT_INDEX) {
  
    return -1;
  }

  if (usbdevices[device].callback.fp_read == NULL) {

    PRINT_ERROR_OTHER("missing read callback")
    return -1;
  }
  
  unsigned int size = usbdevices[device].endpoints[endpointIndex].in.size;

  unsigned char * buf = calloc(size, sizeof(char));
  if (buf == NULL) {

    PRINT_ERROR_ALLOC_FAILED("calloc")
    return -1;
  }

  struct libusb_transfer * transfer = libusb_alloc_transfer(0);
  if (transfer == NULL) {

    PRINT_ERROR_ALLOC_FAILED("libusb_alloc_transfer")
    free(buf);
    return -1;
  }

  switch (usbdevices[device].endpoints[endpointIndex].in.type) {
  case LIBUSB_TRANSFER_TYPE_INTERRUPT:
    libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, endpoint, buf, size,
        (libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, 0);
    break;
  default:
    PRINT_ERROR_OTHER("unsupported endpoint type")
    libusb_free_transfer(transfer);
    free(buf);
    return -1;
  }

  return submit_transfer(transfer);
}

static int handle_events(int unused) {
#ifndef WIN32
  return libusb_handle_events(ctx);
#else
  if(ctx != NULL)
  {
    struct timeval tv = {};
    return libusb_handle_events_timeout(ctx, &tv);
  }
  else
  {
    return 0;
  }
#endif
}

static int transfer_timeout(int device, unsigned char endpointIndex, unsigned char direction, const void * buf, unsigned int count, unsigned int timeout) {

  int transfered = -1;
  
  uint8_t endpointAddress = (endpointIndex + 1) | direction;

  uint8_t type;
  if (direction == LIBUSB_ENDPOINT_IN) {
    type = usbdevices[device].endpoints[endpointIndex].in.type;
  } else {
    type = usbdevices[device].endpoints[endpointIndex].out.type;
  }

  int ret = -1;
  switch (type) {
  case LIBUSB_TRANSFER_TYPE_INTERRUPT:
    ret = libusb_interrupt_transfer(usbdevices[device].devh, endpointAddress,
      (void *) buf, count, &transfered, timeout);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_TIMEOUT) {

      PRINT_ERROR_LIBUSB("libusb_interrupt_transfer", ret)
      return -1;
    }
    break;
  default:
    PRINT_ERROR_OTHER("unsupported endpoint type")
    break;
  }

  return transfered;
}

int gusb_write_timeout(int device, unsigned char endpoint, const void * buf, unsigned int count, unsigned int timeout) {

  USBASYNC_CHECK_DEVICE(device, -1)

  unsigned char endpointIndex = GET_ENDPOINT(device, endpoint, LIBUSB_ENDPOINT_OUT, count)
  if(endpointIndex == INVALID_ENDPOINT_INDEX) {
  
    return -1;
  }

  return transfer_timeout(device, endpointIndex, LIBUSB_ENDPOINT_OUT, buf, count, timeout);
}

int gusb_read_timeout(int device, unsigned char endpoint, void * buf, unsigned int count, unsigned int timeout) {

  USBASYNC_CHECK_DEVICE(device, -1)

  unsigned char endpointIndex = GET_ENDPOINT(device, endpoint, LIBUSB_ENDPOINT_IN, count)
  if(endpointIndex == INVALID_ENDPOINT_INDEX) {
  
    return -1;
  }

  return transfer_timeout(device, endpointIndex, LIBUSB_ENDPOINT_IN, buf, count, timeout);
}

static int get_configurations (int device) {

  s_usb_descriptors * descriptors = &usbdevices[device].descriptors;

  descriptors->configurations = calloc(descriptors->device.bNumConfigurations, sizeof(*descriptors->configurations));
  if (descriptors->configurations == NULL) {
    PRINT_ERROR_ALLOC_FAILED("calloc");
    return -1;
  }
  
  unsigned char index;
  for (index = 0; index < descriptors->device.bNumConfigurations; ++index) {
  
    struct usb_config_descriptor descriptor;
    
    int ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
        LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_CONFIG << 8) | index, 0, (unsigned char *)&descriptor,
        sizeof(descriptor), USBASYNC_DEFAULT_TIMEOUT);
    
    if (ret < 0) {
      PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
      return -1;
    }
    
    struct p_configuration * configurations = descriptors->configurations + index;

    configurations->raw = calloc(descriptor.wTotalLength, sizeof(unsigned char));
    if (configurations->raw == NULL) {
      PRINT_ERROR_ALLOC_FAILED("calloc");
      return -1;
    }
    
    ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
        LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_CONFIG << 8) | index, 0, configurations->raw,
        descriptor.wTotalLength, USBASYNC_DEFAULT_TIMEOUT);
    
    if (ret < 0) {
      PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
      return -1;
    }
  }

  return 0;
}

static int add_descriptor (int device, unsigned short wValue, unsigned short wIndex, unsigned short wLength, unsigned char * data) {

  s_usb_descriptors * descriptors = &usbdevices[device].descriptors;
  
  void * ptr = realloc(descriptors->others, (descriptors->nbOthers + 1) * sizeof(*descriptors->others));
  if (ptr == NULL) {
    PRINT_ERROR_ALLOC_FAILED("realloc");
    free(data);
    return -1;
  }

  descriptors->others = ptr;
  memset(descriptors->others + descriptors->nbOthers, 0x00, sizeof(*descriptors->others));
  descriptors->others[descriptors->nbOthers].wValue = wValue;
  descriptors->others[descriptors->nbOthers].wIndex = wIndex;
  descriptors->others[descriptors->nbOthers].wLength = wLength;
  descriptors->others[descriptors->nbOthers].data = data;
  ++descriptors->nbOthers;
  
  return 0;
}

static int get_string_descriptor (int device, unsigned char index) {

  s_usb_descriptors * descriptors = &usbdevices[device].descriptors;

  unsigned char * data = calloc(DEFAULT_STRING_BUFFER_SIZE, sizeof(*data));
  if (data == NULL) {
    PRINT_ERROR_ALLOC_FAILED("calloc");
    return -1;
  }

  int ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
      LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8) | index, descriptors->langId0.wData[0], data, DEFAULT_STRING_BUFFER_SIZE, USBASYNC_DEFAULT_TIMEOUT);

  if (ret < 0) {
    PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
    free(data);
    return -1;
  }

  struct usb_descriptor_header * descriptor = (struct usb_descriptor_header *)data;

  if (descriptor->bLength > ret) {
    void * ptr = realloc(data, descriptor->bLength * sizeof(*data));
    if (ptr == NULL) {
      PRINT_ERROR_ALLOC_FAILED("realloc");
      free(data);
      return -1;
    }

    data = ptr;

    ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
        LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8) | index, descriptors->langId0.wData[0], data, descriptor->bLength, USBASYNC_DEFAULT_TIMEOUT);
    if (ret < 0) {
      PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
      free(data);
      return -1;
    }
  }
    
  return add_descriptor(device, (LIBUSB_DT_STRING << 8) | index, descriptors->langId0.wData[0], ret, data);
}

static int probe_interface (int device, unsigned char configurationIndex, struct usb_interface_descriptor * interface) {

  struct p_configuration * pConfiguration = usbdevices[device].descriptors.configurations + configurationIndex;

  if (interface->bInterfaceNumber >= pConfiguration->descriptor->bNumInterfaces) {
    PRINT_ERROR_OTHER("bad interface number")
    return -1;
  }

  struct p_interface * pInterface = pConfiguration->interfaces + interface->bInterfaceNumber;

  void * altInterfaces = realloc(pInterface->altInterfaces, (pInterface->bNumAltInterfaces + 1) * sizeof(*pInterface->altInterfaces));
  if(altInterfaces == NULL) {
    PRINT_ERROR_ALLOC_FAILED("realloc");
    return -1;
  }

  pInterface->altInterfaces = altInterfaces;
  memset(pInterface->altInterfaces + pInterface->bNumAltInterfaces, 0x00, sizeof(*pInterface->altInterfaces));
  pInterface->altInterfaces[pInterface->bNumAltInterfaces].descriptor = interface;
  ++pInterface->bNumAltInterfaces;

  if (interface->iInterface) {
    int ret = get_string_descriptor (device, interface->iInterface);
    if (ret < 0) {
      return -1;
    }
  }

  return 0;
}

struct p_altInterface * get_interface (int device, unsigned char configurationIndex, struct usb_interface_descriptor * interface) {

  if (interface == NULL) {
      PRINT_ERROR_OTHER("missing interface")
      return NULL;
    }

    struct p_configuration * pConfiguration = usbdevices[device].descriptors.configurations + configurationIndex;

    if (interface->bInterfaceNumber >= pConfiguration->descriptor->bNumInterfaces) {
      PRINT_ERROR_OTHER("bad interface number")
      return NULL;
    }

    if (interface->bAlternateSetting >= pConfiguration->interfaces[interface->bInterfaceNumber].bNumAltInterfaces) {
      PRINT_ERROR_OTHER("bad alternative interface number")
      return NULL;
    }

    return pConfiguration->interfaces[interface->bInterfaceNumber].altInterfaces + interface->bAlternateSetting;
}

static int probe_hid (int device, unsigned char configurationIndex, struct usb_interface_descriptor * interface, struct usb_hid_descriptor * hid) {

  struct p_altInterface * pAltInterface = get_interface(device, configurationIndex, interface);
  if (pAltInterface == NULL) {
    return -1;
  }
  
  if (pAltInterface->descriptor->bInterfaceClass != LIBUSB_CLASS_HID) {
    return 0;
  }
  
  pAltInterface->hidDescriptor = hid;

  unsigned char rdescIndex;
  for (rdescIndex = 0; rdescIndex < hid->bNumDescriptors; ++ rdescIndex) {
    if (hid->rdesc[rdescIndex].wReportDescriptorLength > 0) {
      unsigned char * data = calloc(hid->rdesc[rdescIndex].wReportDescriptorLength, sizeof(unsigned char));
      if (data == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc")
        return -1;
      }
      int ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE,
          LIBUSB_REQUEST_GET_DESCRIPTOR, (hid->rdesc[rdescIndex].bReportDescriptorType << 8) | 0, pAltInterface->descriptor->bInterfaceNumber, data, hid->rdesc[rdescIndex].wReportDescriptorLength, USBASYNC_DEFAULT_TIMEOUT);
      if (ret < 0) {
        PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
        free(data);
        return -1;
      }
      
      return add_descriptor(device, (hid->rdesc[rdescIndex].bReportDescriptorType << 8), pAltInterface->descriptor->bInterfaceNumber, ret, data);
    }
  }

  return 0;
}

static int probe_endpoint (int device, unsigned char configurationIndex, struct usb_interface_descriptor * interface, struct usb_endpoint_descriptor * endpoint) {

  struct p_altInterface * pAltInterface = get_interface(device, configurationIndex, interface);
  if (pAltInterface == NULL) {
    return -1;
  }

  void * endpoints = realloc(pAltInterface->endpoints, (pAltInterface->bNumEndpoints + 1) * sizeof(*pAltInterface->endpoints));
  if(endpoints == NULL) {
    PRINT_ERROR_ALLOC_FAILED("realloc");
    return -1;
  }

  pAltInterface->endpoints = endpoints;
  memset(pAltInterface->endpoints + pAltInterface->bNumEndpoints, 0x00, sizeof(*pAltInterface->endpoints));
  pAltInterface->endpoints[pAltInterface->bNumEndpoints] = endpoint;
  ++pAltInterface->bNumEndpoints;

  uint16_t size = endpoint->wMaxPacketSize;
  uint8_t type = endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
  uint8_t endpointNumber = endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK;
  if (endpointNumber > 0) {
    if (IS_ENDPOINT_IN(endpoint->bEndpointAddress)) {
      usbdevices[device].endpoints[endpointNumber - 1].in.type = type;
      usbdevices[device].endpoints[endpointNumber - 1].in.size = size;
    } else {
      usbdevices[device].endpoints[endpointNumber - 1].out.type = type;
      usbdevices[device].endpoints[endpointNumber - 1].out.size = size;
    }
  }

  return 0;
}

static int probe_configurations (int device) {

  s_usb_descriptors * descriptors = &usbdevices[device].descriptors;

  int ret;

  unsigned char index;
  for (index = 0; index < descriptors->device.bNumConfigurations; ++index) {
  
    void * ptr = descriptors->configurations[index].raw;

    descriptors->configurations[index].descriptor = ptr;
    struct usb_config_descriptor * configuration = ptr;

    descriptors->configurations[index].interfaces = calloc(configuration->bNumInterfaces, sizeof(*descriptors->configurations[index].interfaces));
    if (descriptors->configurations[index].interfaces == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc");
        return -1;
    }
  
    if (configuration->iConfiguration) {
      ret = get_string_descriptor (device, configuration->iConfiguration);
      if (ret < 0) {
        return -1;
      }
    }
    
    ptr += configuration->bLength;

    struct usb_interface_descriptor * interface = NULL;
  
    while (ptr < (void *)configuration + configuration->wTotalLength) {
      
      struct usb_descriptor_header * header = ptr;
      
      switch (header->bDescriptorType) {
      break;
      case LIBUSB_DT_INTERFACE:
      interface = ptr;
      ret = probe_interface(device, index, ptr);
      if (ret < 0) {
        return -1;
      }
      break;
      case LIBUSB_DT_ENDPOINT:
      ret = probe_endpoint(device, index, interface, ptr);
      if (ret < 0) {
        return -1;
      }
      break;
      case LIBUSB_DT_HID:
        ret = probe_hid(device, index, interface, ptr);
        if (ret < 0) {
          return -1;
        }
      break;
      case LIBUSB_DT_CONFIG:
      case LIBUSB_DT_REPORT:
      case LIBUSB_DT_PHYSICAL:
      case LIBUSB_DT_DEVICE:
      case LIBUSB_DT_STRING:
      default:
      fprintf(stderr, "unhandled descriptor type: 0x%02x\n", header->bDescriptorType);
      break;
      }
      
      ptr += header->bLength;
    }
  }
  
  return 0;
}

static int get_device (int device) {

  struct usb_device_descriptor * descriptor = &usbdevices[device].descriptors.device;
  
  int ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
      LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_DEVICE << 8) | 0, 0, (unsigned char *)descriptor,
      sizeof(*descriptor), USBASYNC_DEFAULT_TIMEOUT);
  
  if (ret < 0) {
    PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
    return -1;
  }
  
  if (descriptor->iManufacturer) {
    ret = get_string_descriptor (device, descriptor->iManufacturer);
    if (ret < 0) {
      return -1;
    }
  }
  
  if (descriptor->iProduct) {
    ret = get_string_descriptor (device, descriptor->iProduct);
    if (ret < 0) {
      return -1;
    }
  }
  
  if (descriptor->iSerialNumber) {
    ret = get_string_descriptor (device, descriptor->iSerialNumber);
    if (ret < 0) {
      return -1;
    }
  }

  return 0;
}

static void get_lang_id_0 (int device) {

  struct usb_string_descriptor * descriptor = &usbdevices[device].descriptors.langId0;

  int ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN,
      LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8) | 0, 0, (unsigned char *)descriptor,
      sizeof(*descriptor), USBASYNC_DEFAULT_TIMEOUT);
  
  if (ret < 0) {
    PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
  }
}

static int get_descriptors (int device) {

  get_lang_id_0(device);
  
  int ret = get_device(device);
  if (ret < 0) {
    return -1;
  }
  
  if(usbdevices[device].descriptors.device.bNumConfigurations == 0) {
    PRINT_ERROR_OTHER("device has no configuration")
    return -1;
  }
  
  ret = get_configurations(device);
  if (ret < 0) {
    return -1;
  }
  
  return probe_configurations(device);
}

static int handle_interfaces(int device, int claim) {

  libusb_device * dev = libusb_get_device(usbdevices[device].devh);
  if (dev == NULL) {
    PRINT_ERROR_OTHER("libusb_get_device failed")
    return -1;
  }

  struct libusb_device_descriptor desc;
  int ret = libusb_get_device_descriptor(dev, &desc);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_get_device_descriptor", ret)
    return -1;
  }

  if (desc.bNumConfigurations) {

    struct libusb_config_descriptor * configuration;
    ret = libusb_get_config_descriptor(dev, 0, &configuration);
    if (ret != LIBUSB_SUCCESS) {
      return -1;
    }
    int interfaceIndex;
    for (interfaceIndex = 0; interfaceIndex < configuration->bNumInterfaces; ++interfaceIndex) {
      const struct libusb_interface * interface = configuration->interface + interfaceIndex;
      if(claim) {
        ret = libusb_claim_interface(usbdevices[device].devh,  interface->altsetting->bInterfaceNumber);
        if (ret != LIBUSB_SUCCESS) {
          PRINT_ERROR_LIBUSB("libusb_claim_interface", ret)
          libusb_free_config_descriptor(configuration);
          return -1;
        }
      } else {
        ret = libusb_release_interface(usbdevices[device].devh, interface->altsetting->bInterfaceNumber); //warning: this is a blocking function
        if (ret != LIBUSB_SUCCESS) {
          PRINT_ERROR_LIBUSB("libusb_release_interface", ret)
          libusb_free_config_descriptor(configuration);
          return -1;
        }
      }
    }
    libusb_free_config_descriptor(configuration);
  }

  return 0;
}

static int claim_device(int device, libusb_device * dev, struct libusb_device_descriptor * desc) {

  int ret = libusb_open(dev, &usbdevices[device].devh);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_open", ret)
    return -1;
  }

#if defined(LIBUSB_API_VERSION) || defined(LIBUSBX_API_VERSION)
  libusb_set_auto_detach_kernel_driver(usbdevices[device].devh, 1);
#else
#ifndef WIN32
  ret = libusb_kernel_driver_active(usbdevices[device].devh, 0);
  if(ret == 1)
  {
    ret = libusb_detach_kernel_driver(usbdevices[device].devh, 0);
    if(ret != LIBUSB_SUCCESS)
    {
      PRINT_ERROR_LIBUSB("libusb_detach_kernel_driver", ret)
      return -1;
    }
  }
  else if(ret != LIBUSB_SUCCESS)
  {
    PRINT_ERROR_LIBUSB("libusb_kernel_driver_active", ret)
    return -1;
  }
#endif
#endif

  ret = libusb_reset_device(usbdevices[device].devh);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_reset_device", ret)
    return -1;
  }

  int configuration;
  
  ret = libusb_get_configuration(usbdevices[device].devh, &configuration);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_get_configuration", ret)
    return -1;
  }

  if (configuration == 0) {
    configuration = 1;
    ret = libusb_set_configuration(usbdevices[device].devh, 1); //warning: this is a blocking function
    if (ret != LIBUSB_SUCCESS) {
      PRINT_ERROR_LIBUSB("libusb_set_configuration", ret)
      return -1;
    }
  }

  ret = handle_interfaces(device, 1);
  if(ret < 0) {
      return -1;
  }

  // Don't use libusb_get_config_descriptor: it squeezes out some parts of the descriptor!
  ret = get_descriptors(device);
  if(ret < 0) {
      return -1;
  }

  return 0;
}

s_usb_dev * gusb_enumerate(unsigned short vendor, unsigned short product) {

  s_usb_dev * usb_devs = NULL;
  unsigned int nb_usb_devs = 0;

  int ret = -1;

  static libusb_device** devs = NULL;
  static ssize_t cnt = 0;
  int dev_i;

  if (!ctx) {
    PRINT_ERROR_OTHER("no libusb context")
    return NULL;
  }

  cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    PRINT_ERROR_LIBUSB("libusb_get_device_list", cnt)
    return NULL;
  }

  for (dev_i = 0; dev_i < cnt; ++dev_i) {
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[dev_i], &desc);
    if (!ret) {
      if (vendor) {
        if (desc.idVendor != vendor) {
          continue;
        }
        if (product) {
          if (desc.idProduct != product) {
            continue;
          }
        }
      }

      const char * spath = make_path(devs[dev_i]);
      if (spath == NULL) {
        continue;
      }

      char * path = strdup(spath);
      if (path == NULL) {
        PRINT_ERROR_OTHER("strdup failed")
        continue;
      }

      void * ptr = realloc(usb_devs, (nb_usb_devs + 1) * sizeof(*usb_devs));
      if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc")
        free(path);
        continue;
      }

      usb_devs = ptr;

      if (nb_usb_devs > 0) {
        usb_devs[nb_usb_devs - 1].next = 1;
      }

      usb_devs[nb_usb_devs].path = path;
      usb_devs[nb_usb_devs].vendor_id = desc.idVendor;
      usb_devs[nb_usb_devs].product_id = desc.idProduct;
      usb_devs[nb_usb_devs].next = 0;

      ++nb_usb_devs;
    }
  }

  libusb_free_device_list(devs, 1);

  return usb_devs;
}

void gusb_free_enumeration(s_usb_dev * usb_devs) {

  s_usb_dev * current;
  for (current = usb_devs; current != NULL; ++current) {

    free(current->path);

    if (current->next == 0) {
      break;
    }
  }
  free(usb_devs);
}

int gusb_open_ids(unsigned short vendor, unsigned short product) {

  int ret = -1;

  static libusb_device** devs = NULL;
  static ssize_t cnt = 0;
  int dev_i;

  if (!ctx) {
    PRINT_ERROR_OTHER("no libusb context")
    return -1;
  }

  cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    PRINT_ERROR_LIBUSB("libusb_get_device_list", cnt)
    return -1;
  }

  for (dev_i = 0; dev_i < cnt; ++dev_i) {
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[dev_i], &desc);
    if (!ret) {
      if (desc.idVendor == vendor && desc.idProduct == product) {

        const char * spath = make_path(devs[dev_i]);
        if (spath == NULL) {
          continue;
        }

        int device = add_device(spath, 0);
        if (device < 0) {
          continue;
        }

        if (claim_device(device, devs[dev_i], &desc) != -1) {
          libusb_free_device_list(devs, 1);
          return device;
        } else {
          gusb_close(device);
        }
      }
    }
  }

  libusb_free_device_list(devs, 1);

  return -1;
}

int gusb_open_path(const char * path) {

  int ret = -1;

  static libusb_device** devs = NULL;
  static ssize_t cnt = 0;
  int dev_i;

  if (path == NULL) {
    PRINT_ERROR_OTHER("path is NULL");
    return -1;
  }

  if (!ctx) {
    PRINT_ERROR_OTHER("no libusb context")
    return -1;
  }

  cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    PRINT_ERROR_LIBUSB("libusb_get_device_list", cnt)
    return -1;
  }

  for (dev_i = 0; dev_i < cnt; ++dev_i) {
    const char * spath = make_path(devs[dev_i]);
    if (spath == NULL || strcmp(spath, path)) {
      continue;
    }
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[dev_i], &desc);
    if (!ret) {

      int device = add_device(path, 0);
      if (device < 0) {
        continue;
      }

      if (claim_device(device, devs[dev_i], &desc) != -1) {
        libusb_free_device_list(devs, 1);
        return device;
      } else {
        gusb_close(device);
      }
    }
  }

  libusb_free_device_list(devs, 1);

  return -1;
}

s_usb_descriptors * gusb_get_usb_descriptors(int device) {

  USBASYNC_CHECK_DEVICE(device, NULL)

  return &usbdevices[device].descriptors;
}

static int close_callback(int device) {

  USBASYNC_CHECK_DEVICE(device, -1)

  return usbdevices[device].callback.fp_close(usbdevices[device].callback.user);
}

int gusb_register(int device, int user, USBASYNC_READ_CALLBACK fp_read, USBASYNC_WRITE_CALLBACK fp_write,
    USBASYNC_CLOSE_CALLBACK fp_close, GPOLL_REGISTER_FD fp_register) {

  USBASYNC_CHECK_DEVICE(device, -1)

  int ret = 0;

  const struct libusb_pollfd** pfd_usb = libusb_get_pollfds(ctx);
  int poll_i;
  for (poll_i = 0; pfd_usb[poll_i] != NULL && ret != -1; ++poll_i) {

    ret = fp_register(pfd_usb[poll_i]->fd, device, handle_events, handle_events, close_callback);
  }
  free(pfd_usb);

  if (ret != -1) {
    usbdevices[device].callback.user = user;
    usbdevices[device].callback.fp_read = fp_read;
    usbdevices[device].callback.fp_write = fp_write;
    usbdevices[device].callback.fp_close = fp_close;
  }

  return ret;
}

/*
 * Cancel all pending tranfers for a given device.
 */
static void cancel_transfers(int device) {
  unsigned int i;
  for (i = 0; i < transfers_nb; ++i) {

    if ((unsigned long) (transfers[i]->user_data) == (unsigned long) device) {

      libusb_cancel_transfer(transfers[i]);
    }
  }

  while (usbdevices[device].pending_transfers) {

    if (libusb_handle_events(ctx) != LIBUSB_SUCCESS) {

      break;
    }
  }
}

int gusb_close(int device) {

  if (device < 0 || device >= USBASYNC_MAX_DEVICES) {
    PRINT_ERROR_OTHER("invalid device");
    return -1;
  }

  if (usbdevices[device].devh) {

    usbdevices[device].closing = 1;

    cancel_transfers(device);

    handle_interfaces(device, 0); //warning: this is a blocking function
#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
#ifndef WIN32
        libusb_attach_kernel_driver(usbdevices[device].devh, 0);
#endif
#endif
    libusb_close(usbdevices[device].devh);
  }

  free(usbdevices[device].path);
  unsigned char configurationIndex;
  for (configurationIndex = 0; configurationIndex < usbdevices[device].descriptors.device.bNumConfigurations; ++configurationIndex) {
    struct p_configuration * pConfiguration = usbdevices[device].descriptors.configurations + configurationIndex;
    if (pConfiguration->descriptor != NULL) {
      unsigned char interfaceIndex;
      for (interfaceIndex = 0; interfaceIndex < pConfiguration->descriptor->bNumInterfaces; ++interfaceIndex) {
        struct p_interface * pInterface = pConfiguration->interfaces + interfaceIndex;
        unsigned char altInterfaceIndex;
        for (altInterfaceIndex = 0; altInterfaceIndex < pInterface->bNumAltInterfaces; ++altInterfaceIndex) {
          struct p_altInterface * pAltInterface = pInterface->altInterfaces + altInterfaceIndex;
          free(pAltInterface->endpoints);
        }
        free(pInterface->altInterfaces);
      }
      free(pConfiguration->interfaces);
    }
    free(pConfiguration->raw);
  }
  free(usbdevices[device].descriptors.configurations);
  unsigned int othersIndex;
  for (othersIndex = 0; othersIndex < usbdevices[device].descriptors.nbOthers; ++othersIndex) {
    free(usbdevices[device].descriptors.others[othersIndex].data);
  }
  free(usbdevices[device].descriptors.others);

  memset(usbdevices + device, 0x00, sizeof(*usbdevices));

  return 1;
}

int gusb_write(int device, unsigned char endpoint, const void * buf, unsigned int count) {

  USBASYNC_CHECK_DEVICE(device, -1)

  if (endpoint != 0) {

    unsigned char endpointIndex = GET_ENDPOINT(device, endpoint, LIBUSB_ENDPOINT_OUT, 0)
    if(endpointIndex == INVALID_ENDPOINT_INDEX) {

      return -1;
    }
  } else {

    struct libusb_control_setup * control_setup = (struct libusb_control_setup *)buf;
    if(control_setup->bmRequestType & LIBUSB_ENDPOINT_IN) {

      count += control_setup->wLength;
    }
  }

  if (usbdevices[device].callback.fp_write == NULL) {

    PRINT_ERROR_OTHER("missing write callback")
    return -1;
  }

  unsigned char * buffer = malloc(count * sizeof(unsigned char));
  if (buffer == NULL) {

    PRINT_ERROR_ALLOC_FAILED("calloc")
    return -1;
  }

  memcpy(buffer, buf, count);

  struct libusb_transfer * transfer = libusb_alloc_transfer(0);
  if (transfer == NULL) {

    PRINT_ERROR_ALLOC_FAILED("libusb_alloc_transfer")
    free(buffer);
    return -1;
  }

  if (endpoint == 0) {

    libusb_fill_control_transfer(transfer, usbdevices[device].devh,
        buffer, (libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, 50);
  } else {

    libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, endpoint,
        buffer, count, (libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, USBASYNC_OUT_TIMEOUT);
  }

  return submit_transfer(transfer);
}
