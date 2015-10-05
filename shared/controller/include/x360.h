/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef X360_H_
#define X360_H_

#include <defs.h>

#define X360_VENDOR 0x045e
#define X360_PRODUCT 0x028e

#ifndef WIN32
#define X360_NAME "Microsoft X-Box 360 pad"
#else
#define X360_NAME "X360 Controller"
#endif

#define X360_USB_INTERRUPT_ENDPOINT_IN 1
#define X360_USB_INTERRUPT_ENDPOINT_OUT 2
#define X360_USB_INTERRUPT_PACKET_SIZE 20

#define X360_USB_HID_IN_REPORT_ID 0x00

#define X360_UP_MASK    0x0001
#define X360_DOWN_MASK  0x0002
#define X360_LEFT_MASK  0x0004
#define X360_RIGHT_MASK 0x0008

#define X360_START_MASK 0x0010
#define X360_BACK_MASK  0x0020
#define X360_LS_MASK    0x0040
#define X360_RS_MASK    0x0080

#define X360_LB_MASK    0x0100
#define X360_RB_MASK    0x0200
#define X360_GUIDE_MASK 0x0400

#define X360_A_MASK 0x1000
#define X360_B_MASK 0x2000
#define X360_X_MASK 0x4000
#define X360_Y_MASK 0x8000

#ifndef WIN32
#define X360_UP_ID    11
#define X360_DOWN_ID  13
#define X360_LEFT_ID  14
#define X360_RIGHT_ID 12

#define X360_START_ID 7
#define X360_BACK_ID  6
#define X360_LS_ID    9
#define X360_RS_ID    10

#define X360_LB_ID    4
#define X360_RB_ID    5
#define X360_GUIDE_ID 8

#define X360_A_ID 0
#define X360_B_ID 1
#define X360_X_ID 2
#define X360_Y_ID 3

#define X360_LT_ID 2
#define X360_RT_ID 5

#define X360_AXIS_X_ID  0
#define X360_AXIS_Y_ID  1
#define X360_AXIS_Z_ID  3
#define X360_AXIS_RZ_ID 4
#else
#define X360_UP_ID    11
#define X360_DOWN_ID  12
#define X360_LEFT_ID  13
#define X360_RIGHT_ID 14

#define X360_START_ID 6
#define X360_BACK_ID  4
#define X360_LS_ID    7
#define X360_RS_ID    8

#define X360_LB_ID    9
#define X360_RB_ID    10
#define X360_GUIDE_ID 5

#define X360_A_ID 0
#define X360_B_ID 1
#define X360_X_ID 2
#define X360_Y_ID 3

#define X360_LT_ID 4
#define X360_RT_ID 5

#define X360_AXIS_X_ID  0
#define X360_AXIS_Y_ID  1
#define X360_AXIS_Z_ID  2
#define X360_AXIS_RZ_ID 3
#endif

typedef enum
{
  x360a_lstick_x = rel_axis_lstick_x,
  x360a_lstick_y = rel_axis_lstick_y,
  x360a_rstick_x = rel_axis_rstick_x,
  x360a_rstick_y = rel_axis_rstick_y,
  x360a_back = abs_axis_0,
  x360a_start = abs_axis_1,
  x360a_guide = abs_axis_2,
  x360a_up = abs_axis_3,
  x360a_right = abs_axis_4,
  x360a_down = abs_axis_5,
  x360a_left = abs_axis_6,
  x360a_Y = abs_axis_7,
  x360a_B = abs_axis_8,
  x360a_A = abs_axis_9,
  x360a_X = abs_axis_10,
  x360a_LB = abs_axis_11,
  x360a_RB = abs_axis_12,
  x360a_LT = abs_axis_13,
  x360a_RT = abs_axis_14,
  x360a_LS = abs_axis_15,
  x360a_RS = abs_axis_16,
} e_x360_axis_index;

typedef struct GIMX_PACKED
{
  unsigned char type;
  unsigned char size;
  unsigned short buttons;
  unsigned char ltrigger;
  unsigned char rtrigger;
  short xaxis;
  short yaxis;
  short zaxis;
  short taxis;
  unsigned char unused[6];
} s_report_x360;

#endif /* X360_H_ */
