/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DEFS_H_
#define DEFS_H_

#ifdef WIN32
#define GIMX_PACKED __attribute__((gcc_struct, packed))
#else
#define GIMX_PACKED __attribute__((packed))
#endif

#define MAX_AXIS_VALUE_8BITS 255
#define MAX_AXIS_VALUE_10BITS 1023
#define MAX_AXIS_VALUE_12BITS 4095
#define MAX_AXIS_VALUE_14BITS 16383
#define MAX_AXIS_VALUE_16BITS 65535

#define CENTER_AXIS_VALUE_8BITS (MAX_AXIS_VALUE_8BITS/2+1)
#define CENTER_AXIS_VALUE_10BITS (MAX_AXIS_VALUE_10BITS/2+1)
#define CENTER_AXIS_VALUE_12BITS (MAX_AXIS_VALUE_12BITS/2+1)
#define CENTER_AXIS_VALUE_14BITS (MAX_AXIS_VALUE_14BITS/2+1)
#define CENTER_AXIS_VALUE_16BITS (MAX_AXIS_VALUE_16BITS/2+1)

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
  C_TYPE_G27_PS3,
  C_TYPE_G29_PS4,
  C_TYPE_DF_PS2,
  C_TYPE_DFP_PS2,
  C_TYPE_GTF_PS2,
  C_TYPE_G920_XONE,
  C_TYPE_SWITCH,
  // <- add new types here
  C_TYPE_NONE,
  C_TYPE_MAX = C_TYPE_NONE,
} e_controller_type;

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
  abs_axis_22,
  abs_axis_23,
  abs_axis_24,
  abs_axis_25,
  abs_axis_26,
  abs_axis_27,
  abs_axis_28,
  abs_axis_29,
  abs_axis_30,
  abs_axis_31,
  abs_axis_max = abs_axis_31,
  AXIS_MAX,
} e_controller_axis_index;

#endif /* DEFS_H_ */
