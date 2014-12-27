/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XONE_H_
#define XONE_H_

#include <control.h>

#define XONE_VENDOR 0x045e
#define XONE_PRODUCT 0x02d1

//TODO XONE
#define XONE_NAME "Controller"

#define XONE_USB_INTERRUPT_ENDPOINT_IN 1
#define XONE_USB_INTERRUPT_ENDPOINT_OUT 2
#define XONE_USB_INTERRUPT_PACKET_SIZE 32

#define XONE_USB_HID_IN_REPORT_ID 0x20

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

typedef struct __attribute__ ((packed))
{
  /*
   * TODO XONE
   */
  unsigned char type;
  unsigned char size;
  unsigned short buttons;
  unsigned char ltrigger;
  unsigned char rtrigger;
  unsigned short xaxis;
  unsigned short yaxis;
  unsigned short zaxis;
  unsigned short taxis;
  unsigned char unused[6];
} s_report_xone;

#endif /* XONE_H_ */
