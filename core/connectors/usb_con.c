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

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

static libusb_context* ctx = NULL;
static libusb_device** devs = NULL;
static ssize_t cnt = 0;
static int nb_opened = 0;

static int usb_state_indexes[MAX_CONTROLLERS] = {};

static struct usb_state {
  libusb_device_handle* devh;
  unsigned char ack;
  int device_id;
  s_report report;
} usb_states[MAX_CONTROLLERS];

static int usb_poll_interrupt(int usb_number);

static inline unsigned char hatToButtons(const unsigned char hat)
{
  unsigned char buttons = 0x00;
  switch(hat)
  {
    case 0x00:
      buttons = DS4_UP_MASK;
      break;
    case 0x01:
      buttons = DS4_UP_MASK | DS4_RIGHT_MASK;
      break;
    case 0x02:
      buttons = DS4_RIGHT_MASK;
      break;
    case 0x03:
      buttons = DS4_DOWN_MASK | DS4_RIGHT_MASK;
      break;
    case 0x04:
      buttons = DS4_DOWN_MASK;
      break;
    case 0x05:
      buttons = DS4_DOWN_MASK | DS4_LEFT_MASK;
      break;
    case 0x06:
      buttons = DS4_LEFT_MASK;
      break;
    case 0x07:
      buttons = DS4_UP_MASK | DS4_LEFT_MASK;
      break;
    case 0x08:
      break;
  }
  return buttons;
}

static int (*event_callback)(GE_Event*) = NULL;

void usb_set_event_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

