/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/usb_con.h>
#include <connectors/protocol.h>
#include <controller.h>
#include <mainloop.h>
#include <connectors/report2event/report2event.h>
#include <gimx.h>
#include <gimxusb/include/gusb.h>
#include <gimxtime/include/gtime.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define REPORTS_MAX 2

static struct
{
  const char* name;
  struct {
      uint16_t vendor;
      uint16_t product;
  } ids;
  int configuration;
  struct
  {
    struct
    {
      uint8_t address;
      uint16_t size;
      struct
      {
        uint8_t nb;
        struct
        {
          uint8_t report_id;
          uint8_t report_length;
        } elements[REPORTS_MAX];
      } reports;
    } in;
    struct
    {
      uint8_t address;
      uint16_t size;
    } out;
  } endpoints;
} controller[C_TYPE_MAX][3] =
{
  [C_TYPE_DS4] =
  { {
    .name = DS4_DEVICE_NAME,
    .ids =
        { .vendor = DS4_VENDOR, .product = DS4_PRODUCT },
    .configuration = 1,
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
  {
    .name = DS4_DEVICE_NAME,
    .ids =
        { .vendor = DS4_VENDOR, .product = DS4_PRODUCT_2 },
    .configuration = 1,
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
  }, },
  [C_TYPE_T300RS_PS4] =
  { {
    .name = DS4_DEVICE_NAME,
    .ids =
        { .vendor = DS4_VENDOR, .product = DS4_PRODUCT },
    .configuration = 1,
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
  {
    .name = DS4_DEVICE_NAME,
    .ids =
        { .vendor = DS4_VENDOR, .product = DS4_PRODUCT_2 },
    .configuration = 1,
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
  }, },
  [C_TYPE_G29_PS4] =
  { {
    .name = DS4_DEVICE_NAME,
    .ids =
        { .vendor = DS4_VENDOR, .product = DS4_PRODUCT },
    .configuration = 1,
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
  {
  .name = DS4_DEVICE_NAME,
  .ids =
      { .vendor = DS4_VENDOR, .product = DS4_PRODUCT_2 },
  .configuration = 1,
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
  }, },
  [C_TYPE_360_PAD] =
  { {
    .name = X360_NAME,
    .ids =
        { .vendor = X360_VENDOR, .product = X360_PRODUCT },
    .configuration = 1,
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
  }, },
  [C_TYPE_XONE_PAD] =
  { {
    .name = XONE_NAME,
    .ids =
        { .vendor = XONE_VENDOR, .product = XONE_PRODUCT },
    .configuration = 1,
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
  },
  {
    .name = XONE_NAME_2,
    .ids =
        { .vendor = XONE_VENDOR, .product = XONE_PRODUCT_2 },
    .configuration = 1,
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
  },
  {
    .name = XONE_NAME_S,
    .ids =
        { .vendor = XONE_VENDOR, .product = XONE_PRODUCT_S },
    .configuration = 1,
    .endpoints =
    {
      .in =
      {
        .address = XONE_S_USB_INTERRUPT_ENDPOINT_IN | USB_DIR_IN,
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
        .address = XONE_S_USB_INTERRUPT_ENDPOINT_OUT | USB_DIR_OUT,
        .size = XONE_USB_INTERRUPT_PACKET_SIZE
      }
    }
  } },
};

static struct usb_state {
  e_controller_type type;
  int index;
  struct gusb_device * usb_device;
  unsigned char ack;
  int joystick_id;
  struct
  {
    unsigned char report_id;
    s_report_packet report;
  } reports[REPORTS_MAX];
  unsigned char counter; // this is used for the Xbox One (interrupt out)
  unsigned char disconnected;
} usb_states[MAX_CONTROLLERS];

static void dump(unsigned char * packet, unsigned char length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    if(i && !(i%8))
    {
      ginfo("\n");
    }
    ginfo("0x%02x ", packet[i]);
  }
  ginfo("\n");
}

#define DEBUG_PACKET(TYPE, DATA, LENGTH) \
  do { \
    if(gimx_params.debug.usb_con) \
    { \
      ginfo("> %s\n", __func__); \
      ginfo("type: 0x%02x\n", TYPE); \
      dump(DATA, LENGTH); \
    } \
  } while (0)

static void process_report(int usb_number, struct usb_state * state, unsigned char * buf, unsigned int count) {

  DEBUG_PACKET(BYTE_IN_REPORT, buf, count);

  int i;
  for (i = 0; i < controller[state->type][state->index].endpoints.in.reports.nb; ++i) {
    unsigned char report_id = controller[state->type][state->index].endpoints.in.reports.elements[i].report_id;
    unsigned char report_length = controller[state->type][state->index].endpoints.in.reports.elements[i].report_length;
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
            if (controller[state->type][state->index].ids.product == XONE_PRODUCT_S) {
              uint8_t guide_ack[] = {
                  0x01, 0x20, buf[2], 0x09, 0x00, 0x07, 0x20, 0x02,
                  0x00, 0x00, 0x00, 0x00, 0x00
              };
              usb_send_interrupt_out(usb_number, guide_ack, sizeof(guide_ack));
            }
          }
        }
      } else {
        gwarn("incorrect report length on interrupt endpoint: received %d bytes, expected %d bytes\n", count,
            report_length);
      }
      break;
    }
  }

  if (state->type == C_TYPE_XONE_PAD && buf[0] == 0x02 && buf[1] == 0x20) {
      state->counter = buf[2];
      static uint8_t desc[] = {
              0x02, 0x20, 0x01, 0x1c, 0x7e, 0xed, 0x82, 0x8b,
              0xec, 0x97, 0x00, 0x00, 0x5e, 0x04, 0xd1, 0x02,
              0x02, 0x00, 0x03, 0x00, 0x4d, 0x09, 0x00, 0x00,
              0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
      };
      memcpy(buf, desc, sizeof(desc));
      buf[2] = state->counter;
  }

  if (i == controller[state->type][state->index].endpoints.in.reports.nb) {
    if (state->type == C_TYPE_XONE_PAD && !adapter_get(usb_number)->status) {
      if (adapter_forward_interrupt_in(usb_number, buf, count) < 0) {
        gwarn("can't forward interrupt data to the adapter\n");
      }
    }
  }
}

