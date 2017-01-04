/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef G29_PS4_H_
#define G29_PS4_H_

#include "defs.h"

typedef enum
{
  g29Ps4a_wheel = rel_axis_lstick_x,
  g29Ps4a_share = abs_axis_0,
  g29Ps4a_options = abs_axis_1,
  g29Ps4a_ps = abs_axis_2,
  g29Ps4a_up = abs_axis_3,
  g29Ps4a_right = abs_axis_4,
  g29Ps4a_down = abs_axis_5,
  g29Ps4a_left = abs_axis_6,
  g29Ps4a_triangle = abs_axis_7,
  g29Ps4a_circle = abs_axis_8,
  g29Ps4a_cross = abs_axis_9,
  g29Ps4a_square = abs_axis_10,
  g29Ps4a_l1 = abs_axis_11,
  g29Ps4a_r1 = abs_axis_12,
  g29Ps4a_l2 = abs_axis_13,
  g29Ps4a_r2 = abs_axis_14,
  g29Ps4a_l3 = abs_axis_15,
  g29Ps4a_r3 = abs_axis_16,
  g29Ps4a_gasPedal = abs_axis_17,
  g29Ps4a_brakePedal = abs_axis_18,
  g29Ps4a_clutchPedal = abs_axis_19,
  g29Ps4a_gearShifter1 = abs_axis_20,
  g29Ps4a_gearShifter2 = abs_axis_21,
  g29Ps4a_gearShifter3 = abs_axis_22,
  g29Ps4a_gearShifter4 = abs_axis_23,
  g29Ps4a_gearShifter5 = abs_axis_24,
  g29Ps4a_gearShifter6 = abs_axis_25,
  g29Ps4a_gearShifterR = abs_axis_26,
} e_g29Ps4_axis_index;

/*
 *
 */
typedef struct GIMX_PACKED
{
  unsigned char report_id; //0x01
  unsigned char X; //0x80
  unsigned char Y; //0x80
  unsigned char Z; //0x80
  unsigned char Rz; //0x80
  unsigned char HatAndButtons;
  unsigned short Buttons;
  unsigned char unused0[35];
  unsigned short wheel; //signed, center = 0x7FFF
  unsigned short gasPedal; //unsigned, released = 0xFFFF
  unsigned short brakePedal; //unsigned, released = 0xFFFF
  unsigned short clutchPedal; //unsigned, released = 0xFFFF
  unsigned char Buttons2;
  unsigned short unknown3; //0xFFFF
  unsigned char unused1[10];
} s_report_g29Ps4;

typedef struct
{
  // TODO MLA
} s_gc_state_g29Ps4;

#endif /* G29_PS4_H_ */
