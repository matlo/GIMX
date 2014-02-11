/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "controllers/ds4.h"
#include "connectors/btds4.h"
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
    /*** Values provided by the system (PS3): */
    int reporting_enabled;
    int shutdown;
    int feature_ef_byte_6;


    uint8_t rumble[2];
};

struct btds4_state {
    char dongle_bdaddr[18];
    char ps4_bdaddr[18];
    char ds4_bdaddr[18];
    int dongle_index;
    int btds4_number;
    struct btds4_state_sys sys;
    s_report_ds4 user;
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

/*
 * TODO MLA: fix table size
 */
static struct btds4_state states[7] = {};
static int debug = 1;

static int sdp_fd = -1;
static int hid_control_fd = -1;
static int hid_interrupt_fd = -1;

static const char *hid_report_name[] =
{ "reserved", "input", "output", "feature" };

static int is_connected(int fd)
{
  int error = 0;
  socklen_t lerror = sizeof(error);

  int ret = getsockopt (fd, SOL_SOCKET, SO_ERROR, &error, &lerror);

  if(ret < 0)
  {
    perror("getsockopt SO_ERROR");
    exit(-1);
  }
  else
  {
    if(error == EINPROGRESS)
    {
      fprintf(stderr, "EINPROGRESS\n");
    }
    else if(error < 0)
    {
      fprintf(stderr, "connection failed: %s\n", strerror(error));
    }
    else
    {
      return 1;
    }
  }
  return 0;
}

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

    if(ret != len)
    {
      fprintf(stderr, "error writing ds4 sdp\n");
    }
  }

  return 0;
}

static int close_ps4_sdp(int btds4_number)
{
  struct btds4_state* state = states + btds4_number;

  close(state->ps4_sdp);
  state->ps4_sdp = -1;

  return 1;
}

