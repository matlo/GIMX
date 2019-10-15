/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>
#include "gimx.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#ifndef WIN32
#include "connectors/btds4.h"
#include "connectors/bluetooth/linux/bt_mgmt.h"
#include "connectors/bluetooth/bt_device_abs.h"
#include <mhash.h>
#include <poll.h>
#include <arpa/inet.h> /* for htons */
#else
#include <winsock2.h> /* for htons */
#endif
#include "connectors/bluetooth/l2cap_abs.h"
#include <gimxinput/include/ginput.h>
#include "connectors/report2event/report2event.h"

#define DS4_DEVICE_CLASS 0x2508

#define SDP_PACKET_SIZE 1024

#define HID_HANDSHAKE 0x0
#define HID_GET_REPORT 0x4
#define HID_SET_REPORT 0x5
#define HID_DATA 0xA

#define HID_TYPE_RESERVED 0
#define HID_TYPE_INPUT 1
#define HID_TYPE_OUTPUT 2
#define HID_TYPE_FEATURE 3

static unsigned char sdp_ps4[] =
{
  0x07, 0x00, 0x01, 0x01, 0x53, 0x01, 0x50, 0x36, 0x01, 0x4d, 0x36, 0x00, 0x32, 0x09, 0x00, 0x00,
  0x0a, 0x00, 0x01, 0x00, 0x05, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x11, 0x0a, 0x09, 0x00, 0x04,
  0x35, 0x10, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x19, 0x35, 0x06, 0x19, 0x00, 0x19, 0x09,
  0x01, 0x02, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x0d, 0x09, 0x01, 0x02, 0x36,
  0x00, 0x32, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x06, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19,
  0x11, 0x0b, 0x09, 0x00, 0x04, 0x35, 0x10, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x19, 0x35,
  0x06, 0x19, 0x00, 0x19, 0x09, 0x01, 0x02, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11,
  0x0d, 0x09, 0x01, 0x02, 0x36, 0x00, 0x3b, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x07, 0x09,
  0x00, 0x01, 0x35, 0x06, 0x19, 0x11, 0x0e, 0x19, 0x11, 0x0f, 0x09, 0x00, 0x04, 0x35, 0x10, 0x35,
  0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x17, 0x35, 0x06, 0x19, 0x00, 0x17, 0x09, 0x01, 0x03, 0x09,
  0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x0e, 0x09, 0x01, 0x04, 0x09, 0x03, 0x11, 0x09,
  0x00, 0x02, 0x36, 0x00, 0x4d, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x08, 0x09, 0x00, 0x01,
  0x35, 0x03, 0x19, 0x11, 0x0c, 0x09, 0x00, 0x04, 0x35, 0x10, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09,
  0x00, 0x17, 0x35, 0x06, 0x19, 0x00, 0x17, 0x09, 0x01, 0x03, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35,
  0x06, 0x19, 0x11, 0x0e, 0x09, 0x01, 0x04, 0x09, 0x00, 0x0d, 0x35, 0x10, 0x35, 0x06, 0x19, 0x01,
  0x00, 0x09, 0x00, 0x1b, 0x35, 0x06, 0x19, 0x00, 0x17, 0x09, 0x01, 0x03, 0x09, 0x03, 0x11, 0x09,
  0x00, 0x01, 0x36, 0x00, 0x52, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x0a, 0x09, 0x00, 0x01,
  0x35, 0x03, 0x19, 0x12, 0x00, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09,
  0x00, 0x01, 0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x12,
  0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x01, 0x09, 0x05, 0x4c,
  0x09, 0x02, 0x02, 0x09, 0x08, 0x1f, 0x09, 0x02, 0x03, 0x09, 0x01, 0x00, 0x09, 0x02, 0x04, 0x28,
  0x01, 0x09, 0x02, 0x05, 0x09, 0x00, 0x02, 0x00
};

