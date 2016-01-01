/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#ifndef WIN32
#include <poll.h>
#include <arpa/inet.h> /* for htons */
#endif
#include <errno.h>
#include <ginput.h>
#include <connectors/sixaxis.h>
#include <connectors/bluetooth/bt_device_abs.h>
#include <connectors/bluetooth/l2cap_abs.h>
#include <config.h>
#include "gimx.h"
#include <adapter.h>

#define DS3_DEVICE_CLASS 0x508

#define HID_HANDSHAKE 0x0
#define HID_GET_REPORT 0x4
#define HID_SET_REPORT 0x5
#define HID_DATA 0xA

#define HID_TYPE_RESERVED 0
#define HID_TYPE_INPUT 1
#define HID_TYPE_OUTPUT 2
#define HID_TYPE_FEATURE 3

enum led_state_t { LED_OFF = 0, LED_FLASH, LED_ON };

struct sixaxis_state_sys {
    /*** Values provided by the system (PS3): */
    int reporting_enabled;
    int shutdown;
    int feature_ef_byte_6;

    /* led[0] is the spare */
    /* led[1]..led[4] correspond to printed text 1..4 */
    enum led_state_t led[5];
    uint8_t rumble[2];
};

struct sixaxis_state {
    struct {
      char str[18];
      bdaddr_t ba;
    } bdaddr_src;
    char bdaddr_dst[18];
    int dongle_index;
    int sixaxis_number;
    struct sixaxis_state_sys sys;
    s_report_ds3 user;
    struct
    {
      struct
      {
        int id;
        int pending;
      } control;
      struct
      {
        int id;
        int pending;
      } interrupt;
    } channels;
};

struct sixaxis_assemble_t {
    int type;
    uint8_t report;
    int (*func)(uint8_t *buf, int maxlen, struct sixaxis_state *state);
};

struct sixaxis_process_t {
    int type;
    uint8_t report;
    int (*func)(const uint8_t *buf, int len, struct sixaxis_state *state);
};

static struct sixaxis_state states[MAX_CONTROLLERS] = {};
static int debug = 0;

static unsigned char buf[1024];

static const char *hid_report_name[] =
{ "reserved", "input", "output", "feature" };

static void sixaxis_init(int sixaxis_number)
{
  struct sixaxis_state* state = states+sixaxis_number;

  state->sys.reporting_enabled = 0;
  state->sys.feature_ef_byte_6 = 0xb0;

  state->channels.control.id = -1;
  state->channels.interrupt.id = -1;

  state->user.X = 128;
  state->user.Y = 128;
  state->user.Z = 128;
  state->user.Rz = 128;
  state->user.acc_x[0] = 512 >> 8;
  state->user.acc_x[1] = 512 & 0xFF;
  state->user.acc_y[0] = 512 >> 8;
  state->user.acc_y[1] = 512 & 0xFF;
  state->user.acc_z[0] = 400 >> 8;
  state->user.acc_z[1] = 400 & 0xFF;
  state->user.gyro[0] = 512 >> 8;
  state->user.gyro[1] = 512 & 0xFF;
}

/* Main input report from Sixaxis -- assemble it */
static int assemble_input_01(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  if ((unsigned int)maxlen < sizeof(s_report_ds3) - 1)
    return -1;

  memcpy(buf, ((uint8_t*) &state->user) + 1, sizeof(s_report_ds3) - 1);

  buf[30] = 0x16; //bluetooth+rumble off (0x14 = on)

  /* Unknown, some values fluctuate? */
  buf[35] = 0x33;
  buf[36] = 0xfa;
  buf[37] = 0x77;
  buf[38] = 0x01;
  buf[39] = 0xc0; //rumble off (0x40 = on)

  return sizeof(s_report_ds3) - 1;
}

/* Main input report from Sixaxis -- decode it */
static int process_input_01(const uint8_t *buf, int len, struct sixaxis_state *state)
{
  if ((unsigned int) len < sizeof(s_report_ds3))
    return -1;

  memcpy(&state->user, buf, sizeof(s_report_ds3));

  return 0;
}

