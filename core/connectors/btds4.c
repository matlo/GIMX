/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "connectors/btds4.h"
#include "connectors/bt_mgmt.h"
#include <adapter.h>
#include "gimx.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#ifndef WIN32
#include <poll.h>
#include <arpa/inet.h> /* for htons */
#else
#include <winsock2.h> /* for htons */
#endif
#include <connectors/bt_utils.h>
#include <connectors/l2cap_con.h>
#include <GE.h>

#include <mhash.h>

#define DS4_DEVICE_CLASS 0x2508

#define PSM_SDP           0x0001
#define PSM_HID_CONTROL   0x0011
#define PSM_HID_INTERRUPT 0x0013

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

typedef struct __attribute__ ((packed)) {
  unsigned char header;
  unsigned char code;
  unsigned char unknown1;
  s_report_ds4 report;
  unsigned char crc32[4];
} s_btds4_report;

struct btds4_state {
    char dongle_bdaddr[18];
    char ps4_bdaddr[18];
    char ds4_bdaddr[18];
    int dongle_index;
    int btds4_number;
    struct btds4_state_sys sys;
    s_btds4_report bt_report;
    unsigned short inactivity_counter;
    unsigned char active;
    int ps4_control_pending;
    int ps4_interrupt_pending;
    int ps4_control;
    int ps4_interrupt;
    int ps4_sdp;
    int ps4_sdp_cid;
    int ds4_control;
    int ds4_interrupt;
    int ds4_sdp_pending;
    int ds4_sdp;
};

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

static int sdp_fd = -1;
static int hid_control_fd = -1;
static int hid_interrupt_fd = -1;

static int read_ds4_sdp(int btds4_number)
{
  unsigned char buf[SDP_PACKET_SIZE];

  ssize_t len = l2cap_recv(states[btds4_number].ds4_sdp, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ds4 sdp\n");
  }
  else
  {
    int ret = l2cap_send(states[btds4_number].ds4_sdp, sdp_ps4, sizeof(sdp_ps4), 0);

    if(ret != sizeof(sdp_ps4))
    {
      fprintf(stderr, "error writing ds4 sdp: %d\n", ret);
    }
  }

  return 0;
}

static int close_ds4_sdp(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ds4_sdp);
  state->ds4_sdp = -1;

  return 1;
}

static int read_ps4_sdp(int btds4_number)
{
  unsigned char buf[SDP_PACKET_SIZE];

  ssize_t len = l2cap_recv(states[btds4_number].ps4_sdp, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ps4 sdp\n");
  }
  else
  {
    int ret = acl_send_data(states[btds4_number].ps4_bdaddr, states[btds4_number].ps4_sdp_cid, sdp_ds4, sizeof(sdp_ds4));

    if(ret < 0)
    {
      fprintf(stderr, "error writing ps4 sdp\n");
    }
  }

  return 0;
}

static int close_ps4_control(int btds4_number);
static int connect_ps4_control(int btds4_number);
static int connect_ps4_interrupt(int btds4_number);

static int close_ps4_sdp(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ps4_sdp);
  state->ps4_sdp = -1;

  /*
   * Warning: this is really hackish...
   * There is an issue at the very first connection.
   * Maybe the acl_send_data hack interferes in some way with the l2cap sockets.
   * As a work-around, we disconnect and restart the connection.
   */
   
  GE_RemoveSource(state->ps4_control_pending);
  close(state->ps4_control_pending);
  state->ps4_control_pending = -1;

  GE_RemoveSource(state->ps4_interrupt_pending);
  close(state->ps4_interrupt_pending);
  state->ps4_interrupt_pending = -1;

  bt_disconnect(state->ps4_bdaddr);

  gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
      state->dongle_bdaddr, state->ps4_bdaddr, PSM_HID_CONTROL);

  if ((state->ps4_control_pending = l2cap_connect(state->dongle_bdaddr, state->ps4_bdaddr,
      PSM_HID_CONTROL, L2CAP_LM_MASTER | L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT)) < 0)
  {
    fprintf(stderr, "can't connect to control psm\n");
    return -1;
  }

  gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", state->dongle_index,
      state->dongle_bdaddr, state->ps4_bdaddr, PSM_HID_INTERRUPT);

  if ((state->ps4_interrupt_pending = l2cap_connect(state->dongle_bdaddr, state->ps4_bdaddr,
      PSM_HID_INTERRUPT, L2CAP_LM_MASTER | L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT)) < 0)
  {
    close(state->ps4_control_pending);
    state->ps4_control_pending = -1;
    fprintf(stderr, "can't connect to interrupt psm\n");
    return -1;
  }

  GE_AddSource(state->ps4_control_pending, btds4_number, NULL, &connect_ps4_control, &connect_ps4_control);
  GE_AddSource(state->ps4_interrupt_pending, btds4_number, NULL, &connect_ps4_interrupt, &connect_ps4_interrupt);

  return 1;
}

