/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XBOX_H_
#define XBOX_H_

#include "defs.h"

#define XBOX_VENDOR 0x045e
#define XBOX_PRODUCT 0x0289

#define XBOX_UP_MASK    0x01
#define XBOX_DOWN_MASK  0x02
#define XBOX_LEFT_MASK  0x04
#define XBOX_RIGHT_MASK 0x08

#define XBOX_START_MASK 0x10
#define XBOX_BACK_MASK  0x20
#define XBOX_LS_MASK    0x40
#define XBOX_RS_MASK    0x80

typedef enum
{
  xboxa_lstick_x = rel_axis_lstick_x,
  xboxa_lstick_y = rel_axis_lstick_y,
  xboxa_rstick_x = rel_axis_rstick_x,
  xboxa_rstick_y = rel_axis_rstick_y,
  xboxa_back = abs_axis_0,
  xboxa_start = abs_axis_1,
  xboxa_up = abs_axis_3,
  xboxa_right = abs_axis_4,
  xboxa_down = abs_axis_5,
  xboxa_left = abs_axis_6,
  xboxa_Y = abs_axis_7,
  xboxa_B = abs_axis_8,
  xboxa_A = abs_axis_9,
  xboxa_X = abs_axis_10,
  xboxa_white = abs_axis_11,
  xboxa_black = abs_axis_12,
  xboxa_LT = abs_axis_13,
  xboxa_RT = abs_axis_14,
  xboxa_LS = abs_axis_15,
  xboxa_RS = abs_axis_16,
} e_xbox_axis_index;

typedef struct GIMX_PACKED
{
  unsigned char type; //0x00
  unsigned char size;     //0x14
  unsigned char buttons;
  unsigned char ununsed2; //0x00
  unsigned char btnA;
  unsigned char btnB;
  unsigned char btnX;
  unsigned char btnY;
  unsigned char btnBlack;
  unsigned char btnWhite;
  unsigned char ltrigger;
  unsigned char rtrigger;
  short xaxis;
  short yaxis;
  short zaxis;
  short taxis;
} s_report_xbox;

typedef struct
{
  // TODO MLA
} s_gc_state_xbox;

#endif /* XBOX_H_ */
