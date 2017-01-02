/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DFP_PS2_H_
#define DFP_PS2_H_

#include "defs.h"

typedef enum
{
  dfpPs2a_wheel = rel_axis_lstick_x,
  dfpPs2a_select = abs_axis_0,
  dfpPs2a_start = abs_axis_1,
  dfpPs2a_up = abs_axis_3,
  dfpPs2a_right = abs_axis_4,
  dfpPs2a_down = abs_axis_5,
  dfpPs2a_left = abs_axis_6,
  dfpPs2a_triangle = abs_axis_7,
  dfpPs2a_circle = abs_axis_8,
  dfpPs2a_cross = abs_axis_9,
  dfpPs2a_square = abs_axis_10,
  dfpPs2a_l1 = abs_axis_11,
  dfpPs2a_r1 = abs_axis_12,
  dfpPs2a_l2 = abs_axis_13,
  dfpPs2a_r2 = abs_axis_14,
  dfpPs2a_l3 = abs_axis_15,
  dfpPs2a_r3 = abs_axis_16,
  dfpPs2a_gasPedal = abs_axis_17,
  dfpPs2a_brakePedal = abs_axis_18,
  dfpPs2a_shifter_forward = abs_axis_27,
  dfpPs2a_shifter_back = abs_axis_28,
} e_dfpPs2_axis_index;

/*
 *
 */
typedef struct GIMX_PACKED
{
  unsigned short buttonsAndWheel; // 14 LSB = wheel, 2 MSB = buttons
  unsigned short hatAndButtons; // 12 LSB = buttons, 4 MSB = hat
  unsigned char unknown1;
  unsigned char gasPedal;
  unsigned char brakePedal;
  unsigned char unknown2;
} s_report_dfpPs2;

#endif /* DFP_PS2_H_ */
