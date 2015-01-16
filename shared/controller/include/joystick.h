/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <control.h>

typedef enum
{
  jsa_lstick_x = rel_axis_lstick_x,
  jsa_lstick_y = rel_axis_lstick_y,
  jsa_rstick_x = rel_axis_rstick_x,
  jsa_rstick_y = rel_axis_rstick_y,
  jsa_up = abs_axis_3,
  jsa_right = abs_axis_4,
  jsa_down = abs_axis_5,
  jsa_left = abs_axis_6,
  jsa_B0 = abs_axis_10,
  jsa_B1 = abs_axis_9,
  jsa_B2 = abs_axis_8,
  jsa_B3 = abs_axis_7,
  jsa_B4 = abs_axis_11,
  jsa_B5 = abs_axis_12,
  jsa_B6 = abs_axis_13,
  jsa_B7 = abs_axis_14,
  jsa_B8 = abs_axis_0,
  jsa_B9 = abs_axis_1,
  jsa_B10 = abs_axis_15,
  jsa_B11 = abs_axis_16,
  jsa_B12 = abs_axis_2,
} e_joystick_axis_index;

typedef struct __attribute__ ((gcc_struct,packed))
{
  short X;
  short Y;
  short Z;
  short Rz;
  unsigned short Hat;
  unsigned short Bt;
} s_report_joystick;

#endif /* JOYSTICK_H_ */