static unsigned char sdp_ds4[] =
{
  0x07, 0x00, 0x01, 0x02, 0xbf, 0x02, 0xbc, 0x36, 0x02, 0xb9, 0x36, 0x02, 0x61, 0x09, 0x00, 0x00,
  0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x11, 0x24, 0x09, 0x00, 0x04,
  0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09,
  0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, 0x09, 0x00, 0x09,
  0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x24, 0x09, 0x01, 0x00, 0x09, 0x00, 0x0d, 0x35, 0x0f, 0x35,
  0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x13, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01,
  0x00, 0x25, 0x13, 0x57, 0x69, 0x72, 0x65, 0x6c, 0x65, 0x73, 0x73, 0x20, 0x43, 0x6f, 0x6e, 0x74,
  0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x09, 0x01, 0x01, 0x25, 0x0f, 0x47, 0x61, 0x6d, 0x65, 0x20,
  0x43, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x09, 0x01, 0x02, 0x25, 0x1b, 0x53,
  0x6f, 0x6e, 0x79, 0x20, 0x43, 0x6f, 0x6d, 0x70, 0x75, 0x74, 0x65, 0x72, 0x20, 0x45, 0x6e, 0x74,
  0x65, 0x72, 0x74, 0x61, 0x69, 0x6e, 0x6d, 0x65, 0x6e, 0x74, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00,
  0x09, 0x02, 0x01, 0x09, 0x01, 0x11, 0x09, 0x02, 0x02, 0x08, 0x08, 0x09, 0x02, 0x03, 0x08, 0x00,
  0x09, 0x02, 0x04, 0x28, 0x00, 0x09, 0x02, 0x05, 0x28, 0x01, 0x09, 0x02, 0x06, 0x36, 0x01, 0x6c,
  0x36, 0x01, 0x69, 0x08, 0x22, 0x26, 0x01, 0x64, 0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x85, 0x01,
  0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95,
  0x04, 0x81, 0x02, 0x09, 0x39, 0x15, 0x00, 0x25, 0x07, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x05,
  0x09, 0x19, 0x01, 0x29, 0x0e, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x0e, 0x81, 0x02, 0x75,
  0x06, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01, 0x09, 0x33, 0x09, 0x34, 0x15, 0x00, 0x26, 0xff, 0x00,
  0x75, 0x08, 0x95, 0x02, 0x81, 0x02, 0x06, 0x04, 0xff, 0x85, 0x02, 0x09, 0x24, 0x95, 0x24, 0xb1,
  0x02, 0x85, 0xa3, 0x09, 0x25, 0x95, 0x30, 0xb1, 0x02, 0x85, 0x05, 0x09, 0x26, 0x95, 0x28, 0xb1,
  0x02, 0x85, 0x06, 0x09, 0x27, 0x95, 0x34, 0xb1, 0x02, 0x85, 0x07, 0x09, 0x28, 0x95, 0x30, 0xb1,
  0x02, 0x85, 0x08, 0x09, 0x29, 0x95, 0x2f, 0xb1, 0x02, 0x06, 0x03, 0xff, 0x85, 0x03, 0x09, 0x21,
  0x95, 0x26, 0xb1, 0x02, 0x85, 0x04, 0x09, 0x22, 0x95, 0x2e, 0xb1, 0x02, 0x85, 0xf0, 0x09, 0x47,
  0x95, 0x3f, 0xb1, 0x02, 0x85, 0xf1, 0x09, 0x48, 0x95, 0x3f, 0xb1, 0x02, 0x85, 0xf2, 0x09, 0x49,
  0x95, 0x0f, 0xb1, 0x02, 0x06, 0x00, 0xff, 0x85, 0x11, 0x09, 0x20, 0x15, 0x00, 0x26, 0xff, 0x00,
  0x75, 0x08, 0x95, 0x4d, 0x81, 0x02, 0x09, 0x21, 0x91, 0x02, 0x85, 0x12, 0x09, 0x22, 0x95, 0x8d,
  0x81, 0x02, 0x09, 0x23, 0x91, 0x02, 0x85, 0x13, 0x09, 0x24, 0x95, 0xcd, 0x81, 0x02, 0x09, 0x25,
  0x91, 0x02, 0x85, 0x14, 0x09, 0x26, 0x96, 0x0d, 0x01, 0x81, 0x02, 0x09, 0x27, 0x91, 0x02, 0x85,
  0x15, 0x09, 0x28, 0x96, 0x4d, 0x01, 0x81, 0x02, 0x09, 0x29, 0x91, 0x02, 0x85, 0x16, 0x09, 0x2a,
  0x96, 0x8d, 0x01, 0x81, 0x02, 0x09, 0x2b, 0x91, 0x02, 0x85, 0x17, 0x09, 0x2c, 0x96, 0xcd, 0x01,
  0x81, 0x02, 0x09, 0x2d, 0x91, 0x02, 0x85, 0x18, 0x09, 0x2e, 0x96, 0x0d, 0x02, 0x81, 0x02, 0x09,
  0x2f, 0x91, 0x02, 0x85, 0x19, 0x09, 0x30, 0x96, 0x22, 0x02, 0x81, 0x02, 0x09, 0x31, 0x91, 0x02,
  0x06, 0x80, 0xff, 0x85, 0x82, 0x09, 0x22, 0x95, 0x3f, 0xb1, 0x02, 0x85, 0x83, 0x09, 0x23, 0xb1,
  0x02, 0x85, 0x84, 0x09, 0x24, 0xb1, 0x02, 0x85, 0x90, 0x09, 0x30, 0xb1, 0x02, 0x85, 0x91, 0x09,
  0x31, 0xb1, 0x02, 0x85, 0x92, 0x09, 0x32, 0xb1, 0x02, 0x85, 0x93, 0x09, 0x33, 0xb1, 0x02, 0x85,
  0xa0, 0x09, 0x40, 0xb1, 0x02, 0x85, 0xa4, 0x09, 0x44, 0xb1, 0x02, 0xc0, 0x09, 0x02, 0x07, 0x35,
  0x08, 0x35, 0x06, 0x09, 0x04, 0x09, 0x09, 0x01, 0x00, 0x09, 0x02, 0x08, 0x28, 0x00, 0x09, 0x02,
  0x09, 0x28, 0x01, 0x09, 0x02, 0x0a, 0x28, 0x01, 0x09, 0x02, 0x0b, 0x09, 0x01, 0x00, 0x09, 0x02,
  0x0c, 0x09, 0x1f, 0x40, 0x09, 0x02, 0x0d, 0x28, 0x00, 0x09, 0x02, 0x0e, 0x28, 0x00, 0x36, 0x00,
  0x52, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x02, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x12,
  0x00, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03,
  0x19, 0x00, 0x01, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x12, 0x00, 0x09, 0x01, 0x03,
  0x09, 0x02, 0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x01, 0x09, 0x05, 0x4c, 0x09, 0x02, 0x02, 0x09,
  0x05, 0xc4, 0x09, 0x02, 0x03, 0x09, 0x01, 0x00, 0x09, 0x02, 0x04, 0x28, 0x01, 0x09, 0x02, 0x05,
  0x09, 0x00, 0x02, 0x00
};