int usb_poll_interrupts() {

  int status = 0;

  unsigned int i;
  for (i = 0; i < MAX_CONTROLLERS; ++i) {
    struct usb_state * state = usb_states + i;
    if (state->disconnected)
    {
      status = -1;
      continue;
    }
    if (state->usb_device != NULL && state->ack) {
      int ret = gusb_poll(state->usb_device, controller[state->type][state->index].endpoints.in.address);
      if (ret != -1) {
        state->ack = 0;
      } else {
        status = -1;
      }
    }
  }

  return status;
}

static int usb_read_callback(void * user, unsigned char endpoint, const void * buf, int status) {

  int adapter = (intptr_t) user;

  struct usb_state * state = usb_states + adapter;

  if (endpoint != 0x00) {
    state->ack = 1;
  }

  if (status == E_STATUS_TRANSFER_TIMED_OUT) {
    return 0;
  }

  if (status == E_STATUS_NO_DEVICE) {
    state->disconnected = 1;
    return -1;
  }

  if (endpoint == 0x00) {

    if (status > BUFFER_SIZE) {
      PRINT_ERROR_OTHER("too many bytes transfered");
      return -1;
    }

    // in case of error, send 0 to prevent adapter deadlock

    int ret = adapter_forward_control_in(adapter, (unsigned char *)buf, status >= 0 ? status : 0);
    if (ret < 0) {
      return -1;
    }

  } else {

    if (status > controller[state->type][state->index].endpoints.in.size) {
      PRINT_ERROR_OTHER("too many bytes transfered");
      return -1;
    }

    if (controller[state->type][state->index].endpoints.in.address != endpoint) {
      PRINT_ERROR_OTHER("wrong endpoint");
      return -1;
    }

    if (status >= 0) {

      process_report(adapter, state, (unsigned char *)buf, status);
    }
  }

  return 0;
}

static int usb_write_callback(void * user, unsigned char endpoint, int status) {

  struct usb_state * state = usb_states + (intptr_t) user;

  if (status == E_STATUS_NO_DEVICE) {
    state->disconnected = 1;
    return -1;
  }

  if (endpoint != 0x00) {

    if (controller[state->type][state->index].endpoints.out.address != endpoint) {
      PRINT_ERROR_OTHER("wrong endpoint");
      return -1;
    }
  }

  return 0;
}

static int usb_close_callback(void * user __attribute__((unused))) {

  // TODO MLA: anything to do here?

  return 1;
}

static int usb_send_interrupt_out_sync(int usb_number, unsigned char * buf, unsigned int count) {

  struct usb_state * state = usb_states + usb_number;

  int ret = gusb_write_timeout(state->usb_device, controller[state->type][state->index].endpoints.out.address, buf, count, 1000);
  if (ret < 0) {
    PRINT_ERROR_OTHER("error sending interrupt out");
    return -1;
  }

  return 0;
}

