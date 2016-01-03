/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gusbhid.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define USBHIDASYNC_MAX_DEVICES 256

#define USBHIDASYNC_OUT_TIMEOUT 20 // ms

typedef struct {
  int configuration;
  struct {
    int number;
    int alternateSetting;
  } interface;
  struct {
    struct {
      unsigned char address;
      unsigned short size;
    } in;
    struct {
      unsigned char address;
      unsigned short size;
    } out;
  } endpoints;
  s_hid_info hidInfo;
} s_config;

static struct {
  char * path;
  libusb_device_handle * devh;
  s_config config;
  struct {
    int user;
    ASYNC_READ_CALLBACK fp_read;
    ASYNC_WRITE_CALLBACK fp_write;
    ASYNC_CLOSE_CALLBACK fp_close;
  } callback;
  int pending_transfers;
  int closing;
} usbdevices[USBHIDASYNC_MAX_DEVICES] = { };

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

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

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

void usbhidasync_init(void) __attribute__((constructor (101)));
void usbhidasync_init(void) {
  int ret = libusb_init(&ctx);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_init", ret)
    exit(-1);
  }
}

void usbhidasync_clean(void) __attribute__((destructor (101)));
void usbhidasync_clean(void) {
  int i;
  for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].devh != NULL) {
      gusbhid_close(i);
    }
  }
  libusb_exit(ctx);
}

inline int usbhidasync_check_device(int device, const char * file, unsigned int line, const char * func) {
  if (device < 0 || device >= USBHIDASYNC_MAX_DEVICES) {
    fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
    return -1;
  }
  if (usbdevices[device].devh == NULL) {
    fprintf(stderr, "%s:%d %s: no such device\n", file, line, func);
    return -1;
  }
  return 0;
}
#define USBHIDASYNC_CHECK_DEVICE(device,retValue) \
  if(usbhidasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

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

static int add_device(const char * path, s_config * config, int print) {
  int i;
  for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].path && !strcmp(usbdevices[i].path, path)) {
      if (print) {
        PRINT_ERROR_OTHER("device already opened")
      }
      return -1;
    }
  }
  for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
    if (usbdevices[i].devh == NULL) {
      usbdevices[i].path = strdup(path);
      if (usbdevices[i].path != NULL) {
        usbdevices[i].config = *config;
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

static void usb_callback(struct libusb_transfer* transfer);

int gusbhid_poll(int device) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  unsigned int address = usbdevices[device].config.endpoints.in.address;
  unsigned int size = usbdevices[device].config.endpoints.in.size;

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

  libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, address, buf, size,
      (libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, 1000);

  return submit_transfer(transfer);
}

static void usb_callback(struct libusb_transfer* transfer) {

  int device = (unsigned long) transfer->user_data;

  //make sure the device still exists, in case something went wrong
  if(usbhidasync_check_device(device, __FILE__, __LINE__, __func__) == 0) {
    if (transfer->type == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
      if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        if (transfer->endpoint == usbdevices[device].config.endpoints.in.address) {
          usbdevices[device].callback.fp_read(usbdevices[device].callback.user, transfer->buffer,
              transfer->actual_length);
        } else if (transfer->endpoint == usbdevices[device].config.endpoints.out.address) {
          usbdevices[device].callback.fp_write(usbdevices[device].callback.user, transfer->actual_length);
        }
      } else {
        if (transfer->endpoint == usbdevices[device].config.endpoints.out.address
            || (transfer->status != LIBUSB_TRANSFER_TIMED_OUT && transfer->status != LIBUSB_TRANSFER_CANCELLED)) {
          fprintf(stderr, "libusb_transfer failed with status %s (endpoint=0x%02x)\n",
              libusb_error_name(transfer->status), transfer->endpoint);
        }
        if (transfer->endpoint == usbdevices[device].config.endpoints.out.address) {
          usbdevices[device].callback.fp_write(usbdevices[device].callback.user, -1);
        }
      }
    }
  }

  remove_transfer(transfer);
}

int handle_events(int unused) {
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

int handle_write_events(int unused1, int unused2) {
  return handle_events(unused1);
}

int gusbhid_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  if (usbdevices[device].config.endpoints.out.address == 0x00) {

    PRINT_ERROR_OTHER("the device has no HID OUT endpoint")
    return -1;
  }

  int transfered;

  int length = count;

  if (((unsigned char *) buf)[0] == 0x00) {
    --length;
    ++buf;
  }

  if (length > usbdevices[device].config.endpoints.out.size) {

    PRINT_ERROR_OTHER("incorrect write size")
    return -1;
  }

  int ret = libusb_interrupt_transfer(usbdevices[device].devh, usbdevices[device].config.endpoints.out.address,
      (void *) buf, length, &transfered, timeout);
  if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_TIMEOUT) {

    PRINT_ERROR_LIBUSB("libusb_interrupt_transfer", ret)
    return -1;
  }

  return count;
}

