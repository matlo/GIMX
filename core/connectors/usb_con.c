/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <connectors/usb_con.h>
#include <connectors/protocol.h>
#include <adapter.h>
#include <mainloop.h>
#include <report2event/report2event.h>
#include <gimx.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
static const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

static libusb_context* ctx = NULL;
static libusb_device** devs = NULL;
static ssize_t cnt = 0;
static int nb_opened = 0;

#define REPORTS_MAX 2

static struct libusb_transfer ** transfers = NULL;
static unsigned int transfers_nb = 0;

static int add_transfer(struct libusb_transfer * transfer)
{
  void * ptr = realloc(transfers, (transfers_nb + 1) * sizeof(*transfers));
  if (ptr)
  {
    transfers = ptr;
    transfers[transfers_nb] = transfer;
    transfers_nb++;
    return 0;
  }
  else
  {
    fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
    return -1;
  }
}

static void remove_transfer(struct libusb_transfer * transfer)
{
  unsigned int i;
  for (i = 0; i < transfers_nb; ++i)
  {
    if (transfers[i] == transfer)
    {
      memmove(transfers + i, transfers + i + 1, (transfers_nb - i - 1) * sizeof(*transfers));
      transfers_nb--;
      void * ptr = realloc(transfers, transfers_nb * sizeof(*transfers));
      if (ptr || !transfers_nb)
      {
        transfers = ptr;
      }
      else
      {
        fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
      }
      break;
    }
  }
}

static struct
{
  const char* name;
  unsigned short vendor;
  unsigned short product;
  int configuration;
  int interface;
  struct
  {
    struct
    {
      unsigned char address;
      unsigned short size;
      struct
      {
        unsigned char nb;
        struct
        {
          unsigned char report_id;
          unsigned char report_length;
        } elements[REPORTS_MAX];
      } reports;
    } in;
    struct
    {
      unsigned char address;
      unsigned short size;
    } out;
  } endpoints;
} controller[C_TYPE_MAX] =
{
  [C_TYPE_DS4] =
  {
    .name = DS4_DEVICE_NAME,
    .vendor = DS4_VENDOR,
    .product = DS4_PRODUCT,
    .configuration = 1,
    .interface = 0,
    .endpoints =
    {
      .in =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE,
        .reports =
        {
          .nb = 1,
          .elements =
          {
            {
              .report_id = DS4_USB_HID_IN_REPORT_ID,
              .report_length = DS4_USB_INTERRUPT_PACKET_SIZE
            }
          }
        }
      },
      .out =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE
      }
    }
  },
  [C_TYPE_T300RS_PS4] =
  {
    .name = DS4_DEVICE_NAME,
    .vendor = DS4_VENDOR,
    .product = DS4_PRODUCT,
    .configuration = 1,
    .interface = 0,
    .endpoints =
    {
      .in =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE,
        .reports =
        {
          .nb = 1,
          .elements =
          {
            {
              .report_id = DS4_USB_HID_IN_REPORT_ID,
              .report_length = DS4_USB_INTERRUPT_PACKET_SIZE
            }
          }
        }
      },
      .out =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE
      }
    }
  },
  [C_TYPE_G29_PS4] =
  {
    .name = DS4_DEVICE_NAME,
    .vendor = DS4_VENDOR,
    .product = DS4_PRODUCT,
    .configuration = 1,
    .interface = 0,
    .endpoints =
    {
      .in =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE,
        .reports =
        {
          .nb = 1,
          .elements =
          {
            {
              .report_id = DS4_USB_HID_IN_REPORT_ID,
              .report_length = DS4_USB_INTERRUPT_PACKET_SIZE
            }
          }
        }
      },
      .out =
      {
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT,
        .size = DS4_USB_INTERRUPT_PACKET_SIZE
      }
    }
  },
  [C_TYPE_360_PAD] =
  {
    .name = X360_NAME,
    .vendor = X360_VENDOR,
    .product = X360_PRODUCT,
    .configuration = 1,
    .interface = 0,
    .endpoints =
    {
      .in =
      {
        .address = X360_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN,
        .size = X360_USB_INTERRUPT_PACKET_SIZE,
        .reports =
        {
          .nb = 1,
          .elements =
          {
            {
              .report_id = X360_USB_HID_IN_REPORT_ID,
              .report_length = sizeof(s_report_x360)
            }
          }
        }
      },
      .out =
      {
        .address = X360_USB_INTERRUPT_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT,
        .size = X360_USB_INTERRUPT_PACKET_SIZE
      }
    }
  },
  [C_TYPE_XONE_PAD] =
  {
    .name = XONE_NAME,
    .vendor = XONE_VENDOR,
    .product = XONE_PRODUCT,
    .configuration = 1,
    .interface = 0,
    .endpoints =
    {
      .in =
      {
        .address = XONE_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN,
        .size = XONE_USB_INTERRUPT_PACKET_SIZE,
        .reports =
        {
          .nb = 2,
          .elements =
          {
            {
              .report_id = XONE_USB_HID_IN_REPORT_ID,
              .report_length = sizeof(((s_report_xone*)NULL)->input)
            },
            {
              .report_id = XONE_USB_HID_IN_GUIDE_REPORT_ID,
              .report_length = sizeof(((s_report_xone*)NULL)->guide)
            },
          }
        }
      },
      .out =
      {
        .address = XONE_USB_INTERRUPT_ENDPOINT_OUT | LIBUSB_ENDPOINT_OUT,
        .size = XONE_USB_INTERRUPT_PACKET_SIZE
      }
    }
  }
};