struct btds4_state_sys {
    int shutdown;
    uint8_t rumble[2];
};

typedef struct GIMX_PACKED {
  unsigned char header;
  unsigned char code;
  unsigned char unknown1;
  s_report_ds4 report;
  unsigned char crc32[4];
} s_btds4_report;

typedef struct
{
  int id;
  int pending;//only for ps4 control and interrupt
} s_channel;

typedef struct
{
  s_channel sdp;
  s_channel control;
  s_channel interrupt;
} s_channels;

struct btds4_state {
    struct {
      char str[18];
      bdaddr_t ba;
    } dongle_bdaddr;
    char ps4_bdaddr[18];
    char ds4_bdaddr[18];
    int dongle_index;
    int btds4_number;
    struct btds4_state_sys sys;
    s_btds4_report bt_report;
    s_report_ds4 previous;
    int joystick_id;
    unsigned short inactivity_counter;
    unsigned char active;
    s_channels ps4_channels;
    s_channels ds4_channels;
};

static struct
{
  int sdp;
  int hid_control;
  int hid_interrupt;
} listening_channels = { -1, -1, -1 };

struct btds4_assemble_t {
    int type;
    uint8_t report;
    int (*func)(uint8_t *buf, int maxlen, struct btds4_state *state);
};

struct btds4_process_t {
    int type;
    uint8_t report;
    int (*func)(const uint8_t *buf, int len, struct btds4_state *state);
};

static struct btds4_state states[MAX_CONTROLLERS] = {};

static int read_ds4_sdp(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[SDP_PACKET_SIZE];

  ssize_t len = l2cap_abs_get()->recv(states[btds4_number].ds4_channels.sdp.id, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ds4 sdp\n");
  }
  else
  {
    int ret = l2cap_abs_get()->send(states[btds4_number].ds4_channels.sdp.id, sdp_ps4, sizeof(sdp_ps4), 0);

    if(ret != sizeof(sdp_ps4))
    {
      fprintf(stderr, "error writing ds4 sdp: %d\n", ret);
    }
  }

  return 0;
}

static int close_ds4_sdp(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ds4_channels.sdp.id >= 0)
  {
    l2cap_abs_get()->close(state->ds4_channels.sdp.id);
    state->ds4_channels.sdp.id = -1;
  }

  return 1;
}

static int read_ps4_sdp(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[SDP_PACKET_SIZE];

  ssize_t len = l2cap_abs_get()->recv(states[btds4_number].ps4_channels.sdp.id, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ps4 sdp\n");
  }
  else
  {
    //assume the ps4 requests the sdp descriptor

    int ret = l2cap_abs_get()->send(states[btds4_number].ps4_channels.sdp.id, sdp_ds4, sizeof(sdp_ds4), 0);

    if(ret < 0)
    {
      fprintf(stderr, "error writing ps4 sdp\n");
    }
  }

  return 0;
}

static int connect_ps4_control(void * user);
static int connect_ps4_interrupt(void * user);
static int close_ps4_control(void * user);
static int close_ps4_interrupt(void * user);