static int read_ds4_control(int btds4_number)
{
  unsigned char buf[1024];

  ssize_t len = l2cap_recv(states[btds4_number].ds4_control, buf, sizeof(buf));

  /*if(buf[1] == 0x04)
  {
    printf("alter report id 0x04\n");
    buf[4] = ~buf[4];
  }*/

  if(len < 0)
  {
    fprintf(stderr, "error reading ds4 control\n");
  }
  else
  {
    int ret = l2cap_send(states[btds4_number].ps4_control, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing ps4 control\n");
    }
  }

  return 0;
}

static int close_ds4_control(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ds4_control);
  state->ds4_control = -1;

  return 1;
}

//static int cpt = 0;

static int read_ds4_interrupt(int btds4_number)
{
  unsigned char buf[1024];

  int ret = l2cap_recv(states[btds4_number].ds4_interrupt, buf, sizeof(buf));

  if(ret < 0)
  {
    fprintf(stderr, "error reading ds4 interrupt\n");
  }

  return 0;

  /*unsigned char buf[1024];

  ssize_t len = l2cap_recv(states[btds4_number].ds4_interrupt, buf, sizeof(buf));

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

    int ret = l2cap_send(states[btds4_number].ps4_interrupt, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing %d bytes on ps4 interrupt\n", len);
    }
  }

  return 0;*/
}

static int close_ds4_interrupt(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ds4_interrupt);
  state->ds4_interrupt = -1;

  return 1;
}

static int read_ps4_control(int btds4_number)
{
  unsigned char buf[1024];

  ssize_t len = l2cap_recv(states[btds4_number].ps4_control, buf, sizeof(buf));

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

      if(l2cap_send(states[btds4_number].ps4_control, resp02, sizeof(resp02), 0) != sizeof(resp02))
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

      if(l2cap_send(states[btds4_number].ps4_control, resp06, sizeof(resp06), 0) != sizeof(resp06))
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

      if(l2cap_send(states[btds4_number].ps4_control, respa3, sizeof(respa3), 0) != sizeof(respa3))
      {
        fprintf(stderr, "error writing ps4 control\n");
      }

      return 0;
    }*/

    /*
     * todo: forward to USB device, and deactivate l2cap_send
     */

    if(buf[1] == 0x03 || buf[1] == 0x04)
    {
      struct timeval t;
      gettimeofday(&t, NULL);
      printf("%ld.%06ld ", t.tv_sec, t.tv_usec);
      printf("report id 0x%02x\n", buf[1]);
    }

    int ret = l2cap_send(states[btds4_number].ds4_control, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing ds4 control\n");
    }
  }

  return 0;
}

static int close_ps4_control(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ps4_control);
  state->ps4_control = -1;

  return 1;
}

static int connect_ps4_control(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  if(l2cap_is_connected(state->ps4_control_pending))
  {
    gprintf("connected\n");
    GE_RemoveSource(state->ps4_control_pending);
    state->ps4_control = state->ps4_control_pending;
    state->ps4_control_pending = -1;
    if(state->ds4_control >= 0)
    {
      GE_AddSource(state->ps4_control, btds4_number, &read_ps4_control, NULL, &close_ps4_control);
      GE_AddSource(state->ds4_control, btds4_number, &read_ds4_control, NULL, &close_ds4_control);
    }
  }
  else
  {
    GE_RemoveSource(state->ps4_control_pending);
    close(state->ps4_control_pending);
    state->ps4_control_pending = -1;
    fprintf(stderr, "can't connect to control psm\n");
    return -1;
  }

  return 0;
}

static int read_ps4_interrupt(int btds4_number)
{
  unsigned char buf[1024];

  ssize_t len = l2cap_recv(states[btds4_number].ps4_interrupt, buf, sizeof(buf));

  if(len < 0)
  {
    fprintf(stderr, "error reading ps4 interrupt\n");
  }
  else
  {
    /*
     * TODO MLA: process rumble!
     */

    /*switch(buf[2])
    {
      case 0xc0:
        printf("0x%02x 0x%02x 0x%02x\n", buf[1], buf[7], buf[8]);
        break;
    }*/

    /*int ret = l2cap_send(states[btds4_number].ds4_interrupt, buf, len, 0);

    if(ret != len)
    {
      fprintf(stderr, "error writing ds4 interrupt\n");
    }*/
  }

  return 0;

}

static int close_ps4_interrupt(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ps4_interrupt);
  state->ps4_interrupt = -1;
  state->sys.shutdown = 1;

  return 1;
}