int gusbhid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  if (usbdevices[device].config.endpoints.in.address == 0x00) {

    PRINT_ERROR_OTHER("the device has no HID IN endpoint")
    return -1;
  }

  int transfered;

  if (count > usbdevices[device].config.endpoints.in.size) {
    count = usbdevices[device].config.endpoints.in.size;
  }

  int ret = libusb_interrupt_transfer(usbdevices[device].devh, usbdevices[device].config.endpoints.in.address,
      (void *) buf, count, &transfered, timeout);
  if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_TIMEOUT) {
    PRINT_ERROR_LIBUSB("libusb_interrupt_transfer", ret)
    return -1;
  }

  return transfered;
}

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

typedef struct
PACKED
{
  unsigned char bLength;
  unsigned char bDescriptorType;
  unsigned short bcdHID;
  unsigned char bCountryCode;
  unsigned char bNumDescriptors;
  unsigned char bReportDescriptorType;
  unsigned short wReportDescriptorLength;
} s_hid_descriptor;

static s_hid_info probe_hid(const unsigned char * extra, unsigned int extra_length) {

  s_hid_info hidInfo = { 0 };

  unsigned int pos;
  for (pos = 0; pos < extra_length && extra[pos]; pos += extra[pos]) {

    if (extra[pos] < sizeof(s_hid_descriptor)) {
      continue;
    }

    s_hid_descriptor * hidDescriptor = (s_hid_descriptor *) (extra + pos);

    if (hidDescriptor->bDescriptorType == LIBUSB_DT_HID && hidDescriptor->bReportDescriptorType == LIBUSB_DT_REPORT) {
      hidInfo.version = hidDescriptor->bcdHID;
      hidInfo.countryCode = hidDescriptor->bCountryCode;
      hidInfo.reportDescriptorLength = hidDescriptor->wReportDescriptorLength;
      return hidInfo;
    }
  }

  return hidInfo;
}

/*
 * Look for an interface that is from the HID class and that has at least one interrupt endpoint.
 * Return the following properties:
 * - bConfigurationValue
 * - bInterfaceNumber
 * - bAlternateSetting
 * - interrupt in endpoint: bEndpointAddress, wMaxPacketSize
 * - interrupt out endpoint: bEndpointAddress, wMaxPacketSize
 * - hid info (version, country, report descriptor)
 */
static s_config probe_device(libusb_device * dev, struct libusb_device_descriptor * desc) {

  s_config config = { .configuration = -1 };

  int cfg;
  for (cfg = 0; cfg < desc->bNumConfigurations && config.configuration == -1; ++cfg) {
    struct libusb_config_descriptor * configuration;
    int ret = libusb_get_config_descriptor(dev, cfg, &configuration);
    if (ret != LIBUSB_SUCCESS) {
      PRINT_ERROR_LIBUSB("libusb_get_config_descriptor", ret)
      return config;
    }
    int itf;
    for (itf = 0; itf < configuration->bNumInterfaces && config.configuration == -1; ++itf) {
      int alt;
      const struct libusb_interface * interface = configuration->interface + itf;
      for (alt = 0; alt < interface->num_altsetting && config.configuration == -1; ++alt) {
        const struct libusb_interface_descriptor * interfaceDesc = interface->altsetting + alt;
        if (interfaceDesc->bInterfaceClass == LIBUSB_CLASS_HID) {
          config.configuration = configuration->bConfigurationValue;
          config.interface.number = itf;
          config.interface.alternateSetting = alt;
          config.hidInfo = probe_hid(interfaceDesc->extra, interfaceDesc->extra_length);
          config.hidInfo.vendor_id = desc->idVendor;
          config.hidInfo.product_id = desc->idProduct;
          config.hidInfo.bcdDevice = desc->bcdDevice;
          int ep;
          for (ep = 0; ep < interfaceDesc->bNumEndpoints; ++ep) {
            const struct libusb_endpoint_descriptor * endpoint = interfaceDesc->endpoint + ep;
            if ((endpoint->bmAttributes & 0b00000011) == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
              if ((endpoint->bEndpointAddress & 0b10000000) == LIBUSB_ENDPOINT_IN) {
                if (config.endpoints.in.address == 0) {
                  config.endpoints.in.address = endpoint->bEndpointAddress;
                  config.endpoints.in.size = endpoint->wMaxPacketSize;
                }
              } else {
                if (config.endpoints.out.address == 0) {
                  config.endpoints.out.address = endpoint->bEndpointAddress;
                  config.endpoints.out.size = endpoint->wMaxPacketSize;
                }
              }
            }
          }
        }
      }
    }
    libusb_free_config_descriptor(configuration);
  }

  return config;
}