static int close_ps4_sdp(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ps4_channels.sdp.id >= 0)
  {
    l2cap_abs_get()->close(state->ps4_channels.sdp.id);
    state->ps4_channels.sdp.id = -1;
  }

  if(state->sys.shutdown)
  {
    return 1;
  }

  /*
   * Warning: this is really hackish...
   * There is an issue at the very first connection.
   * Maybe the acl_send_data hack interferes in some way with the l2cap sockets.
   * As a work-around, we disconnect and restart the connection.
   */

  l2cap_abs_get()->disconnect(state->ps4_channels.control.id);
  close_ps4_control(user);
  close_ps4_interrupt(user);

  ginfo("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
      state->dongle_bdaddr.str, state->ps4_bdaddr, PSM_HID_CONTROL);

  if ((state->ps4_channels.control.id = l2cap_abs_get()->connect(state->dongle_bdaddr.str, state->ps4_bdaddr,
      PSM_HID_CONTROL, L2CAP_ABS_LM_MASTER | L2CAP_ABS_LM_AUTH | L2CAP_ABS_LM_ENCRYPT, user, connect_ps4_control, close_ps4_control)) < 0)
  {
    fprintf(stderr, "can't connect to control psm\n");
    return -1;
  }

  state->ps4_channels.control.pending = 1;

  ginfo("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
      state->dongle_bdaddr.str, state->ps4_bdaddr, PSM_HID_INTERRUPT);

  if ((state->ps4_channels.interrupt.id = l2cap_abs_get()->connect(state->dongle_bdaddr.str, state->ps4_bdaddr,
      PSM_HID_INTERRUPT, L2CAP_ABS_LM_MASTER | L2CAP_ABS_LM_AUTH | L2CAP_ABS_LM_ENCRYPT, user, connect_ps4_interrupt, close_ps4_control)) < 0)
  {
    close_ps4_control(user);
    fprintf(stderr, "can't connect to interrupt psm\n");
    return -1;
  }

  state->ps4_channels.interrupt.pending = 1;

  return 1;
}

static int read_ds4_control(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[1024];

  ssize_t len = l2cap_abs_get()->recv(states[btds4_number].ds4_channels.control.id, buf, sizeof(buf));

  /*if(buf[1] == 0x04)
  {
    ginfo("alter report id 0x04\n");
    buf[4] = ~buf[4];
  }*/

  if(len < 0)
  {
    fprintf(stderr, "error reading ds4 control\n");
  }
  else
  {
    int ret = l2cap_abs_get()->send(states[btds4_number].ps4_channels.control.id, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing ps4 control\n");
    }
  }

  return 0;
}

static int close_ds4_control(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ds4_channels.control.id >= 0)
  {
    l2cap_abs_get()->close(state->ds4_channels.control.id);
    state->ds4_channels.control.id = -1;
  }

  return 1;
}

//static int cpt = 0;

static int read_ds4_interrupt(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[1024];

  int ret = l2cap_abs_get()->recv(states[btds4_number].ds4_channels.interrupt.id, buf, sizeof(buf));

  if(ret < 0)
  {
    fprintf(stderr, "error reading ds4 interrupt\n");
  }

  unsigned char* pbuf = buf;
  s_report_ds4* current = &((s_btds4_report*)pbuf)->report;
  s_report_ds4* previous = &states[btds4_number].previous;

  report2event(C_TYPE_DS4, btds4_number, (s_report*)current, (s_report*)previous, states[btds4_number].joystick_id);

  *previous = *current;

  return 0;

  /*unsigned char buf[1024];

  ssize_t len = l2cap_abs_get(E_L2CAP_BLUEZ)->recv(states[btds4_number].ds4_channels.interrupt, buf, sizeof(buf));

  cpt++;
  if(cpt%8)
  {
    return 0;
  }

  if(len < 0)
  {
    fprintf(stderr, "error reading ds4 interrupt\n");
  }
  else
  {

    int ret = l2cap_abs_get(E_L2CAP_BLUEZ)->send(states[btds4_number].ps4_channels.interrupt, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing %d bytes on ps4 interrupt\n", len);
    }
  }

  return 0;*/
}

static int close_ds4_interrupt(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ds4_channels.interrupt.id >= 0)
  {
    l2cap_abs_get()->close(state->ds4_channels.interrupt.id);
    state->ds4_channels.interrupt.id = -1;
  }

  return 1;
}