void usb_callback(struct libusb_transfer* transfer)
{
  int usb_number = *(int*)transfer->user_data;
  struct usb_state* state = usb_states+usb_number;

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
          if(adapter_forward_data_in(usb_number, data, transfer->actual_length) < 0)
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
  else if(transfer->type == LIBUSB_TRANSFER_TYPE_INTERRUPT)
  {
    state->ack = 1;

    if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
      // process joystick events
      if(transfer->actual_length == DS4_USB_INTERRUPT_PACKET_SIZE)
      {
        s_report_ds4* report = (s_report_ds4*) transfer->buffer;

        GE_Event event = { .jbutton.which = state->device_id };

        unsigned short value;

        unsigned char hatAndButtons = report->HatAndButtons;
        unsigned char prevHatAndButtons = state->report.value.ds4.HatAndButtons;

        unsigned short buttonsAndCounter = report->ButtonsAndCounter;
        unsigned short prevButtonsAndCounter = state->report.value.ds4.ButtonsAndCounter;

        unsigned char dirButtons = hatToButtons(hatAndButtons & 0x0F);
        unsigned char prevDirButtons = hatToButtons(prevHatAndButtons & 0x0F);

        if((value = (dirButtons & DS4_UP_MASK)) ^ (prevDirButtons & DS4_UP_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_UP_ID;
          event_callback(&event);
        }
        if((value = (dirButtons & DS4_RIGHT_MASK)) ^ (prevDirButtons & DS4_RIGHT_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_RIGHT_ID;
          event_callback(&event);
        }
        if((value = (dirButtons & DS4_DOWN_MASK)) ^ (prevDirButtons & DS4_DOWN_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_DOWN_ID;
          event_callback(&event);
        }
        if((value = (dirButtons & DS4_LEFT_MASK)) ^ (prevDirButtons & DS4_LEFT_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_LEFT_ID;
          event_callback(&event);
        }

        if((value = (hatAndButtons & DS4_SQUARE_MASK)) ^ (prevHatAndButtons & DS4_SQUARE_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_SQUARE_ID;
          event_callback(&event);
        }
        if((value = (hatAndButtons & DS4_CROSS_MASK)) ^ (prevHatAndButtons & DS4_CROSS_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_CROSS_ID;
          event_callback(&event);
        }
        if((value = (hatAndButtons & DS4_CIRCLE_MASK)) ^ (prevHatAndButtons & DS4_CIRCLE_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_CIRCLE_ID;
          event_callback(&event);
        }
        if((value = (hatAndButtons & DS4_TRIANGLE_MASK)) ^ (prevHatAndButtons & DS4_TRIANGLE_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_TRIANGLE_ID;
          event_callback(&event);
        }

        if((value = (buttonsAndCounter & DS4_L1_MASK)) ^ (prevButtonsAndCounter & DS4_L1_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_L1_ID;
          event_callback(&event);
        }
        if((value = (buttonsAndCounter & DS4_R1_MASK)) ^ (prevButtonsAndCounter & DS4_R1_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_R1_ID;
          event_callback(&event);
        }
#ifndef WIN32
        if((value = (buttonsAndCounter & DS4_L2_MASK)) ^ (prevButtonsAndCounter & DS4_L2_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_L2_ID;
          event_callback(&event);
        }
        if((value = (buttonsAndCounter & DS4_R2_MASK)) ^ (prevButtonsAndCounter & DS4_R2_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_R2_ID;
          event_callback(&event);
        }
#endif

        if((value = (buttonsAndCounter & DS4_SHARE_MASK)) ^ (prevButtonsAndCounter & DS4_SHARE_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_SHARE_ID;
          event_callback(&event);
        }
        if((value = (buttonsAndCounter & DS4_OPTIONS_MASK)) ^ (prevButtonsAndCounter & DS4_OPTIONS_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_OPTIONS_ID;
          event_callback(&event);
        }
        if((value = (buttonsAndCounter & DS4_L3_MASK)) ^ (prevButtonsAndCounter & DS4_L3_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_L3_ID;
          event_callback(&event);
        }
        if((value = (buttonsAndCounter & DS4_R3_MASK)) ^ (prevButtonsAndCounter & DS4_R3_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_R3_ID;
          event_callback(&event);
        }

        if((value = (buttonsAndCounter & DS4_PS_MASK)) ^ (prevButtonsAndCounter & DS4_PS_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_PS_ID;
          event_callback(&event);
        }
#ifndef WIN32
        if((value = (buttonsAndCounter & DS4_TOUCHPAD_MASK)) ^ (prevButtonsAndCounter & DS4_TOUCHPAD_MASK))
        {
          event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
          event.jbutton.button = DS4_TOUCHPAD_ID;
          event_callback(&event);
        }
#endif

        event.type = GE_JOYAXISMOTION;

        int axisValue;
        int prevAxisValue;

        axisValue = report->X;
        prevAxisValue = state->report.value.ds4.X;
        if(axisValue != prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_X_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }
        axisValue = report->Y;
        prevAxisValue = state->report.value.ds4.Y;
        if(axisValue != prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_Y_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }
        axisValue = report->Z;
        prevAxisValue = state->report.value.ds4.Z;
        if(axisValue != prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_Z_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }
        axisValue = report->Rz;
        prevAxisValue = state->report.value.ds4.Rz;
        if(axisValue != prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_RZ_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }
        axisValue = report->Rx;
        prevAxisValue = state->report.value.ds4.Rx;
        if(axisValue != prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_L2_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }
        axisValue = report->Ry;
        prevAxisValue = state->report.value.ds4.Ry;
        if(axisValue || prevAxisValue)
        {
          event.jaxis.axis = DS4_AXIS_R2_ID;
          axisValue = (axisValue - 0x80) * 32765 / 127;
          event.jaxis.value = clamp(-32768, axisValue, 32767);
          event_callback(&event);
        }

        state->report.value.ds4 = *report;
      }
      else
      {
        fprintf(stderr, "incorrect packet size on interrupt endpoint\n");
      }
    }
    else
    {
      fprintf(stderr, "libusb_transfer failed with status %d\n", transfer->status);
    }
  }
}

static int usb_poll_interrupt(int usb_number)
{
  struct usb_state* state = usb_states+usb_number;
  struct libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER ;
  unsigned int size = DS4_USB_INTERRUPT_PACKET_SIZE;
  unsigned char* buf = calloc(size, sizeof(char));
  libusb_fill_interrupt_transfer(transfer, state->devh, DS4_USB_INTERRUPT_ENDPOINT_IN | LIBUSB_ENDPOINT_IN, buf, size, (libusb_transfer_cb_fn)usb_callback, usb_state_indexes+usb_number, 1000);
  int ret = libusb_submit_transfer(transfer);
  if(ret < 0)
  {
    fprintf(stderr, "libusb_submit_transfer: %s.\n", libusb_strerror(ret));
    free(transfer->buffer);
    libusb_free_transfer(transfer);
  }
  else
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
    struct timeval tv = {};
    return libusb_handle_events_timeout(ctx, &tv);
  }
  else
  {
    return 0;
  }
#endif
}

int usb_init(int usb_number, unsigned short vendor, unsigned short product)
{
  int ret = -1;
  int dev_i;

  struct usb_state* state = usb_states+usb_number;

  usb_state_indexes[usb_number] = usb_number;

  memset(state, 0x00, sizeof(*state));
  state->device_id = -1;

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
#if defined(LIBUSB_API_VERSION) || defined(LIBUSBX_API_VERSION)
          libusb_set_auto_detach_kernel_driver(devh, 1);
#else
#ifndef WIN32
          if(libusb_kernel_driver_active(devh, 0))
          {
            ret = libusb_detach_kernel_driver(devh, 0);
            if(ret < 0)
            {
              fprintf(stderr, "libusb_detach_kernel_driver: %s.\n", libusb_strerror(ret));
              libusb_close(devh);
              return -1;
            }
          }
#endif
#endif

          ret = libusb_claim_interface(devh, 0);
          if(ret < 0)
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
#endif
            if(usb_poll_interrupt(usb_number) == LIBUSB_SUCCESS)
            {
              // register joystick
              int device_id = GE_RegisterJoystick(DS4_DEVICE_NAME);
              if(device_id >= 0)
              {
                state->device_id = device_id;
              }
            }

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

  transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

  libusb_fill_control_transfer(transfer, state->devh, buf, (libusb_transfer_cb_fn)usb_callback, usb_state_indexes+usb_number, 1000);

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
