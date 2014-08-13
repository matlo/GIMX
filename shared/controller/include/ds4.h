/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DS4_H_
#define DS4_H_

#include <control.h>

#define DS4_USB_INTERRUPT_PACKET_SIZE 64

#define DS4_VENDOR 0x054c
#define DS4_PRODUCT 0x05c4

typedef enum
{
  ds4a_lstick_x = rel_axis_lstick_x,
  ds4a_lstick_y = rel_axis_lstick_y,
  ds4a_rstick_x = rel_axis_rstick_x,
  ds4a_rstick_y = rel_axis_rstick_y,
  ds4a_finger1_x = rel_axis_4,
  ds4a_finger1_y = rel_axis_5,
  ds4a_finger2_x = rel_axis_6,
  ds4a_finger2_y = rel_axis_7,
  ds4a_up = abs_axis_3,
  ds4a_right = abs_axis_4,
  ds4a_down = abs_axis_5,
  ds4a_left = abs_axis_6,
  ds4a_square = abs_axis_10,
  ds4a_cross = abs_axis_9,
  ds4a_circle = abs_axis_8,
  ds4a_triangle = abs_axis_7,
  ds4a_l1 = abs_axis_11,
  ds4a_r1 = abs_axis_12,
  ds4a_l2 = abs_axis_13,
  ds4a_r2 = abs_axis_14,
  ds4a_share = abs_axis_0,
  ds4a_options = abs_axis_1,
  ds4a_l3 = abs_axis_15,
  ds4a_r3 = abs_axis_16,
  ds4a_ps = abs_axis_2,
  ds4a_touchpad = abs_axis_17,
  ds4a_finger1 = abs_axis_18,
  ds4a_finger2 = abs_axis_19,
} e_ds4_axis_index;

typedef struct __attribute__ ((packed))
{
  unsigned char id;
  unsigned char coords[3];
} s_trackpad_finger;

typedef struct __attribute__ ((packed))
{
  unsigned char counter;
  s_trackpad_finger finger1;
  s_trackpad_finger finger2;
} s_trackpad_packet;

/*
 *
 */
typedef struct __attribute__ ((packed))
{
  unsigned char report_id; //USB: 0x01, BT: 0x00
  unsigned char X;
  unsigned char Y;
  unsigned char Z;
  unsigned char Rz;
  unsigned char HatAndButtons;
  unsigned short ButtonsAndCounter;
  unsigned char Rx;
  unsigned char Ry;
  unsigned char _time[2];
  unsigned char battery_level;
  unsigned char rel_gyro[6];
  unsigned char abs_gyro[6];
  unsigned char _unknown1[5];
  unsigned char ext;
  unsigned char _unknown2[2];
  unsigned char packets;
  s_trackpad_packet packet1;
  s_trackpad_packet packet2;
  s_trackpad_packet packet3;
  s_trackpad_packet packet4;
  unsigned char _unknown5[2];
} s_report_ds4;

void ds4_init_report(s_report_ds4* ds4);

#endif /* DS4_H_ */
