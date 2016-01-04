/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/usb_con.h>
#include <connectors/protocol.h>
#include <adapter.h>
#include <mainloop.h>
#include <report2event/report2event.h>
#include <gimx.h>
#include <gusb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define REPORTS_MAX 2

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
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
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
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
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
        .address = DS4_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = DS4_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
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
        .address = X360_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = X360_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
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
        .address = XONE_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = XONE_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
        .size = XONE_USB_INTERRUPT_PACKET_SIZE
      }
    }
  }
};

static struct usb_state {
  e_controller_type type;
  int usb_device;
  unsigned char ack;
  int joystick_id;
  struct
  {
    unsigned char report_id;
    s_report_packet report;
  } reports[REPORTS_MAX];
  unsigned char counter; // this is used for the Xbox One (interrupt out)
} usb_states[MAX_CONTROLLERS];

static void process_report(int usb_number, struct usb_state * state, unsigned char * buf, unsigned int count) {

  int i;
  for (i = 0; i < controller[state->type].endpoints.in.reports.nb; ++i) {
    unsigned char report_id = controller[state->type].endpoints.in.reports.elements[i].report_id;
    unsigned char report_length = controller[state->type].endpoints.in.reports.elements[i].report_length;
    if (buf[0] == report_id) {
      if (count == report_length) {
        if (state->type == C_TYPE_XONE_PAD && !adapter_get(usb_number)->status) {
          break;
        }

        s_report* current = (s_report*) buf;
        s_report* previous = &state->reports[i].report.value;

        report2event(state->type, usb_number, (s_report*) current, (s_report*) previous, state->joystick_id);

        if (state->type == C_TYPE_DS4 || state->type == C_TYPE_T300RS_PS4 || state->type == C_TYPE_G29_PS4) {
          memcpy(&previous->ds4, &current->ds4, report_length); //s_report_ds4 is larger than report_length bytes!
        } else if (state->type == C_TYPE_360_PAD) {
          previous->x360 = current->x360;
        } else if (state->type == C_TYPE_XONE_PAD) {
          if (report_id == XONE_USB_HID_IN_REPORT_ID) {
            previous->xone.input = current->xone.input;
          } else if (report_id == XONE_USB_HID_IN_GUIDE_REPORT_ID) {
            previous->xone.guide = current->xone.guide;
          }
        }
      } else {
        fprintf(stderr, "incorrect report length on interrupt endpoint: received %d bytes, expected %d bytes\n", count,
            report_length);
      }
      break;
    }
  }

  if (i == controller[state->type].endpoints.in.reports.nb) {
    if (state->type == C_TYPE_XONE_PAD && !adapter_get(usb_number)->status) {
      if (adapter_forward_interrupt_in(usb_number, buf, count) < 0) {
        fprintf(stderr, "can't forward interrupt data to the adapter\n");
      }
    }
  }
}

void usb_poll_interrupts() {

  unsigned int i;
  for (i = 0; i < MAX_CONTROLLERS; ++i) {
    struct usb_state * state = usb_states + i;
    if (state->usb_device >= 0 && state->ack) {
      int ret = gusb_poll(state->usb_device, controller[state->type].endpoints.in.address);
      if (ret != -1) {
        state->ack = 0;
      }
    }
  }
}

static int usb_read_callback(int user, unsigned char endpoint, const void * buf, int status) {

  struct usb_state * state = usb_states + user;

  if (endpoint != 0x00) {
    state->ack = 1;
  }

  if (status == E_TRANSFER_TIMED_OUT) {
    return 0;
  }

  if (status < 0) {
    return -1;
  }

  if (endpoint == 0x00) {

    if (status > BUFFER_SIZE) {
      PRINT_ERROR_OTHER("too many bytes transfered")
      return -1;
    }

    if (status > 0) {

      int ret = adapter_forward_control_in(user, (unsigned char *)buf, status);
      if (ret < 0) {
        return -1;
      }
    }

  } else {

    if (status > controller[state->type].endpoints.in.size) {
      PRINT_ERROR_OTHER("too many bytes transfered")
      return -1;
    }

    if (controller[state->type].endpoints.in.address != endpoint) {
      PRINT_ERROR_OTHER("wrong endpoint")
      return -1;
    }

    if (status > 0) {

      process_report(user, state, (unsigned char *)buf, status);
    }
  }

  return 0;
}