static int usb_state_indexes[MAX_CONTROLLERS] = {};

static struct usb_state {
  e_controller_type type;
  libusb_device_handle* devh;
  unsigned char ack;
  int joystick_id;
  struct
  {
    unsigned char report_id;
    s_report_packet report;
  } reports[REPORTS_MAX];
  unsigned char counter; // this is used for the Xbox One (interrupt out)
} usb_states[MAX_CONTROLLERS];

static int usb_poll_interrupt(int usb_number);

static void process_report(int usb_number, struct usb_state * state, struct libusb_transfer * transfer)
{
  int i;
  for(i = 0; i < controller[state->type].endpoints.in.reports.nb; ++i)
  {
    unsigned char report_id = controller[state->type].endpoints.in.reports.elements[i].report_id;
    unsigned char report_length = controller[state->type].endpoints.in.reports.elements[i].report_length;
    if(transfer->buffer[0] == report_id)
    {
      if(transfer->actual_length == report_length)
      {
        if(state->type == C_TYPE_XONE_PAD && !adapter_get(usb_number)->status)
        {
          break;
        }

        s_report* current = (s_report*) transfer->buffer;
        s_report* previous = &state->reports[i].report.value;

        report2event(state->type, usb_number, (s_report*)current, (s_report*)previous, state->joystick_id);

        if(state->type == C_TYPE_DS4 || state->type == C_TYPE_T300RS_PS4 || state->type == C_TYPE_G29_PS4)
        {
          memcpy(&previous->ds4, &current->ds4, report_length); //s_report_ds4 is larger than report_length bytes!
        }
        else if(state->type == C_TYPE_360_PAD)
        {
          previous->x360 = current->x360;
        }
        else if(state->type == C_TYPE_XONE_PAD)
        {
          if(report_id == XONE_USB_HID_IN_REPORT_ID)
          {
            previous->xone.input = current->xone.input;
          }
          else if(report_id == XONE_USB_HID_IN_GUIDE_REPORT_ID)
          {
            previous->xone.guide = current->xone.guide;
          }
        }
      }
      else
      {
        fprintf(stderr, "incorrect report length on interrupt endpoint: received %d bytes, expected %d bytes\n", transfer->actual_length, report_length);
      }
      break;
    }
  }

  if(i == controller[state->type].endpoints.in.reports.nb)
  {
    if(state->type == C_TYPE_XONE_PAD && !adapter_get(usb_number)->status)
    {
      if(adapter_forward_interrupt_in(usb_number, transfer->buffer, transfer->actual_length) < 0)
      {
        fprintf(stderr, "can't forward interrupt data to the adapter\n");
      }
    }
  }
}