static int read_ps4_control(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[1024];

  ssize_t len = l2cap_abs_get()->recv(states[btds4_number].ps4_channels.control.id, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ps4 control\n");
  }
  else
  {
    /*if(buf[1] == 0x02)
    {
      unsigned char resp02[] =
      {
          0xa3, 0x02, 0x01, 0x00, 0xff, 0xff, 0x01, 0x00, 0x5e, 0x22, 0x84, 0x22, 0x9b, 0x22, 0xa6, 0xdd,
          0x79, 0xdd, 0x64, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0x85, 0x1f, 0x9f, 0xe0, 0x92, 0x20, 0xdc, 0xe0,
          0x4d, 0x1c, 0x1e, 0xde, 0x08, 0x00
      };

      if(l2cap_abs_get(E_L2CAP_BLUEZ)->send(states[btds4_number].ps4_channels.control, resp02, sizeof(resp02), 0) != sizeof(resp02))
      {
        fprintf(stderr, "error writing ps4 control\n");
      }

      return 0;
    }

    if(buf[1] == 0x06)
    {
      unsigned char resp06[] =
      {
        0xa3, 0x06, 0x41, 0x75, 0x67, 0x20, 0x20, 0x33, 0x20, 0x32, 0x30, 0x31, 0x33, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x30, 0x37, 0x3a, 0x30, 0x31, 0x3a, 0x31, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x31, 0x03, 0x00, 0x00, 0x00, 0x49, 0x00, 0x05, 0x00, 0x00, 0x80,
        0x03, 0x00, 0x4b, 0x52, 0x02, 0xc7
      };

      if(l2cap_abs_get(E_L2CAP_BLUEZ)->send(states[btds4_number].ps4_channels.control, resp06, sizeof(resp06), 0) != sizeof(resp06))
      {
        fprintf(stderr, "error writing ps4 control\n");
      }

      return 0;
    }

    if(buf[1] == 0xa3)
    {
      unsigned char respa3[] =
      {
        0xa3, 0x06, 0x41, 0x75, 0x67, 0x20, 0x20, 0x33, 0x20, 0x32, 0x30, 0x31, 0x33, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x30, 0x37, 0x3a, 0x30, 0x31, 0x3a, 0x31, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x31, 0x03, 0x00, 0x00, 0x00, 0x49, 0x00, 0x05, 0x00, 0x00, 0x80,
        0x03, 0x00
      };

      if(l2cap_abs_get(E_L2CAP_BLUEZ)->send(states[btds4_number].ps4_channels.control, respa3, sizeof(respa3), 0) != sizeof(respa3))
      {
        fprintf(stderr, "error writing ps4 control\n");
      }

      return 0;
    }*/

    /*
     * todo: forward to USB device, and deactivate l2cap_send
     */

    /*if(buf[1] == 0x03 || buf[1] == 0x04)
    {
      gtime now = gtime_gettime();
      ginfo("%lu.%06lu ", GTIME_SECPART(now), GTIME_USECPART(now));
      ginfo("report id 0x%02x\n", buf[1]);
    }*/

    int ret = l2cap_abs_get()->send(states[btds4_number].ds4_channels.control.id, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing ds4 control\n");
    }
  }

  return 0;
}

static int close_ps4_control(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ps4_channels.control.id >= 0)
  {
    l2cap_abs_get()->close(state->ps4_channels.control.id);
    state->ps4_channels.control.id = -1;
    state->ps4_channels.control.pending = 0;
  }

  state->sys.shutdown = 1;

  return 1;
}

static int process(void * user __attribute__((unused)), int psm __attribute__((unused)), const unsigned char *buf __attribute__((unused)), int len __attribute__((unused)))
{
  //TODO MLA: this function could probably be removed
  return 0;
}

static int connect_ps4_control(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  state->ps4_channels.control.pending = 0;

  if(state->ds4_channels.control.id >= 0)
  {
    l2cap_abs_get()->add_source(state->ps4_channels.control.id, user, read_ps4_control, process, close_ps4_control);
    l2cap_abs_get()->add_source(state->ds4_channels.control.id, user, read_ds4_control, process, close_ds4_control);
  }

  return 0;
}

static int read_ps4_interrupt(void * user)
{
  int btds4_number = (intptr_t) user;

  unsigned char buf[1024];

  ssize_t len = l2cap_abs_get()->recv(states[(intptr_t) user].ps4_channels.interrupt.id, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ps4 interrupt\n");
  }
  else
  {
    switch(buf[1])
    {
      case 0x11:
      case 0x15:
      case 0x19:
        {
          int joystick = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, btds4_number);

          if(joystick >= 0 && (ginput_joystick_get_haptic(joystick) & GE_HAPTIC_RUMBLE))
          {
            GE_Event event =
            {
              .jrumble =
              {
                .type = GE_JOYRUMBLE,
                .which = joystick,
                .weak = buf[7] << 8,
                .strong = buf[8] << 8
              }
            };
            ginput_queue_push(&event);
          }
        }
        break;
      default:
        break;
    }
  }

  return 0;
}

