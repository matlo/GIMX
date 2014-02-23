/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <connectors/serial.h>
#include <config.h>

#define DEFAULT_MAX_AXIS_VALUE MAX_AXIS_VALUE_8BITS

#define MAX_AXIS_VALUE_8BITS 255
#define MAX_AXIS_VALUE_10BITS 1023
#define MAX_AXIS_VALUE_16BITS 65535

typedef enum
{
  C_TYPE_JOYSTICK = 0,
  C_TYPE_360_PAD,
  C_TYPE_SIXAXIS,
  C_TYPE_PS2_PAD,
  C_TYPE_XBOX_PAD,
  C_TYPE_DS4,
  C_TYPE_XONE_PAD,
  C_TYPE_GPP,
  C_TYPE_DEFAULT,
  C_TYPE_MAX
} e_controller_type;

typedef enum
{
  rel_axis_0 = 0,
  rel_axis_1,
  rel_axis_2,
  rel_axis_3,
  rel_axis_lstick_x = rel_axis_0,
  rel_axis_lstick_y = rel_axis_1,
  rel_axis_rstick_x = rel_axis_2,
  rel_axis_rstick_y = rel_axis_3,
  rel_axis_4,
  rel_axis_5,
  rel_axis_6,
  rel_axis_7,
  rel_axis_max = rel_axis_7,
  abs_axis_0,
  abs_axis_1,
  abs_axis_2,
  abs_axis_3,
  abs_axis_4,
  abs_axis_5,
  abs_axis_6,
  abs_axis_7,
  abs_axis_8,
  abs_axis_9,
  abs_axis_10,
  abs_axis_11,
  abs_axis_12,
  abs_axis_13,
  abs_axis_14,
  abs_axis_15,
  abs_axis_16,
  abs_axis_17,
  abs_axis_max = abs_axis_17,
  AXIS_MAX,
} e_controller_axis_index;

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
  SERIALOBJECT serial;
  e_controller_type type;
  int event;
  int axis[AXIS_MAX];
  int change;
  int send_command;
  int ts_axis[AXIS_MAX][2]; //issue 15
} s_controller;

void controller_init();
inline s_controller* get_controller(unsigned char index);
int controller_set_port(unsigned char index, char* portname);

void controller_gpp_set_refresh_periods(e_controller_type type);
int get_min_refresh_period(e_controller_type type);
int get_default_refresh_period(e_controller_type type);

inline int get_max_signed(e_controller_type, int);
inline int get_max_unsigned(e_controller_type, int);
inline int get_mean_unsigned(e_controller_type, int);
inline double get_axis_scale(e_controller_type, int);

void set_axis_value(unsigned char c, int axis, int value);

inline int clamp(int, int, int);

typedef struct {
    int value;
    int index;
} s_axis_index;

s_axis_index get_axis_index_from_name(const char*);

void controller_dump_state(s_controller* c);

int controller_network_read(int fd);

void controller_set_device(int controller, e_device_type device_type, int device_id);
int controller_get_device(e_device_type device_type, int controller);
int controller_get_controller(e_device_type device_type, int device_id);

#endif /* CONTROLLER_H_ */