void usb_callback(struct libusb_transfer* transfer)
{
  int usb_number = *(int*)transfer->user_data;
  struct usb_state * state = usb_states+usb_number;

  struct libusb_control_setup* setup = libusb_control_transfer_get_setup(transfer);

  if(transfer->type == LIBUSB_TRANSFER_TYPE_CONTROL)
  {
    if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
      if(setup->bmRequestType & LIBUSB_ENDPOINT_IN)
      {
        if(transfer->actual_length > 0xff)
        {
          fprintf(stderr, "wLength (%hu) is higher than %hu\n", transfer->actual_length, BUFFER_SIZE-LIBUSB_CONTROL_SETUP_SIZE);
        }
        else
        {
          unsigned char *data = libusb_control_transfer_get_data(transfer);
          if(adapter_forward_control_in(usb_number, data, transfer->actual_length) < 0)
          {
            fprintf(stderr, "can't forward control data to the adapter\n");
          }
        }
      }
    }
    else
    {
      if(transfer->status != LIBUSB_TRANSFER_CANCELLED)
      {
        fprintf(stderr, "libusb_transfer failed with status %s (bmRequestType=0x%02x, bRequest=0x%02x, wValue=0x%04x)\n", libusb_error_name(transfer->status), setup->bmRequestType, setup->bRequest, setup->wValue);
      }
    }
  }
  else if(transfer->type == LIBUSB_TRANSFER_TYPE_INTERRUPT)
  {
    if(transfer->endpoint == controller[state->type].endpoints.in.address)
    {
      state->ack = 1;
    }

    if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
      if(transfer->endpoint == controller[state->type].endpoints.in.address)
      {
        // process joystick events
        if(transfer->actual_length <= controller[state->type].endpoints.in.size
            && transfer->actual_length > 0)
        {
          process_report(usb_number, state, transfer);
        }
      }
    }
    else
    {
      if(transfer->status != LIBUSB_TRANSFER_TIMED_OUT && transfer->status != LIBUSB_TRANSFER_CANCELLED)
      {
        fprintf(stderr, "libusb_transfer failed with status %s (endpoint=0x%02x)\n", libusb_error_name(transfer->status), transfer->endpoint);
      }
    }
  }

  remove_transfer(transfer);
}

static int submit_transfer(struct libusb_transfer * transfer)
{
  /*
   * Don't submit the transfer if it can't be added in the 'transfers' table.
   * Otherwise it would not be possible to cleanly cancel it.
   */
  int ret = add_transfer(transfer);

  if (ret != -1)
  {
    ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS)
    {
      fprintf(stderr, "libusb_submit_transfer: %s.\n", libusb_strerror(ret));
      remove_transfer(transfer);
      free(transfer->buffer);
      libusb_free_transfer(transfer);
      return -1;
    }
  }
  return ret;
}

static int usb_poll_interrupt(int usb_number)
{
  struct usb_state* state = usb_states+usb_number;
  struct libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER ;
  unsigned int size = controller[state->type].endpoints.in.size;
  unsigned char* buf = calloc(size, sizeof(char));
  libusb_fill_interrupt_transfer(transfer, state->devh, controller[state->type].endpoints.in.address, buf, size, (libusb_transfer_cb_fn)usb_callback, usb_state_indexes+usb_number, 1000);
  int ret = submit_transfer(transfer);
  if(ret != -1)
  {
    state->ack = 0;
  }
  return ret;
}

void usb_poll_interrupts()
{
  int i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    if(usb_states[i].devh && usb_states[i].ack)
    {
      usb_poll_interrupt(i);
    }
  }
}

int usb_handle_events(int unused)
{
#ifndef WIN32
  return libusb_handle_events(ctx);
#else
  if(ctx != NULL)
  {
    struct timeval tv = { 0 };
    return libusb_handle_events_timeout(ctx, &tv);
  }
  else
  {
    return 0;
  }
#endif
}

static int usb_send_interrupt_out_sync(int usb_number, unsigned char* buffer, unsigned char length)
{
  struct usb_state* state = usb_states+usb_number;

  int transferred;

  if(state->type == C_TYPE_XONE_PAD && length > 2)
  {
    buffer[2] = state->counter++;
  }

  int ret = libusb_interrupt_transfer(state->devh, controller[state->type].endpoints.out.address, buffer, length, &transferred, 1000);
  if(ret != LIBUSB_SUCCESS)
  {
    fprintf(stderr, "Error sending interrupt out: %s\n", libusb_strerror(ret));
    return -1;
  }

  return 0;
}