static int connect_ps4_interrupt(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  if(l2cap_is_connected(state->ps4_interrupt_pending))
  {
    GE_RemoveSource(state->ps4_interrupt_pending);
    state->ps4_interrupt = state->ps4_interrupt_pending;
    state->ps4_interrupt_pending = -1;
    if(state->ds4_interrupt >= 0)
    {
      GE_AddSource(state->ps4_interrupt, btds4_number, &read_ps4_interrupt, NULL, &close_ps4_interrupt);
      GE_AddSource(state->ds4_interrupt, btds4_number, &read_ds4_interrupt, NULL, &close_ds4_interrupt);
    }
  }
  else
  {
    GE_RemoveSource(state->ps4_interrupt_pending);
    close(state->ps4_interrupt_pending);
    state->ps4_interrupt_pending = -1;
    fprintf(stderr, "can't connect to interrupt psm\n");
    return -1;
  }

  return 0;
}

static int btds4_accept(int listen_fd)
{
  bdaddr_t src;
  bdaddr_t cmp;
  unsigned short psm;
  unsigned short cid;
  int fd = l2cap_accept(listen_fd, &src, &psm, &cid);

  if(psm == PSM_SDP)
  {
    int i;
    for(i=0; i<sizeof(states)/sizeof(*states); ++i)
    {
      /*
       * Look for a PS4 address.
       */
      str2ba(states[i].ps4_bdaddr, &cmp);

      if(!memcmp(&src, &cmp, sizeof(src)))
      {
        states[i].ps4_sdp = fd;
        states[i].ps4_sdp_cid = cid;
        GE_AddSource(states[i].ps4_sdp, i, &read_ps4_sdp, NULL, &close_ps4_sdp);
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
          ba2str(&src, states[i].ds4_bdaddr);
          states[i].ds4_sdp = fd;
          GE_AddSource(states[i].ds4_sdp, i, &read_ds4_sdp, NULL, &close_ds4_sdp);

          gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
              states[i].dongle_bdaddr, states[i].ps4_bdaddr, PSM_HID_CONTROL);

          if ((states[i].ps4_control_pending = l2cap_connect(states[i].dongle_bdaddr, states[i].ps4_bdaddr,
              PSM_HID_CONTROL, L2CAP_LM_MASTER | L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT)) < 0)
          {
            fprintf(stderr, "can't connect to control psm\n");
            break;
          }

          GE_AddSource(states[i].ps4_control_pending, i, NULL, &connect_ps4_control, &connect_ps4_control);

          gprintf("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
              states[i].dongle_bdaddr, states[i].ps4_bdaddr, PSM_HID_INTERRUPT);

          if ((states[i].ps4_interrupt_pending = l2cap_connect(states[i].dongle_bdaddr, states[i].ps4_bdaddr,
              PSM_HID_INTERRUPT, L2CAP_LM_MASTER | L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT)) < 0)
          {
            close_ps4_control(i);
            fprintf(stderr, "can't connect to interrupt psm\n");
            break;
          }

          GE_AddSource(states[i].ps4_interrupt_pending, i, NULL, &connect_ps4_interrupt, &connect_ps4_interrupt);

          break;
        }
      }

      if(i == sizeof(states)/sizeof(*states))
      {
        fprintf(stderr, "no ps4/ds4 bdaddr found\n");
      }
    }
  }
  else
  {
    int i;
    for(i=0; i<sizeof(states)/sizeof(*states); ++i)
    {
      /*
       * Look for a DS4 address.
       */
      str2ba(states[i].ds4_bdaddr, &cmp);

      if(!memcmp(&src, &cmp, sizeof(src)))
      {
        if(psm == PSM_HID_CONTROL)
        {
          states[i].ds4_control = fd;
          if(states[i].ps4_control >= 0)
          {
            GE_AddSource(states[i].ds4_control, i, &read_ds4_control, NULL, &close_ds4_control);
            GE_AddSource(states[i].ps4_control, i, &read_ps4_control, NULL, &close_ps4_control);
          }
        }
        else if(psm == PSM_HID_INTERRUPT)
        {
          states[i].ds4_interrupt = fd;
          if(states[i].ps4_interrupt >= 0)
          {
            GE_AddSource(states[i].ds4_interrupt, i, &read_ds4_interrupt, NULL, &close_ds4_interrupt);
            GE_AddSource(states[i].ps4_interrupt, i, &read_ps4_interrupt, NULL, &close_ps4_interrupt);
          }
        }
        break;
      }
    }

    if(i == sizeof(states)/sizeof(*states))
    {
      fprintf(stderr, "no ds4 bdaddr found\n");
      close(fd);
    }
  }

  return 0;
}

