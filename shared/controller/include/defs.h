/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef DEFS_H_
#define DEFS_H_

typedef enum
{
  C_TYPE_JOYSTICK = 0,
  C_TYPE_360_PAD,
  C_TYPE_SIXAXIS,
  C_TYPE_PS2_PAD,
  C_TYPE_XBOX_PAD,
  C_TYPE_DS4,
  C_TYPE_XONE_PAD,
  C_TYPE_T300RS_PS4,
  // <- add new types here
  C_TYPE_GPP,
  C_TYPE_DEFAULT,
  C_TYPE_MAX
} e_controller_type;

typedef struct
{
  int min_refresh_period;
  int default_refresh_period;
  int* max_unsigned_axis_value;
} s_controller_params;

typedef enum
{
  rel_axis_0 = 0,
  rel_axis_1,
  rel_axis_2,
  rel_axis_3,
  rel_axis_lstick_x = rel_axis_0,
  rel_axis_lstick_y = rel_axis_1,
  rel_axis_rstick_x = rel_axis_2,
  rel_axis_rstick_y = rel_axis_3,
  rel_axis_4,
  rel_axis_5,
  rel_axis_6,
  rel_axis_7,
  rel_axis_max = rel_axis_7,
  abs_axis_0,
  abs_axis_1,
  abs_axis_2,
  abs_axis_3,
  abs_axis_4,
  abs_axis_5,
  abs_axis_6,
  abs_axis_7,
  abs_axis_8,
  abs_axis_9,
  abs_axis_10,
  abs_axis_11,
  abs_axis_12,
  abs_axis_13,
  abs_axis_14,
  abs_axis_15,
  abs_axis_16,
  abs_axis_17,
  abs_axis_18,
  abs_axis_19,
  abs_axis_20,
  abs_axis_21,
  abs_axis_max = abs_axis_21,
  AXIS_MAX,
} e_controller_axis_index;

#endif /* DEFS_H_ */