/*
 * This is a wrapper around libusb_get_string_descriptor_ascii.
 * If libusb_get_string_descriptor_ascii returns a timeout, it is called again.
 * libusb_get_string_descriptor_ascii can be called up to 5 times, thus the execution of this function can take up to 5s.
 */
static int get_string_descriptor_ascii(libusb_device_handle *dev, uint8_t desc_index, unsigned char *data, int length) {

  int ret;
  int i;
  for (i = 0; i < 5; ++i) {
    ret = libusb_get_string_descriptor_ascii(dev, desc_index, data, length);
    if (ret < 0) {
      if (ret != LIBUSB_ERROR_TIMEOUT) {
        PRINT_ERROR_LIBUSB("libusb_get_string_descriptor_ascii", ret)
        break;
      }
    } else {
      break;
    }
  }
  if (i == 5) {
    PRINT_ERROR_LIBUSB("libusb_get_string_descriptor_ascii", LIBUSB_ERROR_TIMEOUT)
  }
  return ret;
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
  int configuration;

  ret = libusb_get_configuration(usbdevices[device].devh, &configuration);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_get_configuration", ret)
    return -1;
  }

  if (configuration != usbdevices[device].config.configuration) {
    //warning: this is a blocking function
    ret = libusb_set_configuration(usbdevices[device].devh, usbdevices[device].config.configuration);
    if (ret != LIBUSB_SUCCESS) {
      PRINT_ERROR_LIBUSB("libusb_set_configuration", ret)
      return -1;
    }
  }

  ret = libusb_claim_interface(usbdevices[device].devh, usbdevices[device].config.interface.number);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_claim_interface", ret)
    return -1;
  }

  //warning: this is a blocking function
  ret = libusb_set_interface_alt_setting(usbdevices[device].devh, usbdevices[device].config.interface.number,
      usbdevices[device].config.interface.alternateSetting);
  if (ret != LIBUSB_SUCCESS) {
    PRINT_ERROR_LIBUSB("libusb_set_interface_alt_setting", ret)
    return -1;
  }

  s_hid_info * hidInfo = &usbdevices[device].config.hidInfo;
  if (hidInfo->reportDescriptorLength > 0) {
    hidInfo->reportDescriptor = calloc(hidInfo->reportDescriptorLength, sizeof(unsigned char));
    if (hidInfo->reportDescriptor == NULL) {
      PRINT_ERROR_ALLOC_FAILED("calloc");
      return -1;
    }
    ret = libusb_control_transfer(usbdevices[device].devh, LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE,
        LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_REPORT << 8) | 0, 0, hidInfo->reportDescriptor,
        hidInfo->reportDescriptorLength, 1000);
    if (ret < 0) {
      PRINT_ERROR_LIBUSB("libusb_control_transfer", ret)
      return -1;
    } else {
      hidInfo->reportDescriptorLength = ret;
    }
  }

  if (desc->iManufacturer != 0) {
    char manufacturerString[126] = "";
    ret = get_string_descriptor_ascii(usbdevices[device].devh, desc->iManufacturer,
        (unsigned char *) manufacturerString, sizeof(manufacturerString));
    if (ret < 0) {
      return -1;
    }
    if (ret > 0) {
      hidInfo->manufacturerString = strdup(manufacturerString);
    }
  }

  if (desc->iProduct != 0) {
    char productString[126] = "";
    ret = get_string_descriptor_ascii(usbdevices[device].devh, desc->iProduct, (unsigned char *) productString,
        sizeof(productString));
    if (ret < 0) {
      return -1;
    }
    if (ret > 0) {
      hidInfo->productString = strdup(productString);
    }
  }

  return 0;
}

