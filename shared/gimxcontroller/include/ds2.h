/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DS2_H_
#define DS2_H_

#include "defs.h"

typedef enum
{
  ds2a_lstick_x = rel_axis_lstick_x,
  ds2a_lstick_y = rel_axis_lstick_y,
  ds2a_rstick_x = rel_axis_rstick_x,
  ds2a_rstick_y = rel_axis_rstick_y,
  ds2a_select = abs_axis_0,
  ds2a_start = abs_axis_1,
  ds2a_up = abs_axis_3,
  ds2a_right = abs_axis_4,
  ds2a_down = abs_axis_5,
  ds2a_left = abs_axis_6,
  ds2a_triangle = abs_axis_7,
  ds2a_circle = abs_axis_8,
  ds2a_cross = abs_axis_9,
  ds2a_square = abs_axis_10,
  ds2a_l1 = abs_axis_11,
  ds2a_r1 = abs_axis_12,
  ds2a_l2 = abs_axis_13,
  ds2a_r2 = abs_axis_14,
  ds2a_l3 = abs_axis_15,
  ds2a_r3 = abs_axis_16,
} e_ds2_axis_index;

typedef struct GIMX_PACKED
{
  unsigned char head;
  unsigned char Bt1;
  unsigned char Bt2;
  char Z;
  char Rz;
  char X;
  char Y;
} s_report_ds2;

#endif /* DS2_H_ */