static int ds4_interrupt_rumble(const GE_Event * haptic)
{
  static struct __attribute__ ((packed))
  {
    unsigned char data[75];
    unsigned char crc32[4];
  } report =
  {
    .data =
    {
      0xa2, 0x11, 0xc0, 0x20, 0xf3, 0x04, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x43,
      0x00, 0x4d, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00
    }
  };

  int ret = 0;

  unsigned int i;
  for(i = 0; i < sizeof(states)/sizeof(*states); ++i)
  {
    if(states[i].joystick_id == haptic->which)
    {
      report.data[7] = haptic->jrumble.weak >> 8;
      report.data[8] = haptic->jrumble.strong >> 8;

#ifndef WIN32
      MHASH td = mhash_init(MHASH_CRC32B);

      if (td == MHASH_FAILED)
      {
        perror("mhash_init");
      }

      mhash(td, report.data, sizeof(report.data));
#else
      //TODO MLA: windows port
#endif

      unsigned int digest = 0; // crc32 will be stored here

#ifndef WIN32
      mhash_deinit(td, &digest);
#endif

      report.crc32[3] = digest >> 24;
      report.crc32[2] = (digest >> 16) & 0xFF;
      report.crc32[1] = (digest >> 8) & 0xFF;
      report.crc32[0] = digest & 0xFF;

      int len = sizeof(report);
      ret = l2cap_abs_get()->send(states[i].ds4_channels.interrupt.id, (unsigned char*)&report, len, 0);
      if(ret != len)
      {
        fprintf(stderr, "error writing ds4 interrupt\n");
      }

      break;
    }
  }

  return ret;
}

static int close_ps4_interrupt(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  if(state->ps4_channels.interrupt.id >= 0)
  {
    l2cap_abs_get()->close(state->ps4_channels.interrupt.id);
    state->ps4_channels.interrupt.id = -1;
    state->ps4_channels.interrupt.pending = 0;
  }

  state->sys.shutdown = 1;

  return 1;
}

static int connect_ps4_interrupt(void * user)
{
  int btds4_number = (intptr_t) user;

  struct btds4_state* state = states + btds4_number;

  state->ps4_channels.interrupt.pending = 0;

  if(state->ds4_channels.interrupt.id >= 0)
  {
    l2cap_abs_get()->add_source(state->ps4_channels.interrupt.id, user, read_ps4_interrupt, process, close_ps4_interrupt);
    l2cap_abs_get()->add_source(state->ds4_channels.interrupt.id, user, read_ds4_interrupt, process, close_ds4_interrupt);
  }

  return 0;
}

static int listen_accept_sdp(int channel, bdaddr_t * src)
{
  bdaddr_t cmp;

  unsigned int i;
  for(i=0; i<sizeof(states)/sizeof(*states); ++i)
  {
    /*
     * Look for a PS4 address.
     */
    str2ba(states[i].ps4_bdaddr, &cmp);

    if(!bacmp(src, &cmp))
    {
      states[i].ps4_channels.sdp.id = channel;
      l2cap_abs_get()->add_source(channel, (void *)(intptr_t) i, read_ps4_sdp, process, close_ps4_sdp);
      break;
    }
  }

  if(i == sizeof(states)/sizeof(*states))
  {
    for(i=0; i<sizeof(states)/sizeof(*states); ++i)
    {
      /*
       * Look for a controller that has no ds4 bdaddr.
       */
      if(strchr(states[i].ps4_bdaddr, ':') && !strchr(states[i].ds4_bdaddr, ':'))
      {
        ba2str(src, states[i].ds4_bdaddr);
        states[i].ds4_channels.sdp.id = channel;
        l2cap_abs_get()->add_source(channel, (void *)(intptr_t) i, read_ds4_sdp, process, close_ds4_sdp);

        ginfo("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
            states[i].dongle_bdaddr.str, states[i].ps4_bdaddr, PSM_HID_CONTROL);

        if ((states[i].ps4_channels.control.id = l2cap_abs_get()->connect(states[i].dongle_bdaddr.str, states[i].ps4_bdaddr,
            PSM_HID_CONTROL, L2CAP_ABS_LM_MASTER | L2CAP_ABS_LM_AUTH | L2CAP_ABS_LM_ENCRYPT, (void *)(intptr_t) i, connect_ps4_control, close_ps4_control)) < 0)
        {
          fprintf(stderr, "can't connect to control psm\n");
          break;
        }

        states[i].ps4_channels.control.pending = 1;

        ginfo("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
            states[i].dongle_bdaddr.str, states[i].ps4_bdaddr, PSM_HID_INTERRUPT);

        if ((states[i].ps4_channels.interrupt.id = l2cap_abs_get()->connect(states[i].dongle_bdaddr.str, states[i].ps4_bdaddr,
            PSM_HID_INTERRUPT, L2CAP_ABS_LM_MASTER | L2CAP_ABS_LM_AUTH | L2CAP_ABS_LM_ENCRYPT, (void *)(intptr_t) i, connect_ps4_interrupt, close_ps4_interrupt)) < 0)
        {
          close_ps4_control((void *)(intptr_t) i);
          fprintf(stderr, "can't connect to interrupt psm\n");
          break;
        }

        states[i].ps4_channels.interrupt.pending = 1;

        break;
      }
    }

    if(i == sizeof(states)/sizeof(*states))
    {
      fprintf(stderr, "no ps4/ds4 bdaddr found\n");
      return 1;
    }
  }

  return 0;
}