static int read_ds4_control(int btds4_number)
{
  unsigned char buf[1024];

  ssize_t len = l2cap_recv(states[btds4_number].ds4_control, buf, sizeof(buf));

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

static int process(int psm, const unsigned char *buf, int len, struct btds4_state *state);



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

  if(is_connected(state->ps4_control_pending))
  {
    GE_RemoveSource(state->ps4_control_pending);
    state->ps4_control = state->ps4_control_pending;
    state->ps4_control_pending = -1;
    if(state->ds4_control >= 0)
    {
      GE_AddSource(state->ps4_control, POLLIN, btds4_number, &read_ps4_control, &close_ps4_control);
      GE_AddSource(state->ds4_control, POLLIN, btds4_number, &read_ds4_control, &close_ds4_control);
    }
  }
  else
  {
    GE_RemoveSource(state->ps4_control_pending);
    close(state->ps4_control_pending);
    state->ps4_control_pending = -1;
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

  if(is_connected(state->ps4_interrupt_pending))
  {
    GE_RemoveSource(state->ps4_interrupt_pending);
    state->ps4_interrupt = state->ps4_interrupt_pending;
    state->ps4_interrupt_pending = -1;
    if(state->ds4_interrupt >= 0)
    {
      GE_AddSource(state->ps4_interrupt, POLLIN, btds4_number, &read_ps4_interrupt, &close_ps4_interrupt);
      GE_AddSource(state->ds4_interrupt, POLLIN, btds4_number, &read_ds4_interrupt, &close_ds4_interrupt);
    }
  }
  else
  {
    GE_RemoveSource(state->ps4_interrupt_pending);
    close(state->ps4_interrupt_pending);
    state->ps4_interrupt_pending = -1;
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
        GE_AddSource(states[i].ps4_sdp, POLLIN, i, &read_ps4_sdp, &close_ps4_sdp);
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
          GE_AddSource(states[i].ds4_sdp, POLLIN, i, &read_ds4_sdp, &close_ds4_sdp);

          printf("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
              states[i].dongle_bdaddr, states[i].ps4_bdaddr, PSM_HID_CONTROL);

          if ((states[i].ps4_control_pending = l2cap_connect(states[i].dongle_bdaddr, states[i].ps4_bdaddr,
              PSM_HID_CONTROL)) < 0)
          {
            fprintf(stderr, "can't connect to control psm\n");
            break;
          }

          GE_AddSource(states[i].ps4_control_pending, POLLOUT, i, &connect_ps4_control, &close);

          printf("connecting with hci%d = %s to %s psm 0x%04x\n", states[i].dongle_index,
              states[i].dongle_bdaddr, states[i].ps4_bdaddr, PSM_HID_INTERRUPT);

          if ((states[i].ps4_interrupt_pending = l2cap_connect(states[i].dongle_bdaddr, states[i].ps4_bdaddr,
              PSM_HID_INTERRUPT)) < 0)
          {
            close_ps4_control(i);
            fprintf(stderr, "can't connect to interrupt psm\n");
            break;
          }

          GE_AddSource(states[i].ps4_interrupt_pending, POLLOUT, i, &connect_ps4_interrupt, &close);

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
            GE_AddSource(states[i].ds4_control, POLLIN, i, &read_ds4_control, &close_ds4_control);
            GE_AddSource(states[i].ps4_control, POLLIN, i, &read_ps4_control, &close_ps4_control);
          }
        }
        else if(psm == PSM_HID_INTERRUPT)
        {
          states[i].ds4_interrupt = fd;
          if(states[i].ps4_interrupt >= 0)
          {
            GE_AddSource(states[i].ds4_interrupt, POLLIN, i, &read_ds4_interrupt, &close_ds4_interrupt);
            GE_AddSource(states[i].ps4_interrupt, POLLIN, i, &read_ps4_interrupt, &close_ps4_interrupt);
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

int btds4_init(int btds4_number)
{
  struct btds4_state* state = states+btds4_number;

  state->sys.reporting_enabled = 0;
  state->sys.feature_ef_byte_6 = 0xb0;

  state->ps4_control = -1;
  state->ps4_interrupt = -1;
  state->ps4_control_pending = -1;
  state->ps4_interrupt_pending = -1;
  state->ps4_sdp = -1;
  state->ds4_control = -1;
  state->ds4_interrupt = -1;
  state->ds4_sdp = -1;
  state->ds4_sdp_pending = -1;

  state->user.X = 128;
  state->user.Y = 128;
  state->user.Z = 128;
  state->user.Rz = 128;

  state->user.HatAndButtons = 0x08;

  /*
   * TDO MLA: initialize other values
   */

  if (bt_get_device_bdaddr(state->dongle_index, state->dongle_bdaddr) < 0)
  {
    fprintf(stderr, "failed to get device number\n");
    return -1;
  }
  state->btds4_number = btds4_number;

  if (bt_write_device_class(state->dongle_index, DS4_DEVICE_CLASS) < 0)
  {
    fprintf(stderr, "failed to set device class\n");
    return -1;
  }

  if(sdp_fd < 0)
  {
    if((sdp_fd = l2cap_listen(PSM_SDP)) >= 0)
    {
      GE_AddSource(sdp_fd, POLLIN, sdp_fd, &btds4_accept, &btds4_close_listen);
    }
    else
    {
      return -1;
    }
  }

  if(hid_control_fd < 0)
  {
    if((hid_control_fd = l2cap_listen(PSM_HID_CONTROL)) >= 0)
    {
      GE_AddSource(hid_control_fd, POLLIN, hid_control_fd, &btds4_accept, &btds4_close_listen);
    }
    else
    {
      return -1;
    }
  }

  if(hid_interrupt_fd < 0)
  {
    if((hid_interrupt_fd = l2cap_listen(PSM_HID_INTERRUPT)) >= 0)
    {
      GE_AddSource(hid_interrupt_fd, POLLIN, hid_interrupt_fd, &btds4_accept, &btds4_close_listen);
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

/* Main input report from Sixaxis -- assemble it */
static int assemble_input_01(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  const uint8_t data[] =
  {
    /*
     *  TODO MLA
     *
     *  4 axes + d-pad + buttons?
     */
    0x7d, 0x7d, 0x80, 0x7e, 0x08, 0x00, 0x00, 0x00, 0x00
  };

  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

static int assemble_input_11(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  uint8_t data[79] =
  {
    0xa1, 0x11, 0xc0, 0x00, 0x7d, 0x7d, 0x81, 0x7e,
    0x08, 0x00, 0x28, 0x00, 0x00, 0x8c, 0xf3, 0x01,
    0x13, 0x00, 0xf8, 0xff, 0x05, 0x00, 0x31, 0xfe,
    0x3f, 0x0f, 0xd1, 0xe3, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x5e, 0x22, 0x7b, 0xa0
  };

  memcpy(data+4, ((uint8_t*) &state->user) + 1, 9);

  MHASH td = mhash_init(MHASH_CRC32B);

  if (td == MHASH_FAILED) {
    perror("mhash_init");
  }

  mhash(td, data, 75);

  unsigned int digest = 0; // crc32 will be stored here

  mhash_deinit(td, &digest);

  data[78] = digest >> 24;
  data[77] = (digest >> 16) & 0xFF;
  data[76] = (digest >> 8) & 0xFF;
  data[75] = digest & 0xFF;

  int len = sizeof(data) - 2;
  if (len > maxlen)
    return -1;
  memcpy(buf, data+2, len);

  return len;
}

/* Unknown */
static int assemble_feature_02(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  const uint8_t data[] =
  {
    0x01, 0x00, 0xff, 0xff, 0x01, 0x00, 0x5e, 0x22,
    0x84, 0x22, 0x9b, 0x22, 0xa6, 0xdd, 0x79, 0xdd,
    0x64, 0xdd, 0x1c, 0x02, 0x1c, 0x02, 0x85, 0x1f,
    0x9f, 0xe0, 0x92, 0x20, 0xdc, 0xe0, 0x4d, 0x1c,
    0x1e, 0xde, 0x08, 0x00
  };

  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

/* Unknown */
static int assemble_feature_04(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  /*
   * TODO MLA: forward
   */

  return 0;
}

static int assemble_feature_06(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  const uint8_t data[] =
  {
    0x41, 0x75, 0x67, 0x20, 0x20, 0x33, 0x20, 0x32,
    0x30, 0x31, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x30, 0x37, 0x3a, 0x30, 0x31, 0x3a, 0x31, 0x32,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x31, 0x03, 0x00, 0x00, 0x00,
    0x49, 0x00, 0x05, 0x00, 0x00, 0x80, 0x03, 0x00,
    0x4b, 0x52, 0x02, 0xc7 /* <-- CRC-32 */
  };

  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

static int assemble_feature_a3(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  const uint8_t data[] =
  {
    0x41, 0x75, 0x67, 0x20, 0x20, 0x33, 0x20, 0x32,
    0x30, 0x31, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x30, 0x37, 0x3a, 0x30, 0x31, 0x3a, 0x31, 0x32,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x31, 0x03, 0x00, 0x00, 0x00,
    0x49, 0x00, 0x05, 0x00, 0x00, 0x80, 0x03, 0x00
  };

  int len = sizeof(data);
  if (len > maxlen)
    return -1;
  memcpy(buf, data, len);

  return len;
}

static int assemble_feature_f1(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  /*
   * TODO MLA: forward
   */

  return 0;
}

static int assemble_feature_f2(uint8_t *buf, int maxlen, struct btds4_state *state)
{
  /*
   *  TODO MLA: forward
   */

  return 0;
}

static int process_input_11(const uint8_t *buf, int len, struct btds4_state *state)
{
  if (len < sizeof(s_report_ds4))
    return -1;

  memcpy(&state->user, buf, sizeof(s_report_ds4));

  return 0;
}

static int process_output_11(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_output_14(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_output_15(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_output_17(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_output_18(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_output_19(const uint8_t *buf, int len, struct btds4_state *state)
{
  return 0;
}

static int process_feature_03(const uint8_t *buf, int len, struct btds4_state *state)
{
  /*
   * TODO MLA: forward
   */

  return 0;
}

static int process_feature_f0(const uint8_t *buf, int len, struct btds4_state *state)
{
  /*
   * TODO MLA: forward
   */

  return 0;
}

static struct btds4_assemble_t btds4_assemble[] =
{
{ HID_TYPE_INPUT, 0x01, assemble_input_01 },
{ HID_TYPE_INPUT, 0x11, assemble_input_11 },
{ HID_TYPE_FEATURE, 0x02, assemble_feature_02 },
{ HID_TYPE_FEATURE, 0x04, assemble_feature_04 },
{ HID_TYPE_FEATURE, 0x06, assemble_feature_06 },
{ HID_TYPE_FEATURE, 0xa3, assemble_feature_a3 },
{ HID_TYPE_FEATURE, 0xf1, assemble_feature_f1 },
{ HID_TYPE_FEATURE, 0xf2, assemble_feature_f2 },
{ 0 } };

static struct btds4_process_t btds4_process[] =
{
{ HID_TYPE_INPUT, 0x11, process_input_11 },
{ HID_TYPE_OUTPUT, 0x11, process_output_11 },
{ HID_TYPE_OUTPUT, 0x14, process_output_14 },
{ HID_TYPE_OUTPUT, 0x15, process_output_15 },
{ HID_TYPE_OUTPUT, 0x17, process_output_17 },
{ HID_TYPE_OUTPUT, 0x18, process_output_18 },
{ HID_TYPE_OUTPUT, 0x19, process_output_19 },
{ HID_TYPE_FEATURE, 0x03, process_feature_03 },
{ HID_TYPE_FEATURE, 0xf0, process_feature_f0 },
{ 0 } };

static int send_report(int fd, uint8_t type, uint8_t report,
    struct btds4_state *state, int blocking)
{
  uint8_t buf[128];
  int len = 0;
  int i;
  struct timeval tv;

  /* Assemble report */
  for (i = 0; btds4_assemble[i].func; i++)
  {
    if (btds4_assemble[i].type == type
        && btds4_assemble[i].report == report)
    {
      len = btds4_assemble[i].func(&buf[2], sizeof(buf) - 2, state);
      break;
    }
  }

  if (!btds4_assemble[i].func || len < 0)
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
  return l2cap_send(fd, buf, len, blocking);
}

static int process_report(uint8_t type, uint8_t report, const uint8_t *buf, int len,
    struct btds4_state *state)
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
  for (i = 0; btds4_process[i].func; i++)
  {
    if (btds4_process[i].type == type && btds4_process[i].report == report)
    {
      ret = btds4_process[i].func(buf, len, state);
      break;
    }
  }

  if (!btds4_process[i].func || ret < 0)
  {
    printf("%s %s report 0x%02x\n", (ret < 0) ? "Error processing" : "Unknown",
        hid_report_name[type], report);
  }

  return ret;
}

static int process(int psm, const unsigned char *buf, int len,
    struct btds4_state *state)
{
  uint8_t transaction;
  uint16_t maxsize;
  uint8_t type;
  uint8_t report;
  const char *name;
  int ret = 0;
  struct timeval tv1, tv2;
  unsigned long time;

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
      ret = send_report(
          psm == PSM_HID_CONTROL ? state->ps4_control : state->ps4_interrupt, type,
          report, state, 1);
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
        char foo = (HID_HANDSHAKE << 4) | 0x0;
        if (write(state->ps4_control, &foo, 1) < 1)
        {
          fprintf(stderr, "write error\n");
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

  return ret;
}

int btds4_send_interrupt(int btds4_number, s_report_ds4* buf)
{
  struct btds4_state* state = states + btds4_number;

  if(state->sys.shutdown)
  {
    printf("shutdown\n");
    return -1;
  }

  if(state->ps4_interrupt < 0 || state->ds4_interrupt < 0)
  {
    return 0;
  }

  process_report(HID_TYPE_INPUT, 0x11, (unsigned char*) buf, sizeof(s_report_ds4), state);

  int ret = send_report(state->ps4_interrupt, HID_TYPE_INPUT, 0x11, state, 0);

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

  bt_disconnect(state->ps4_bdaddr);

  close_ps4_sdp(btds4_number);
  close_ps4_interrupt(btds4_number);
  close_ps4_control(btds4_number);
  close_ds4_sdp(btds4_number);
  close_ds4_interrupt(btds4_number);
  close_ds4_control(btds4_number);
}