/* Unknown */
static int assemble_feature_01(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  uint8_t data[] =
  { 0x01, 0x03, 0x00, 0x04, 0x0c, 0x01, 0x02, 0x18, 0x18, 0x18, 0x18, 0x09,
      0x0a, 0x10, 0x11, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x02,
      0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00,
      0x02, 0x01, 0x02, 0x00, 0x64, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00 };
  data[3] += state->sixaxis_number;
  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

/* Unknown */
static int assemble_feature_ef(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  const uint8_t data[] =
  { 0xef, 0x04, 0x00, 0x05, 0x03, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, /* acc_x center & radius? */
      0x02, 0x00, 0x03, 0xff, /* acc_y center & radius? */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  /* Byte 6 must match the byte set by the PS3 */
  buf[6] = state->sys.feature_ef_byte_6;

  return len;
}

/* Unknown */
static int assemble_feature_f2(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  uint8_t data[] =
  {
    0xff, 0xff,
    0x00, 0x00, 0x1e, 0x3d, 0x24, 0x97, 0xde, /* device bdaddr */
    0x00, 0x03, 0x50, 0x89, 0xc0, 0x01, 0x8a, 0x09
  };
  data[3] = state->bdaddr_src.ba.b[5];
  data[4] = state->bdaddr_src.ba.b[4];
  data[5] = state->bdaddr_src.ba.b[3];
  data[6] = state->bdaddr_src.ba.b[2];
  data[7] = state->bdaddr_src.ba.b[1];
  data[8] = state->bdaddr_src.ba.b[0];
  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

/* Unknown */
static int assemble_feature_f7(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  const uint8_t data[] =
  { 0x00, 0x02, 0xe4, 0x02, 0xa9, 0x01, 0x05, 0xff, 0x14, 0x23, 0x00 };
  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

/* Unknown */
static int assemble_feature_f8(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
  const uint8_t data[] =
  { 0x01, 0x00, 0x00, 0x00 };
  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

/* Main output report from PS3 including rumble and LEDs */
static int process_output_01(const uint8_t *buf, int len, struct sixaxis_state *state)
{
  int i;

  if (len < 48)
    return -1;

  /* Decode LEDs.  This ignores a lot of details of the LED bits */
  for (i = 0; i < 5; i++)
  {
    if (buf[9] & (1 << i))
    {
      if (buf[13 + 5 * i] == 0)
        state->sys.led[i] = LED_ON;
      else
        state->sys.led[i] = LED_FLASH;
    }
    else
    {
      state->sys.led[i] = LED_OFF;
    }
  }

  /* Decode rumble.  Again, ignores some details */
  state->sys.rumble[0] = buf[1] ? buf[2] : 0;
  state->sys.rumble[1] = buf[3] ? buf[4] : 0;

  int controller = (state-states)/sizeof(*state);
  int joystick = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, controller);

  if(ginput_joystick_has_rumble(joystick))
  {
    GE_Event event =
    {
      .jrumble =
      {
        .type = GE_JOYRUMBLE,
        .which = joystick,
        .weak = buf[2] << 8,
        .strong = buf[4] << 8
      }
    };
    ginput_queue_push(&event);
  }

  return 0;
}

/* Unknown */
static int process_feature_ef(const uint8_t *buf, int len, struct sixaxis_state *state)
{
  if (len < 7)
    return -1;
  /* Need to remember byte 6 for assemble_feature_ef */
  state->sys.feature_ef_byte_6 = buf[6];
  return 0;
}

/* Enable reporting */
static int process_feature_f4(const uint8_t *buf, int len, struct sixaxis_state *state)
{
  /* Enable event reporting */
  if (buf[1] == 0x08)
  {
    state->sys.shutdown = 1;
  }
  else
  {
    state->sys.reporting_enabled = 1;
  }
  return 0;
}

static struct sixaxis_assemble_t sixaxis_assemble[] =
{
{ HID_TYPE_INPUT, 0x01, assemble_input_01 },
{ HID_TYPE_FEATURE, 0x01, assemble_feature_01 },
{ HID_TYPE_FEATURE, 0xef, assemble_feature_ef },
{ HID_TYPE_FEATURE, 0xf2, assemble_feature_f2 },
{ HID_TYPE_FEATURE, 0xf7, assemble_feature_f7 },
{ HID_TYPE_FEATURE, 0xf8, assemble_feature_f8 },
{ 0 } };

static struct sixaxis_process_t sixaxis_process[] =
{
{ HID_TYPE_INPUT, 0x01, process_input_01 },
{ HID_TYPE_OUTPUT, 0x01, process_output_01 },
{ HID_TYPE_FEATURE, 0xef, process_feature_ef },
{ HID_TYPE_FEATURE, 0xf4, process_feature_f4 },
{ 0 } };

static int send_report(int sixaxis_number, uint16_t psm, uint8_t type, uint8_t report, int blocking)
{
  uint8_t buf[128];
  int len = 0;
  int i;
  struct timeval tv;

  struct sixaxis_state* state = states + sixaxis_number;

  /* Assemble report */
  for (i = 0; sixaxis_assemble[i].func; i++)
  {
    if (sixaxis_assemble[i].type == type
        && sixaxis_assemble[i].report == report)
    {
      len = sixaxis_assemble[i].func(&buf[2], sizeof(buf) - 2, state);
      break;
    }
  }

  if (!sixaxis_assemble[i].func || len < 0)
  {
    printf("%s %s report 0x%02x, sending empty response\n",
        (len < 0) ? "Error assembling" : "Unknown", hid_report_name[type],
        report);
    len = 0;
  }

  /* Fill common portion */
  buf[0] = 0xa0 | type;
  buf[1] = report;
  len += 2;

  /* Dump contents */
  if (debug >= 2)
  {
    gettimeofday(&tv, NULL);
    printf("%ld.%06ld Sixaxis %-7s %02x:", tv.tv_sec, tv.tv_usec,
        hid_report_name[type], report);
    for (i = 2; i < len; i++)
      printf(" %02x", buf[i]);
    printf("\n");
  }

  /* Send response.  Some messages (periodic input report) can be
   sent nonblocking, since they're not critical */
  return l2cap_abs_get()->send((psm == PSM_HID_CONTROL) ? state->channels.control.id : state->channels.interrupt.id, buf, len, blocking);
}

static int process_report(uint8_t type, uint8_t report, const uint8_t *buf, int len,
    struct sixaxis_state *state)
{
  int i;
  int ret = 0;
  struct timeval tv;

  /* Dump contents */
  if (debug >= 2)
  {
    gettimeofday(&tv, NULL);
    printf("%ld.%06ld     PS3 %-7s %02x:", tv.tv_sec, tv.tv_usec,
        hid_report_name[type], report);
    for (i = 0; i < len; i++)
      printf(" %02x", buf[i]);
    printf("\n");
  }

  /* Process report */
  for (i = 0; sixaxis_process[i].func; i++)
  {
    if (sixaxis_process[i].type == type && sixaxis_process[i].report == report)
    {
      ret = sixaxis_process[i].func(buf, len, state);
      break;
    }
  }

  if (!sixaxis_process[i].func || ret < 0)
  {
    printf("%s %s report 0x%02x\n", (ret < 0) ? "Error processing" : "Unknown",
        hid_report_name[type], report);
  }

  return ret;
}

static int process(int sixaxis_number, int psm, const unsigned char *buf, int len)
{
  uint8_t transaction;
  uint16_t maxsize;
  uint8_t type;
  uint8_t report;
  const char *name;
  int ret = 0;
  struct timeval tv1, tv2;
  unsigned long time;

  struct sixaxis_state* state = states + sixaxis_number;

  if (len < 1)
    return -1;

  transaction = (buf[0] & 0xf0) >> 4;
  switch (transaction)
  {
    case HID_HANDSHAKE:
      if (buf[0] & 0x0f)
      {
        printf("handshake error: 0x%x\n", buf[0] & 0x0f);
        return -1;
      }
      break;

    case HID_GET_REPORT:
      if (buf[0] & 0x08)
      {
        if (len < 4)
        {
          printf("GET_REPORT short\n");
          return -1;
        }
        maxsize = (buf[3] << 8) | buf[2];
        if (maxsize < 64)
        {
          printf("GET_REPORT short buf (%d)\n", maxsize);
          return -1;
        }
      }
      type = buf[0] & 0x03;
      if (type == HID_TYPE_RESERVED)
      {
        printf("GET_REPORT bad type\n");
        return -1;
      }
      report = buf[1];
      /* printf("<- GET_REPORT %s 0x%02x\n", hid_report_name[type], report); */
      if (debug >= 2)
      {
        gettimeofday(&tv1, NULL);
      }
      ret = send_report(sixaxis_number, psm, type, report, 1);
      if (debug >= 2)
      {
        gettimeofday(&tv2, NULL);
        time = (tv2.tv_sec * 1000 + tv2.tv_usec)
            - (tv1.tv_sec * 1000 + tv1.tv_usec);
        printf("blocking send took: %ld µs\n", time);
      }
      break;

    case HID_SET_REPORT:
    case HID_DATA:
      /* SET_REPORT and DATA are similar */
      name = (transaction == HID_DATA) ? "DATA" : "SET_REPORT";
      if (len < 2)
      {
        printf("%s: short\n", name);
        return -1;
      }
      type = buf[0] & 0x03;
      if (type == HID_TYPE_RESERVED)
      {
        printf("%s bad type\n", name);
        return -1;
      }
      report = buf[1];
      ret = process_report(type, report, buf + 2, len - 2, state);
      /* Respond to these on CTRL port with a positive HANDSHAKE */
      if (psm == PSM_HID_CONTROL)
      {
        unsigned char foo = (HID_HANDSHAKE << 4) | 0x0;
        if (l2cap_abs_get()->send(state->channels.control.id, &foo, 1, 1) < 1)
        {
          fprintf(stderr, "l2cap_send error\n");
        }

        /*if(report == 0xf4)
         {
         bdaddr_t dest_addr;
         str2ba(state->bdaddr_dst, &dest_addr);
         if(l2cap_set_flush_timeout(&dest_addr, FLUSH_TIMEOUT) < 0)
         {
         fprintf(stderr, "can't set flush timeout for %s\n", state->bdaddr_dst);
         }
         }*/
      }
      break;

    default:
      fprintf(stderr, "unknown transaction %d\n", transaction);
      return -1;
  }

  if(psm == PSM_HID_INTERRUPT)
  {
    ret = 1;
  }

  return ret;
}

static int read_control(int sixaxis_number)
{
  int ret = 0;

  struct sixaxis_state* state = states + sixaxis_number;

  ssize_t len = l2cap_abs_get()->recv(state->channels.control.id, buf, sizeof(buf));

  if (len > 0)
  {
    if (process(sixaxis_number, PSM_HID_CONTROL, buf, len) == -1)
    {
      fprintf(stderr, "error processing ctrl\n");
    }
    else if (state->sys.shutdown)
    {
      fprintf(stderr, "sixaxis shutdown\n");
      ret = 1;
      adapter_get(sixaxis_number)->send_command = 1;
    }
  }

  return ret;
}

static int close_control(int sixaxis_number)
{
  struct sixaxis_state* state = states + sixaxis_number;

  if(state->channels.control.id >= 0)
  {
    l2cap_abs_get()->close(state->channels.control.id);
    state->channels.control.id = -1;
    state->channels.control.pending = 0;
  }

  state->sys.shutdown = 1;
  adapter_get(sixaxis_number)->send_command = 1;

  return 1;
}

static int read_interrupt(int sixaxis_number)
{
  int ret = 0;

  struct sixaxis_state* state = states + sixaxis_number;

  ssize_t len = l2cap_abs_get()->recv(state->channels.interrupt.id, buf, sizeof(buf));

  if (len > 0)
  {
    if (process(sixaxis_number, PSM_HID_INTERRUPT, buf, len) == -1)
    {
      fprintf(stderr, "error processing data\n");
    }
    else
    {
      ret = 1;
    }
  }

  return ret;
}

static int close_interrupt(int sixaxis_number)
{
  struct sixaxis_state* state = states + sixaxis_number;

  if(state->channels.interrupt.id >= 0)
  {
    l2cap_abs_get()->close(state->channels.interrupt.id);
    state->channels.interrupt.id = -1;
    state->channels.interrupt.pending = 0;
  }

  state->sys.shutdown = 1;
  adapter_get(sixaxis_number)->send_command = 1;

  return 1;
}

int sixaxis_send_interrupt(int sixaxis_number, s_report_ds3* buf)
{
  struct sixaxis_state* state = states + sixaxis_number;

  if(state->sys.shutdown)
  {
    return -1;
  }

  if(state->channels.interrupt.id < 0 || state->channels.interrupt.pending)
  {
    return 0;
  }

  process_report(HID_TYPE_INPUT, 0x01, (unsigned char*) buf, sizeof(s_report_ds3), state);

  return send_report(0, PSM_HID_INTERRUPT, HID_TYPE_INPUT, 0x01, 0);
}

void sixaxis_close(int sixaxis_number)
{
  struct sixaxis_state* state = states + sixaxis_number;

  l2cap_abs_get()->disconnect(state->channels.control.id);

  close_interrupt(sixaxis_number);
  close_control(sixaxis_number);
}

static int connect_interrupt(int sixaxis_number)
{
  struct sixaxis_state* state = states + sixaxis_number;

  state->channels.interrupt.pending = 0;

  gprintf("connected with hci%d = %s to %s\n", state->dongle_index, state->bdaddr_src.str, state->bdaddr_dst);

  l2cap_abs_get()->add_source(state->channels.interrupt.id, sixaxis_number, read_interrupt, process, close_interrupt);

  return 0;
}

static int connect_control(int sixaxis_number)
{
  struct sixaxis_state* state = states + sixaxis_number;

  state->channels.control.pending = 0;

  gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
    state->bdaddr_src.str, state->bdaddr_dst, PSM_HID_INTERRUPT);

  if ((state->channels.interrupt.id = l2cap_abs_get()->connect(state->bdaddr_src.str, state->bdaddr_dst,
      PSM_HID_INTERRUPT, L2CAP_ABS_LM_MASTER, sixaxis_number, connect_interrupt, close_interrupt)) < 0)
  {
    fprintf(stderr, "can't connect to interrupt psm\n");
    l2cap_abs_get()->close(state->channels.control.id);
    return -1;
  }

  state->channels.interrupt.pending = 1;

  l2cap_abs_get()->add_source(state->channels.control.id, sixaxis_number, read_control, process, close_control);

  return 0;
}

int sixaxis_connect(int sixaxis_number, int dongle_index, const char * bdaddr_dst)
{
  struct sixaxis_state* state = states + sixaxis_number;

  if(gimx_params.btstack && sixaxis_number)
  {
    fprintf(stderr, "multiple instances are not supported when using btstack\n");
    return -1;
  }

  state->dongle_index = dongle_index;
  strncpy(state->bdaddr_dst, bdaddr_dst, sizeof(state->bdaddr_dst));

  sixaxis_init(sixaxis_number);

  if(bt_device_abs_get()->init() < 0)
  {
    fprintf(stderr, "failed to initialize the bluetooth interface\n");
    return -1;
  }

  if (bt_device_abs_get()->get_bdaddr(state->dongle_index, &state->bdaddr_src.ba) < 0)
  {
    fprintf(stderr, "failed to get device address\n");
    return -1;
  }
  ba2str(&state->bdaddr_src.ba, state->bdaddr_src.str);
  state->sixaxis_number = sixaxis_number;

  if (bt_device_abs_get()->write_device_class(state->dongle_index, DS3_DEVICE_CLASS) < 0)
  {
    fprintf(stderr, "failed to get device address\n");
    return -1;
  }

  gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
    state->bdaddr_src.str, state->bdaddr_dst, PSM_HID_CONTROL);

  if ((state->channels.control.id = l2cap_abs_get()->connect(state->bdaddr_src.str, state->bdaddr_dst,
      PSM_HID_CONTROL, L2CAP_ABS_LM_MASTER, sixaxis_number, connect_control, close_control)) < 0)
  {
    fprintf(stderr, "can't connect to control psm\n");
    return -1;
  }

  state->channels.control.pending = 1;

  return 0;
}
