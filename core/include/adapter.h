/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <connectors/serial.h>
#include <controller2.h>
#include <config.h>

#define MAX_CONTROLLERS 7
#define MAX_CONFIGURATIONS 8
#define MAX_DEVICES 256
#define MAX_CONTROLS 256

typedef struct
{
  char* bdaddr_dst;
  int dongle_index;
  char* portname;
  unsigned int dst_ip;
  unsigned short dst_port;
  int dst_fd;
  unsigned int src_ip;
  unsigned short src_port;
  int src_fd;
  e_controller_type type;
  int event;
  int axis[AXIS_MAX];
  int change;
  int send_command;
  int ts_axis[AXIS_MAX][2]; //issue 15
  s_report report;
} s_adapter;

void adapter_init();
inline s_adapter* adapter_get(unsigned char index);
int adapter_set_port(unsigned char index, char* portname);

void adapter_dump_state(int id);

int adapter_network_read(int id);
int adapter_network_close(int id);

void adapter_set_device(int controller, e_device_type device_type, int device_id);
int adapter_get_device(e_device_type device_type, int controller);
int adapter_get_controller(e_device_type device_type, int device_id);

void adapter_set_axis(unsigned char c, int axis, int value);

int adapter_forward_data_in(int id, unsigned char* data, unsigned char length);
int adapter_forward_data_out(int id, unsigned char* data, unsigned char length);

int adapter_get_type(int id);
int adapter_send_start(int id);
int adapter_get_status(int id);
int adapter_send_reset(int id);

#endif /* ADAPTER_H_ */