int usb_init(int usb_number, e_controller_type type)
{
  int ret = -1;
  int dev_i;

  struct usb_state* state = usb_states+usb_number;

  if(!controller[type].vendor || !controller[type].product)
  {
    gprintf(_("no pass-through device is needed\n"));
    return 0;
  }

  usb_state_indexes[usb_number] = usb_number;

  memset(state, 0x00, sizeof(*state));
  state->joystick_id = -1;
  state->type = type;
  state->ack = 1;

  if(!ctx)
  {
    ret = libusb_init(&ctx);
    if(ret != LIBUSB_SUCCESS)
    {
      fprintf(stderr, "libusb_init: %s.\n", libusb_strerror(ret));
      return -1;
    }
  }

  if(!devs)
  {
    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0)
    {
      fprintf(stderr, "libusb_get_device_list: %s.\n", libusb_strerror(cnt));
      return -1;
    }
  }

  for(dev_i=0; dev_i<cnt; ++dev_i)
  {
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[dev_i], &desc);
    if(!ret)
    {
      if(desc.idVendor == controller[type].vendor && desc.idProduct == controller[type].product)
      {
        libusb_device_handle* devh;
        ret = libusb_open(devs[dev_i], &devh);
        if(ret != LIBUSB_SUCCESS)
        {
          fprintf(stderr, "libusb_open: %s.\n", libusb_strerror(ret));
          return -1;
        }
        else
        {
          ret = libusb_reset_device(devh);
          if(ret != LIBUSB_SUCCESS)
          {
            fprintf(stderr, "libusb_reset_device: %s.\n", libusb_strerror(ret));
            libusb_close(devh);
            return -1;
          }

#if defined(LIBUSB_API_VERSION) || defined(LIBUSBX_API_VERSION)
          libusb_set_auto_detach_kernel_driver(devh, 1);
#else
#ifndef WIN32
          ret = libusb_kernel_driver_active(devh, 0);
          if(ret == 1)
          {
            ret = libusb_detach_kernel_driver(devh, 0);
            if(ret != LIBUSB_SUCCESS)
            {
              fprintf(stderr, "libusb_detach_kernel_driver: %s.\n", libusb_strerror(ret));
              libusb_close(devh);
              return -1;
            }
          }
          else if(ret != LIBUSB_SUCCESS)
          {
            fprintf(stderr, "libusb_kernel_driver_active: %s.\n", libusb_strerror(ret));
            libusb_close(devh);
            return -1;
          }
#endif
#endif

          int config;

          ret = libusb_get_configuration(devh, &config);
          if(ret != LIBUSB_SUCCESS)
          {
            fprintf(stderr, "libusb_get_configuration: %s.\n", libusb_strerror(ret));
            libusb_close(devh);
            return -1;
          }

          if(config != controller[state->type].configuration)
          {
            ret = libusb_set_configuration(devh, controller[state->type].configuration);
            if(ret != LIBUSB_SUCCESS)
            {
              fprintf(stderr, "libusb_set_configuration: %s.\n", libusb_strerror(ret));
              libusb_close(devh);
              return -1;
            }
          }

          ret = libusb_claim_interface(devh, controller[state->type].interface);
          if(ret != LIBUSB_SUCCESS)
          {
            fprintf(stderr, "libusb_claim_interface: %s.\n", libusb_strerror(ret));
            libusb_close(devh);
          }
          else
          {
            state->devh = devh;
            ++nb_opened;

#ifndef WIN32
            const struct libusb_pollfd** pfd_usb = libusb_get_pollfds(ctx);

            int poll_i;
            for (poll_i=0; pfd_usb[poll_i] != NULL; ++poll_i)
            {
              GE_AddSource(pfd_usb[poll_i]->fd, usb_number, usb_handle_events, usb_handle_events, usb_close);
            }

            free(pfd_usb);
#endif

            if(state->type == C_TYPE_XONE_PAD && adapter_get(usb_number)->status)
            {
              //if the authentication was already performed, activate the controller

              //warning: make sure not to make any libusb async io before this!

              unsigned char activate[] = { 0x05, 0x20, 0x00, 0x01, 0x00 };
              usb_send_interrupt_out_sync(usb_number, activate, sizeof(activate));

              unsigned char activate_rumble[] = { 0x09, 0x00, 0x00, 0x09, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00 };
              usb_send_interrupt_out_sync(usb_number, activate_rumble, sizeof(activate_rumble));
            }

            // register joystick
            state->joystick_id = GE_RegisterJoystick(controller[state->type].name, NULL);

            int i;
            for(i = 0; i < controller[state->type].endpoints.in.reports.nb; ++i)
            {
              usb_states[usb_number].reports[i].report_id = controller[state->type].endpoints.in.reports.elements[i].report_id;
            }

            return 0;
          }
        }
      }
    }
  }

  return -1;
}