int usb_init(int usb_number, e_controller_type type) {

  int ret;

  struct usb_state * state = usb_states + usb_number;

  memset(state, 0x00, sizeof(*state));
  state->joystick_id = -1;
  state->type = type;
  state->ack = 1;

  if(!controller[type][0].ids.vendor || !controller[type][0].ids.product) {
    ginfo(_("no pass-through device is needed\n"));
    return 0;
  }

  unsigned int i;
  for (i = 0; i < sizeof(controller[0]) / sizeof(*controller[0]); ++i)
  {
    if (controller[type][i].ids.vendor == 0x0000 || controller[type][i].ids.product == 0x0000 )
    {
      break;
    }
    state->usb_device = gusb_open_ids(controller[type][i].ids.vendor, controller[type][i].ids.product);

    if (state->usb_device != NULL && type == C_TYPE_XONE_PAD) {

#ifdef WIN32
      if (controller[type][i].ids.product == XONE_PRODUCT_S)
      {
        char * path = strdup(gusb_get_path(state->usb_device));
        gusb_close(state->usb_device);

        gtime start = gtime_gettime();
        gtime now = start;

        do
        {
          state->usb_device = gusb_open_path(path);
          if (gimx_params.debug.usb_con)
          {
            ginfo("path %s device %p time %lldms\n", path, state->usb_device, GTIME_USEC(now - start) / 1000UL);
          }
          now = gtime_gettime();
        }
        while (state->usb_device == NULL && GTIME_USEC(now - start) < 2000000);

        free(path);
      }
#endif

      unsigned char power_off[] = { 0x05, 0x20, 0x00, 0x01, 0x04 };

      int ret = gusb_write_timeout(state->usb_device, controller[type][i].endpoints.out.address, power_off, sizeof(power_off), 1000);
      if (ret < 0) {
        PRINT_ERROR_OTHER("error sending interrupt out");
        gusb_close(state->usb_device);
        return -1;
      }
    }

    if (state->usb_device != NULL) {
      ginfo(_("found pass-through device 0x%04x:0x%04x\n"), controller[type][i].ids.vendor, controller[type][i].ids.product);
      state->index = i;
      break;
    }
  }

  if (state->usb_device == NULL) {
    return -1;
  }

  GUSB_CALLBACKS callbacks = {
          .fp_read = usb_read_callback,
          .fp_write = usb_write_callback,
          .fp_close = usb_close_callback,
          .fp_register = REGISTER_FUNCTION,
          .fp_remove = REMOVE_FUNCTION,
  };
  ret = gusb_register(state->usb_device, (void *)(intptr_t) usb_number, &callbacks);
  if (ret < 0) {
    usb_close(usb_number);
    return -1;
  }

  ret = gusb_poll(state->usb_device, controller[state->type][state->index].endpoints.in.address);
  if (ret < 0) {
    usb_close(usb_number);
    return -1;
  }

  // register joystick
  state->joystick_id = ginput_register_joystick(controller[state->type][state->index].name, GE_HAPTIC_NONE, NULL);

  ginfo("registered joystick %d with name \"%s\"\n", state->joystick_id, controller[state->type][state->index].name);

  for(i = 0; i < controller[state->type][state->index].endpoints.in.reports.nb; ++i) {
    usb_states[usb_number].reports[i].report_id = controller[state->type][state->index].endpoints.in.reports.elements[i].report_id;
  }

  return 0;
}

int usb_close(int usb_number) {

  struct usb_state * state = usb_states + usb_number;

  if (state->usb_device != NULL) {
    if (state->type == C_TYPE_XONE_PAD) {
      unsigned char power_off[] = { 0x05, 0x20, 0x00, 0x01, 0x04 };
      usb_send_interrupt_out_sync(usb_number, power_off, sizeof(power_off));
    }

    gusb_close(state->usb_device);

    state->usb_device = NULL;
  }

  set_done();

  return 1;
}

int usb_send_control(int usb_number, void * buf, unsigned int count) {

  return gusb_write(usb_states[usb_number].usb_device, 0x00, buf, count);
}

int usb_send_interrupt_out(int usb_number, void * buf, unsigned int count) {

  struct usb_state * state = usb_states + usb_number;

  return gusb_write(state->usb_device, controller[state->type][state->index].endpoints.out.address, buf, count);
}

int usb_forward_output(int usb_number, int joystick) {

  switch (usb_states[usb_number].type) {
  case C_TYPE_XONE_PAD:
  case C_TYPE_360_PAD:
  case C_TYPE_DS4:
  case C_TYPE_SIXAXIS:
      return joystick >= 0 && usb_states[usb_number].joystick_id == joystick;
  case C_TYPE_JOYSTICK:
  case C_TYPE_PS2_PAD:
  case C_TYPE_XBOX_PAD:
  case C_TYPE_T300RS_PS4:
  case C_TYPE_G27_PS3:
  case C_TYPE_G29_PS4:
  case C_TYPE_DF_PS2:
  case C_TYPE_DFP_PS2:
  case C_TYPE_GTF_PS2:
  case C_TYPE_G920_XONE:
  case C_TYPE_SWITCH:
  case C_TYPE_NONE:
      break;
  }
  return 0;
}