static int usb_write_callback(int user, unsigned char endpoint, int status) {

  struct usb_state * state = usb_states + user;

  if (status < 0) {
    return -1;
  }

  if (endpoint != 0x00) {

    if (controller[state->type].endpoints.out.address != endpoint) {
      PRINT_ERROR_OTHER("wrong endpoint")
      return -1;
    }
  }

  return 0;
}

static int usb_close_callback(int user) {

  return 1;
}

static int usb_send_interrupt_out_sync(int usb_number, unsigned char * buf, unsigned int count) {

  struct usb_state * state = usb_states + usb_number;

  if (state->type == C_TYPE_XONE_PAD && count > 2) {
    buf[2] = state->counter++;
  }

  int ret = gusb_write_timeout(state->usb_device, controller[state->type].endpoints.out.address, buf, count, 1000);
  if (ret < 0) {
    PRINT_ERROR_OTHER("error sending interrupt out");
    return -1;
  }

  return 0;
}

int usb_init(int usb_number, e_controller_type type) {

  int ret;

  struct usb_state * state = usb_states + usb_number;

  if(!controller[type].vendor || !controller[type].product) {
    gprintf(_("no pass-through device is needed\n"));
    return 0;
  }

  memset(state, 0x00, sizeof(*state));
  state->joystick_id = -1;
  state->type = type;
  state->ack = 1;

  state->usb_device = gusb_open_ids(controller[type].vendor, controller[type].product);

  if (state->usb_device < 0) {
    return -1;
  }

  if(state->type == C_TYPE_XONE_PAD && adapter_get(usb_number)->status) {

    //if the authentication was already performed, activate the controller

    //warning: make sure not to make any async io before this!

    unsigned char activate[] = { 0x05, 0x20, 0x00, 0x01, 0x00 };
    ret = usb_send_interrupt_out_sync(usb_number, activate, sizeof(activate));
    if (ret < 0) {
      usb_close(usb_number);
      return -1;
    }

    unsigned char activate_rumble[] = { 0x09, 0x00, 0x00, 0x09, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00 };
    ret = usb_send_interrupt_out_sync(usb_number, activate_rumble, sizeof(activate_rumble));
    if (ret < 0) {
      usb_close(usb_number);
      return -1;
    }
  }

  ret = gusb_register(state->usb_device, usb_number, usb_read_callback, usb_write_callback, usb_close_callback, gpoll_register_fd);
  if (ret < 0) {
    usb_close(usb_number);
    return -1;
  }

  ret = gusb_poll(state->usb_device, controller[state->type].endpoints.in.address);
  if (ret < 0) {
    usb_close(usb_number);
    return -1;
  }

  // register joystick
  state->joystick_id = ginput_register_joystick(controller[state->type].name, NULL);

  int i;
  for(i = 0; i < controller[state->type].endpoints.in.reports.nb; ++i) {
    usb_states[usb_number].reports[i].report_id = controller[state->type].endpoints.in.reports.elements[i].report_id;
  }

  return 0;
}

int usb_close(int usb_number) {

  struct usb_state * state = usb_states + usb_number;

  if (state->usb_device >= 0) {
    if (state->type == C_TYPE_XONE_PAD) {
      unsigned char power_off[] = { 0x05, 0x20, 0x00, 0x01, 0x04 };
      usb_send_interrupt_out_sync(usb_number, power_off, sizeof(power_off));
    }

    gusb_close(state->usb_device);
  }

  set_done();

  return 1;
}

int usb_send_control(int usb_number, void * buf, unsigned int count) {

  return gusb_write(usb_states[usb_number].usb_device, 0x00, buf, count);
}

int usb_send_interrupt_out(int usb_number, void * buf, unsigned int count) {

  struct usb_state * state = usb_states + usb_number;

  if (state->type == C_TYPE_XONE_PAD && count > 2) {
    ((unsigned char *) buf)[2] = state->counter++;
  }

  return gusb_write(state->usb_device, controller[state->type].endpoints.out.address, buf, count);
}