static int listen_accept_control(int channel, bdaddr_t * src)
{
  bdaddr_t cmp;
  unsigned int i;
  for(i=0; i<sizeof(states)/sizeof(*states); ++i)
  {
    /*
     * Look for a DS4 address.
     */
    str2ba(states[i].ds4_bdaddr, &cmp);

    if(!bacmp(src, &cmp))
    {
      states[i].ds4_channels.control.id = channel;
      if(states[i].ps4_channels.control.id >= 0 && !states[i].ps4_channels.control.pending)
      {
        l2cap_abs_get()->add_source(channel, (void *)(intptr_t) i, read_ds4_control, process, close_ds4_control);
        l2cap_abs_get()->add_source(states[i].ps4_channels.control.id, (void *)(intptr_t) i, read_ps4_control, process, close_ps4_control);
      }
      break;
    }
  }

  if(i == sizeof(states)/sizeof(*states))
  {
    fprintf(stderr, "no ds4 bdaddr found\n");
    return 1;
  }

  return 0;
}

static int listen_accept_interrupt(int channel, bdaddr_t * src)
{
  bdaddr_t cmp;
  unsigned int i;
  for(i=0; i<sizeof(states)/sizeof(*states); ++i)
  {
    /*
     * Look for a DS4 address.
     */
    str2ba(states[i].ds4_bdaddr, &cmp);

    if(!bacmp(src, &cmp))
    {
      states[i].ds4_channels.interrupt.id = channel;
      if(states[i].ps4_channels.interrupt.id >= 0 && !states[i].ps4_channels.interrupt.pending)
      {
        l2cap_abs_get()->add_source(channel, (void *)(intptr_t) i, read_ds4_interrupt, process, close_ds4_interrupt);
        l2cap_abs_get()->add_source(states[i].ps4_channels.interrupt.id, (void *)(intptr_t) i, read_ps4_interrupt, process, close_ps4_interrupt);
      }
      break;
    }
  }

  if(i == sizeof(states)/sizeof(*states))
  {
    fprintf(stderr, "no ds4 bdaddr found\n");
    return 1;
  }

  return 0;
}

static int listen_close(void * user)
{
  int channel = (intptr_t) user;

  if(channel == listening_channels.sdp)
  {
    ginfo("close sdp source\n");
    l2cap_abs_get()->close(listening_channels.sdp);
    listening_channels.sdp = -1;
  }
  else if(channel == listening_channels.hid_control)
  {
    ginfo("close hid control source\n");
    l2cap_abs_get()->close(listening_channels.hid_control);
    listening_channels.hid_control = -1;
  }
  else if(channel == listening_channels.hid_interrupt)
  {
    ginfo("close hid interrupt source\n");
    l2cap_abs_get()->close(listening_channels.hid_interrupt);
    listening_channels.hid_interrupt = -1;
  }

  return 1;
}

static s_btds4_report init_report_btds4 = {
    .header = 0xa1,
    .code = 0x11,
    .unknown1 = 0xc0,
    .crc32 =
    {
        0x00, 0x00, 0x00, 0x00
    }
};

int btds4_init(int btds4_number, int dongle_index, const char * bdaddr_dst)
{
  struct btds4_state* state = states+btds4_number;

  state->ps4_channels.control.id = -1;
  state->ps4_channels.interrupt.id = -1;
  state->ps4_channels.sdp.id = -1;
  state->ds4_channels.control.id = -1;
  state->ds4_channels.interrupt.id = -1;
  state->ds4_channels.sdp.id = -1;

  if(gimx_params.btstack && btds4_number)
  {
    fprintf(stderr, "multiple instances are not supported when using btstack\n");
    return -1;
  }

  state->dongle_index = dongle_index;
  strncpy(state->ps4_bdaddr, bdaddr_dst, sizeof(state->ps4_bdaddr) - 1);

  memcpy(&state->bt_report, &init_report_btds4, sizeof(s_btds4_report));
  state->joystick_id = ginput_register_joystick(DS4_DEVICE_NAME, GE_HAPTIC_RUMBLE, ds4_interrupt_rumble);

  return 0;
}