/*
 * Cancel all pending tranfers.
 */
static void cancel_transfers()
{
  unsigned int i;
  for (i = 0; i < transfers_nb; ++i)
  {
    libusb_cancel_transfer(transfers[i]);
  }
  while (transfers_nb)
  {
    if (libusb_handle_events(ctx) != LIBUSB_SUCCESS)
    {
      break;
    }
  }
  free(transfers);
  transfers = NULL;
  transfers_nb = 0;
}

int usb_close(int usb_number)
{
  struct usb_state* state = usb_states+usb_number;

  if(state->devh)
  {
    if(state->type == C_TYPE_XONE_PAD)
    {
      unsigned char power_off[] = { 0x05, 0x20, 0x00, 0x01, 0x04 };
      usb_send_interrupt_out_sync(usb_number, power_off, sizeof(power_off));
    }

    cancel_transfers();//TODO MLA: selective cancels

    libusb_release_interface(state->devh, controller[state->type].interface);
#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
#ifndef WIN32
    libusb_attach_kernel_driver(state->devh, 0);
#endif
#endif
    libusb_close(state->devh);
    state->devh = NULL;
    --nb_opened;
    if(!nb_opened)
    {
      libusb_free_device_list(devs, 1);
      devs = NULL;
      if(ctx) {
        libusb_exit(ctx);
        ctx = NULL;
      }
    }
  }

  set_done();

  return 1;
}

int usb_send_control(int usb_number, unsigned char* buffer, unsigned char length)
{
  struct usb_state* state = usb_states+usb_number;

  if(!state->devh)
  {
    fprintf(stderr, "no usb device opened for index %d\n", usb_number);
    return -1;
  }

  struct libusb_control_setup* control_setup = (struct libusb_control_setup*)buffer;
  if(control_setup->wLength > BUFFER_SIZE-LIBUSB_CONTROL_SETUP_SIZE)
  {
    fprintf(stderr, "wLength (%hu) is higher than %hu\n", control_setup->wLength, BUFFER_SIZE-LIBUSB_CONTROL_SETUP_SIZE);
    return -1;
  }

  unsigned int size = length;

  if(control_setup->bmRequestType & LIBUSB_ENDPOINT_IN)
  {
    size += control_setup->wLength;
  }

  unsigned char* buf = calloc(size, sizeof(char));
  if(!buf)
  {
    fprintf(stderr, "calloc failed\n");
    return -1;
  }

  memcpy(buf, buffer, length);

  struct libusb_transfer* transfer = libusb_alloc_transfer(0);

  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

  libusb_fill_control_transfer(transfer, state->devh, buf, (libusb_transfer_cb_fn)usb_callback, usb_state_indexes+usb_number, 1000);

  return submit_transfer(transfer);
}

int usb_send_interrupt_out(int usb_number, unsigned char* buffer, unsigned char length)
{
  struct usb_state* state = usb_states+usb_number;

  if(!state->devh)
  {
    fprintf(stderr, "no usb device opened for index %d\n", usb_number);
    return -1;
  }

  unsigned char* buf = calloc(length, sizeof(unsigned char));
  if(!buf)
  {
    fprintf(stderr, "calloc failed\n");
    return -1;
  }

  memcpy(buf, buffer, length);

  if(state->type == C_TYPE_XONE_PAD && length > 2)
  {
    buf[2] = state->counter++;
  }

  struct libusb_transfer* transfer = libusb_alloc_transfer(0);

  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

  libusb_fill_interrupt_transfer(transfer, state->devh, controller[state->type].endpoints.out.address, buf, length, (libusb_transfer_cb_fn)usb_callback, usb_state_indexes+usb_number, 1000);

  return submit_transfer(transfer);
}
