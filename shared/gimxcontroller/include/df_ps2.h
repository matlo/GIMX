/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DF_PS2_H_
#define DF_PS2_H_

#include "defs.h"

typedef enum
{
  dfPs2a_wheel = rel_axis_lstick_x,
  dfPs2a_select = abs_axis_0,
  dfPs2a_start = abs_axis_1,
  dfPs2a_up = abs_axis_3,
  dfPs2a_right = abs_axis_4,
  dfPs2a_down = abs_axis_5,
  dfPs2a_left = abs_axis_6,
  dfPs2a_triangle = abs_axis_7,
  dfPs2a_circle = abs_axis_8,
  dfPs2a_cross = abs_axis_9,
  dfPs2a_square = abs_axis_10,
  dfPs2a_l1 = abs_axis_11,
  dfPs2a_r1 = abs_axis_12,
  dfPs2a_l2 = abs_axis_13,
  dfPs2a_r2 = abs_axis_14,
  dfPs2a_l3 = abs_axis_15,
  dfPs2a_r3 = abs_axis_16,
  dfPs2a_gasPedal = abs_axis_17,
  dfPs2a_brakePedal = abs_axis_18,
} e_dfPs2_axis_index;

/*
 *
 */
typedef struct GIMX_PACKED
{
  unsigned short buttonsAndWheel; // 10 LSB = wheel, 6 MSB = buttons
  unsigned char buttons;
  unsigned char unknown;
  unsigned char hat;
  unsigned char gasPedal;
  unsigned char brakePedal;
} s_report_dfPs2;

#endif /* DF_PS2_H_ */