static int btds4_close_listen(int listen_fd)
{
  if(listen_fd == sdp_fd)
  {
    printf("close sdp source\n");
    close(sdp_fd);
    sdp_fd = -1;
  }
  else if(listen_fd == hid_control_fd)
  {
    printf("close hid control source\n");
    close(hid_control_fd);
    hid_control_fd = -1;
  }
  else if(listen_fd == hid_interrupt_fd)
  {
    printf("close hid interrupt source\n");
    close(hid_interrupt_fd);
    hid_interrupt_fd = -1;
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

int btds4_init(int btds4_number)
{
  struct btds4_state* state = states+btds4_number;

  state->ps4_control = -1;
  state->ps4_interrupt = -1;
  state->ps4_control_pending = -1;
  state->ps4_interrupt_pending = -1;
  state->ps4_sdp = -1;
  state->ds4_control = -1;
  state->ds4_interrupt = -1;
  state->ds4_sdp = -1;
  state->ds4_sdp_pending = -1;

  memcpy(&state->bt_report, &init_report_btds4, sizeof(s_btds4_report));

  if(bt_mgmt_adapter_init(state->dongle_index) < 0)
  {
    fprintf(stderr, "failed to initialize bluetooth device\n");
    return -1;
  }

  if (bt_get_device_bdaddr(state->dongle_index, state->dongle_bdaddr) < 0)
  {
    fprintf(stderr, "failed to get device bdaddr\n");
    return -1;
  }
  state->btds4_number = btds4_number;

  if (bt_write_device_class(state->dongle_index, DS4_DEVICE_CLASS) < 0)
  {
    fprintf(stderr, "failed to set device class\n");
    return -1;
  }

  /*
   * todo: open a USB device, add event sources, and deactivate following code if successful
   */

  if(sdp_fd < 0)
  {
    if((sdp_fd = l2cap_listen(PSM_SDP, L2CAP_LM_MASTER)) >= 0)
    {
      GE_AddSource(sdp_fd, sdp_fd, &btds4_accept, NULL, &btds4_close_listen);
    }
    else
    {
      return -1;
    }
  }

  if(hid_control_fd < 0)
  {
    if((hid_control_fd = l2cap_listen(PSM_HID_CONTROL, L2CAP_LM_MASTER)) >= 0)
    {
      GE_AddSource(hid_control_fd, hid_control_fd, &btds4_accept, NULL, &btds4_close_listen);
    }
    else
    {
      return -1;
    }
  }

  if(hid_interrupt_fd < 0)
  {
    if((hid_interrupt_fd = l2cap_listen(PSM_HID_INTERRUPT, L2CAP_LM_MASTER)) >= 0)
    {
      GE_AddSource(hid_interrupt_fd, hid_interrupt_fd, &btds4_accept, NULL, &btds4_close_listen);
    }
    else
    {
      return -1;
    }
  }

  return 0;
}

void btds4_set_bdaddr(int btds4_number, char* dst)
{
  struct btds4_state* state = states+btds4_number;

  strncpy(state->ps4_bdaddr, dst, sizeof(state->ps4_bdaddr));
}

void btds4_set_dongle(int btds4_number, int dongle_index)
{
  struct btds4_state* state = states+btds4_number;

  state->dongle_index = dongle_index;
}

#define INACTIVITY_THRESHOLD 6000 //6000x10ms=60s

int btds4_send_interrupt(int btds4_number, s_report_ds4* report, int active)
{
  struct btds4_state* state = states + btds4_number;

  if(state->sys.shutdown)
  {
    return -1;
  }

  if(state->ps4_interrupt < 0 || state->ds4_interrupt < 0)
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

  memcpy(&state->bt_report.report, report, sizeof(s_report_ds4));

  MHASH td = mhash_init(MHASH_CRC32B);

  if (td == MHASH_FAILED)
  {
    perror("mhash_init");
  }

  mhash(td, &state->bt_report, sizeof(state->bt_report)-4);

  unsigned int digest = 0; // crc32 will be stored here

  mhash_deinit(td, &digest);

  state->bt_report.crc32[3] = digest >> 24;
  state->bt_report.crc32[2] = (digest >> 16) & 0xFF;
  state->bt_report.crc32[1] = (digest >> 8) & 0xFF;
  state->bt_report.crc32[0] = digest & 0xFF;

  int ret = l2cap_send(state->ps4_interrupt, (unsigned char*) &state->bt_report, sizeof(state->bt_report), 0);

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

  if(state->ps4_control >= 0)
  {
    bt_disconnect(state->ps4_bdaddr);
  }
  if(state->ds4_control >= 0)
  {
    bt_disconnect(state->ds4_bdaddr);
  }

  /*
   * todo: close any a USB device that was opened!
   */

  close_ps4_sdp(btds4_number);
  close_ps4_interrupt(btds4_number);
  close_ps4_control(btds4_number);

  /*
   * todo: deactivate this if we are working with a USB device
   */

  close_ds4_sdp(btds4_number);
  close_ds4_interrupt(btds4_number);
  close_ds4_control(btds4_number);
}
