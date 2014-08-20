/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <connectors/usb_con.h>
#include <adapter.h>
#include <mainloop.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

//warning: ugly hack inside!
typedef union {
  void *ptr;
  int i;
} intptr;

static libusb_context* ctx = NULL;
static libusb_device** devs = NULL;
static ssize_t cnt = 0;
static int nb_opened = 0;

static struct usb_state {
  libusb_device_handle* devh;
  libusb_transfer_cb_fn fp;
} usb_states[MAX_CONTROLLERS];

void usb_callback(struct libusb_transfer* transfer)
{
  intptr cp = { .ptr = transfer->user_data };

  struct libusb_control_setup* setup = libusb_control_transfer_get_setup(transfer);

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
        if(adapter_forward_data_in(cp.i, data, transfer->actual_length) < 0)
        {
          fprintf(stderr, "can't forward data to the adapter\n");
        }
      }
    }
  }
  else
  {
    fprintf(stderr, "libusb_transfer failed with status %d (bmRequestType=0x%02x, bRequest=0x%02x, wValue=0x%04x)\n", transfer->status, setup->bmRequestType, setup->bRequest, setup->wValue);
  }
}

int usb_handle_events(int unused)
{
#ifndef WIN32
  return libusb_handle_events(ctx);
#else
  struct timeval tv = {};
  return libusb_handle_events_timeout(ctx, &tv);
#endif
}

int usb_init(int usb_number, unsigned short vendor, unsigned short product, libusb_transfer_cb_fn fp)
{
  int ret = -1;
  int dev_i;

  struct usb_state* state = usb_states+usb_number;

  memset(state, 0x00, sizeof(*state));

  if(!ctx)
  {
    ret = libusb_init(&ctx);
    if(ret < 0)
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
      if(desc.idVendor == vendor && desc.idProduct == product)
      {
        libusb_device_handle* devh;
        ret = libusb_open(devs[dev_i], &devh);
        if(ret < 0)
        {
          fprintf(stderr, "libusb_open: %s.\n", libusb_strerror(cnt));
          return -1;
        }
        else
        {
          libusb_set_auto_detach_kernel_driver(devh, 1);

          ret = libusb_claim_interface(devh, 0);
          if(ret < 0)
          {
            fprintf(stderr, "libusb_claim_interface: %s.\n", libusb_strerror(ret));
            libusb_close(devh);
          }
          else
          {
            state->devh = devh;
            state->fp = fp;
            ++nb_opened;

#ifndef WIN32
            const struct libusb_pollfd** pfd_usb = libusb_get_pollfds(ctx);

            int poll_i;
            for (poll_i=0; pfd_usb[poll_i] != NULL; ++poll_i)
            {
              GE_AddSource(pfd_usb[poll_i]->fd, usb_number, usb_handle_events, usb_handle_events, usb_close);
            }
#endif

            return 0;
          }
        }
      }
    }
  }

  return -1;
}

int usb_close(int usb_number)
{
  struct usb_state* state = usb_states+usb_number;

  if(state->devh)
  {
    libusb_release_interface(state->devh, 0);
    libusb_close(state->devh);
    state->devh = NULL;
    --nb_opened;
    if(!nb_opened)
    {
      libusb_free_device_list(devs, 1);
      devs = NULL;
      libusb_exit(ctx);
      ctx = NULL;
    }
  }

  set_done();

  return 1;
}

int usb_send(int usb_number, unsigned char* buffer, unsigned char length)
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

  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER ;

  intptr cp = { .i = usb_number };

  libusb_fill_control_transfer(transfer, state->devh, buf, state->fp, cp.ptr, 1000);

  int ret = libusb_submit_transfer(transfer);
  if(ret < 0)
  {
    fprintf(stderr, "libusb_submit_transfer: %s.\n", libusb_strerror(ret));
    free(transfer->buffer);
    libusb_free_transfer(transfer);
    return -1;
  }

  return 0;
}
