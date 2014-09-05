/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DS4_H_
#define DS4_H_

#include <control.h>

#define DS4_USB_INTERRUPT_ENDPOINT_IN 4
#define DS4_USB_INTERRUPT_PACKET_SIZE 64
#ifndef WIN32
#define DS4_DEVICE_NAME "Sony Computer Entertainment Wireless Controller"
#else
#define DS4_DEVICE_NAME "PS4 Controller"
#endif

#define DS4_VENDOR 0x054c
#define DS4_PRODUCT 0x05c4

#define DS4_UP_MASK         0x01
#define DS4_RIGHT_MASK      0x02
#define DS4_DOWN_MASK       0x04
#define DS4_LEFT_MASK       0x08

#define DS4_SQUARE_MASK     0x0010
#define DS4_CROSS_MASK      0x0020
#define DS4_CIRCLE_MASK     0x0040
#define DS4_TRIANGLE_MASK   0x0080

#define DS4_L1_MASK         0x0001
#define DS4_R1_MASK         0x0002
#define DS4_L2_MASK         0x0004
#define DS4_R2_MASK         0x0008

#define DS4_SHARE_MASK      0x0010
#define DS4_OPTIONS_MASK    0x0020
#define DS4_L3_MASK         0x0040
#define DS4_R3_MASK         0x0080

#define DS4_PS_MASK         0x0100
#define DS4_TOUCHPAD_MASK   0x0200

#ifndef WIN32
#define DS4_SQUARE_ID     0
#define DS4_CROSS_ID      1
#define DS4_CIRCLE_ID     2
#define DS4_TRIANGLE_ID   3

#define DS4_L1_ID         4
#define DS4_R1_ID         5
#define DS4_L2_ID         6
#define DS4_R2_ID         7

#define DS4_SHARE_ID      8
#define DS4_OPTIONS_ID    9
#define DS4_L3_ID         10
#define DS4_R3_ID         11

#define DS4_PS_ID         12
#define DS4_TOUCHPAD_ID   13

#define DS4_UP_ID         14
#define DS4_RIGHT_ID      15
#define DS4_DOWN_ID       16
#define DS4_LEFT_ID       17

#define DS4_AXIS_X_ID     0
#define DS4_AXIS_Y_ID     1
#define DS4_AXIS_Z_ID     2
#define DS4_AXIS_L2_ID    3
#define DS4_AXIS_R2_ID    4
#define DS4_AXIS_RZ_ID    5
#else
#define DS4_SQUARE_ID     2
#define DS4_CROSS_ID      0
#define DS4_CIRCLE_ID     1
#define DS4_TRIANGLE_ID   3

#define DS4_L1_ID         9
#define DS4_R1_ID         10
#define DS4_L2_ID         10//bug?
#define DS4_R2_ID         7//bug?

#define DS4_SHARE_ID      4
#define DS4_OPTIONS_ID    6
#define DS4_L3_ID         7
#define DS4_R3_ID         8

#define DS4_PS_ID         5
#define DS4_TOUCHPAD_ID   13//bug?

#define DS4_UP_ID         11
#define DS4_RIGHT_ID      14
#define DS4_DOWN_ID       12
#define DS4_LEFT_ID       13

#define DS4_AXIS_X_ID     0
#define DS4_AXIS_Y_ID     1
#define DS4_AXIS_Z_ID     2
#define DS4_AXIS_RZ_ID    3
#define DS4_AXIS_L2_ID    4
#define DS4_AXIS_R2_ID    5
#endif

#define DS4_TRACKPAD_MAX_X 1919
#define DS4_TRACKPAD_MAX_Y 919

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
