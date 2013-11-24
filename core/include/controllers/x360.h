/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef X360_H_
#define X360_H_

#include "controllers/controller.h"

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

typedef struct
{
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
} s_report_x360;

#endif /* X360_H_ */
