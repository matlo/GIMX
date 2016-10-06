/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <connectors/protocol.h>
#include <controller2.h>
#include <config.h>
#include <gserial.h>

#ifndef WIN32
#include <netinet/in.h>
#else
#include <connectors/windows/sockets.h>
#endif

typedef enum
{
  E_ADAPTER_TYPE_NONE,
  E_ADAPTER_TYPE_BLUETOOTH,
  E_ADAPTER_TYPE_DIY_USB,
  E_ADAPTER_TYPE_REMOTE_GIMX,
  E_ADAPTER_TYPE_GPP,
} e_adapter_type;

typedef struct
{
  char* bdaddr_dst;
  int dongle_index;
  //TODO MLA: refactoring (struct, union...)
  char* portname;
  int serialdevice;
  s_packet packet;
  unsigned int bread;
  in_addr_t dst_ip;
  unsigned short dst_port;
  int dst_fd;
  in_addr_t src_ip;
  unsigned short src_port;
  int src_fd;
  e_adapter_type atype;
  e_controller_type ctype;
  int event;
  int axis[AXIS_MAX];
  int change;
  int send_command;
  int ts_axis[AXIS_MAX][2]; //issue 15
  s_report_packet report[2]; //the xbox one guide button needs a dedicated report
  int status;
  struct {
    int id;
    struct
    {
      unsigned short vendor;
      unsigned short product;
    } usb_ids;
    unsigned char has_rumble;
    unsigned char has_ffb;
    struct {
      int id;
      int write_pending;
      int read_pending;
    } hid;
  } joystick;
  unsigned char forward_out_reports;
  unsigned char process_ffb;
} s_adapter;

int adapter_detect();
int adapter_start();
int adapter_send();
void adapter_clean();

s_adapter* adapter_get(unsigned char index);
int adapter_set_port(unsigned char index, char* portname);

void adapter_set_device(int adapter, e_device_type device_type, int device_id);
int adapter_get_device(e_device_type device_type, int adapter);
int adapter_get_controller(e_device_type device_type, int device_id);

#ifndef WIN32
int adapter_hid_poll();
void adapter_set_hid(int adapter, int hid);
#else
void adapter_set_usb_ids(int adapter, int joystick_id, unsigned short vendor, unsigned short product);
#endif

void adapter_set_axis(unsigned char adapter, int axis, int value);

int adapter_forward_control_in(int adapter, unsigned char* data, unsigned char length);
int adapter_forward_interrupt_in(int adapter, unsigned char* data, unsigned char length);

int adapter_is_usb_auth_required(int adapter);

#endif /* ADAPTER_H_ */