int btds4_listen(int btds4_number)
{
  struct btds4_state * state = states + btds4_number;

#ifndef WIN32
  if(bt_mgmt_adapter_init(state->dongle_index) < 0)
  {
    fprintf(stderr, "failed to initialize bluetooth device\n");
    return -1;
  }
#else
  //TODO MLA: windows port
#endif

  if (bt_device_abs_get()->get_bdaddr(state->dongle_index, &state->dongle_bdaddr.ba) < 0)
  {
    fprintf(stderr, "failed to get device bdaddr\n");
    return -1;
  }
  ba2str(&state->dongle_bdaddr.ba, state->dongle_bdaddr.str);
  state->btds4_number = btds4_number;

  if (bt_device_abs_get()->write_device_class(state->dongle_index, DS4_DEVICE_CLASS) < 0)
  {
    fprintf(stderr, "failed to set device class\n");
    return -1;
  }
  if(listening_channels.sdp < 0)
  {
    if((listening_channels.sdp = l2cap_abs_get()->listen((void *)(intptr_t) btds4_number, state->dongle_bdaddr.str, PSM_SDP, L2CAP_ABS_LM_MASTER, listen_accept_sdp, listen_close)) < 0)
    {
      return -1;
    }
  }

  if(listening_channels.hid_control < 0)
  {
    if((listening_channels.hid_control = l2cap_abs_get()->listen((void *)(intptr_t) btds4_number, state->dongle_bdaddr.str, PSM_HID_CONTROL, L2CAP_ABS_LM_MASTER, listen_accept_control, listen_close)) < 0)
    {
      return -1;
    }
  }

  if(listening_channels.hid_interrupt < 0)
  {
    if((listening_channels.hid_interrupt = l2cap_abs_get()->listen((void *)(intptr_t) btds4_number, state->dongle_bdaddr.str, PSM_HID_INTERRUPT, L2CAP_ABS_LM_MASTER, listen_accept_interrupt, listen_close)) < 0)
    {
      return -1;
    }
  }

  return 0;
}

#define INACTIVITY_THRESHOLD 6000 //6000x10ms=60s

int btds4_send_interrupt(int btds4_number, s_report_ds4* report, int active)
{
  struct btds4_state* state = states + btds4_number;

  if(state->sys.shutdown)
  {
    return -1;
  }

  if(state->ps4_channels.interrupt.id < 0 || state->ps4_channels.interrupt.pending || state->ds4_channels.interrupt.id < 0)
  {
    return 0;
  }

  /*
   * Don't send reports after 60s of inactivity.
   */
  if(active)
  {
    state->active = 1;
    state->inactivity_counter = 0;
  }
  else
  {
    if(!state->active && state->inactivity_counter == INACTIVITY_THRESHOLD)
    {
      return 0;
    }
    state->active = 0;
    ++state->inactivity_counter;
  }

  state->bt_report.report = *report;

#ifndef WIN32
  MHASH td = mhash_init(MHASH_CRC32B);

  if (td == MHASH_FAILED)
  {
    perror("mhash_init");
  }

  mhash(td, &state->bt_report, sizeof(state->bt_report)-4);
#else
  //TODO MLA: windows port
#endif

  unsigned int digest = 0; // crc32 will be stored here

#ifndef WIN32
  mhash_deinit(td, &digest);
#endif

  state->bt_report.crc32[3] = digest >> 24;
  state->bt_report.crc32[2] = (digest >> 16) & 0xFF;
  state->bt_report.crc32[1] = (digest >> 8) & 0xFF;
  state->bt_report.crc32[0] = digest & 0xFF;

  int ret = l2cap_abs_get()->send(state->ps4_channels.interrupt.id, (unsigned char*) &state->bt_report, sizeof(state->bt_report), 0);

  if(ret < 0)
  {
    if(errno == EAGAIN)
    {
      ret = 0;
    }
  }

  return ret;
}

void btds4_close(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  state->sys.shutdown = 1;

  if(state->ps4_channels.control.id >= 0)
  {
    l2cap_abs_get()->disconnect(state->ps4_channels.control.id);
  }
  if(state->ds4_channels.control.id >= 0)
  {
    l2cap_abs_get()->disconnect(state->ds4_channels.control.id);
  }

  /*
   * todo: close any USB device that was opened!
   */

  close_ps4_sdp((void *)(intptr_t) btds4_number);
  close_ps4_interrupt((void *)(intptr_t) btds4_number);
  close_ps4_control((void *)(intptr_t) btds4_number);

  /*
   * todo: deactivate this if we are working with a USB device
   */

  close_ds4_sdp((void *)(intptr_t) btds4_number);
  close_ds4_interrupt((void *)(intptr_t) btds4_number);
  close_ds4_control((void *)(intptr_t) btds4_number);
}
