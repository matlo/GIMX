/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XONE_H_
#define XONE_H_

#include <defs.h>

#define XONE_VENDOR 0x045e
#define XONE_PRODUCT 0x02d1

#define XONE_NAME "Microsoft X-Box One pad"

#define XONE_USB_INTERRUPT_ENDPOINT_IN 1
#define XONE_USB_INTERRUPT_ENDPOINT_OUT 1
#define XONE_USB_INTERRUPT_PACKET_SIZE 64

#define XONE_USB_HID_IN_REPORT_ID 0x20

#define XONE_LB_MASK    0x1000
#define XONE_RB_MASK    0x2000
#define XONE_LS_MASK    0x4000
#define XONE_RS_MASK    0x8000

#define XONE_UP_MASK    0x0100
#define XONE_DOWN_MASK  0x0200
#define XONE_LEFT_MASK  0x0400
#define XONE_RIGHT_MASK 0x0800

#define XONE_A_MASK     0x0010
#define XONE_B_MASK     0x0020
#define XONE_X_MASK     0x0040
#define XONE_Y_MASK     0x0080

#define XONE_MENU_MASK  0x0004
#define XONE_VIEW_MASK  0x0008

#define XONE_USB_HID_IN_GUIDE_REPORT_ID 0x07

#define XONE_GUIDE_MASK 0x01

#define XONE_USB_HID_RUMBLE_REPORT_ID 0x09

typedef enum
{
  xonea_lstick_x = rel_axis_lstick_x,
  xonea_lstick_y = rel_axis_lstick_y,
  xonea_rstick_x = rel_axis_rstick_x,
  xonea_rstick_y = rel_axis_rstick_y,
  xonea_view = abs_axis_0,
  xonea_menu = abs_axis_1,
  xonea_guide = abs_axis_2,
  xonea_up = abs_axis_3,
  xonea_right = abs_axis_4,
  xonea_down = abs_axis_5,
  xonea_left = abs_axis_6,
  xonea_Y = abs_axis_7,
  xonea_B = abs_axis_8,
  xonea_A = abs_axis_9,
  xonea_X = abs_axis_10,
  xonea_LB = abs_axis_11,
  xonea_RB = abs_axis_12,
  xonea_LT = abs_axis_13,
  xonea_RT = abs_axis_14,
  xonea_LS = abs_axis_15,
  xonea_RS = abs_axis_16,
} e_xone_axis_index;

typedef struct GIMX_PACKED
{
  union
  {
	  unsigned char type;
    struct GIMX_PACKED
    {
      unsigned char type;
      unsigned char unknown;
      unsigned char counter;
      unsigned char size;
      unsigned short buttons;
      unsigned short ltrigger;
      unsigned short rtrigger;
      short xaxis;
      short yaxis;
      short zaxis;
      short taxis;
    } input;
    struct GIMX_PACKED
    {
      unsigned char type;
      unsigned char unknown1;
      unsigned char counter;
      unsigned char size;
      unsigned char button;
      unsigned char unknown2;
    } guide;
  };
} s_report_xone;

#endif /* XONE_H_ */