s_hid_dev * gusbhid_enumerate(unsigned short vendor, unsigned short product) {

  s_hid_dev * hid_devs = NULL;
  unsigned int nb_hid_devs = 0;

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

      s_config config = probe_device(devs[dev_i], &desc);
      if (config.configuration == -1) {
        continue;
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

      void * ptr = realloc(hid_devs, (nb_hid_devs + 1) * sizeof(*hid_devs));
      if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc")
        free(path);
        continue;
      }

      hid_devs = ptr;

      if (nb_hid_devs > 0) {
        hid_devs[nb_hid_devs - 1].next = 1;
      }

      hid_devs[nb_hid_devs].path = path;
      hid_devs[nb_hid_devs].vendor_id = desc.idVendor;
      hid_devs[nb_hid_devs].product_id = desc.idProduct;
      hid_devs[nb_hid_devs].next = 0;

      ++nb_hid_devs;
    }
  }

  libusb_free_device_list(devs, 1);

  return hid_devs;
}

void gusbhid_free_enumeration(s_hid_dev * hid_devs) {

  s_hid_dev * current;
  for (current = hid_devs; current != NULL; ++current) {

    free(current->path);

    if (current->next == 0) {
      break;
    }
  }
  free(hid_devs);
}

int gusbhid_open_ids(unsigned short vendor, unsigned short product) {

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
        s_config config = probe_device(devs[dev_i], &desc);
        if (config.configuration == -1) {
          continue;
        }

        const char * spath = make_path(devs[dev_i]);
        if (spath == NULL) {
          continue;
        }

        int device = add_device(spath, &config, 0);
        if (device < 0) {
          continue;
        }

        if (claim_device(device, devs[dev_i], &desc) != -1) {
          libusb_free_device_list(devs, 1);
          return device;
        } else {
          gusbhid_close(device);
        }
      }
    }
  }

  libusb_free_device_list(devs, 1);

  return -1;
}

int gusbhid_open_path(const char * path) {

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
      s_config config = probe_device(devs[dev_i], &desc);
      if (config.configuration == -1) {
        continue;
      }

      int device = add_device(path, &config, 0);
      if (device < 0) {
        continue;
      }

      if (claim_device(device, devs[dev_i], &desc) != -1) {
        libusb_free_device_list(devs, 1);
        return device;
      } else {
        gusbhid_close(device);
      }
    }
  }

  libusb_free_device_list(devs, 1);

  return -1;
}

const s_hid_info * gusbhid_get_hid_info(int device) {

  USBHIDASYNC_CHECK_DEVICE(device, NULL)

  return &usbdevices[device].config.hidInfo;
}

static int close_callback(int device) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  return usbdevices[device].callback.fp_close(usbdevices[device].callback.user);
}

int gusbhid_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write,
    ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  if (usbdevices[device].callback.fp_read != NULL && usbdevices[device].config.endpoints.in.address == 0x00) {

    PRINT_ERROR_OTHER("the device has no HID IN endpoint")
    return -1;
  }

  // Checking the presence of a HID OUT endpoint is done in the usbhidasync_write* functions.

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

int gusbhid_close(int device) {

  if (device < 0 || device >= USBHIDASYNC_MAX_DEVICES) {
    PRINT_ERROR_OTHER("invalid device");
    return -1;
  }

  if (usbdevices[device].devh) {

    usbdevices[device].closing = 1;

    cancel_transfers(device);

    libusb_release_interface(usbdevices[device].devh, usbdevices[device].config.interface.number); //warning: this is a blocking function
#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
#ifndef WIN32
        libusb_attach_kernel_driver(usbdevices[device].devh, 0);
#endif
#endif
    libusb_close(usbdevices[device].devh);
  }

  free(usbdevices[device].path);
  free(usbdevices[device].config.hidInfo.reportDescriptor);
  free(usbdevices[device].config.hidInfo.manufacturerString);
  free(usbdevices[device].config.hidInfo.productString);

  memset(usbdevices + device, 0x00, sizeof(*usbdevices));

  return 1;
}

int gusbhid_write(int device, const void * buf, unsigned int count) {

  USBHIDASYNC_CHECK_DEVICE(device, -1)

  if (usbdevices[device].config.endpoints.out.address == 0x00) {

    PRINT_ERROR_OTHER("the device has no HID OUT endpoint")
    return -1;
  }

  if (usbdevices[device].callback.fp_write == NULL) {

    PRINT_ERROR_OTHER("missing write callback")
    return -1;
  }

  if (((unsigned char *) buf)[0] == 0x00) {
    --count;
    ++buf;
  }

  if (count > usbdevices[device].config.endpoints.out.size) {

    PRINT_ERROR_OTHER("incorrect write size")
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

  libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, usbdevices[device].config.endpoints.out.address,
      buffer, count, (libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, USBHIDASYNC_OUT_TIMEOUT);

  return submit_transfer(transfer);
}

