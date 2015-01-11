/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef T300RS_H_
#define T300RS_H_

#include <control.h>

#define T300RS_SQUARE_MASK     0x10
#define T300RS_CROSS_MASK      0x20
#define T300RS_CIRCLE_MASK     0x40
#define T300RS_TRIANGLE_MASK   0x80

#define T300RS_L1_MASK         0x0001
#define T300RS_R1_MASK         0x0002
#define T300RS_L2_MASK         0x0004
#define T300RS_R2_MASK         0x0008

#define T300RS_SHARE_MASK      0x0010
#define T300RS_OPTIONS_MASK    0x0020
#define T300RS_L3_MASK         0x0040
#define T300RS_R3_MASK         0x0080

#define T300RS_PS_MASK         0x0100

typedef enum
{
  t300rsPs4a_wheel = rel_axis_lstick_x,
  t300rsPs4a_share = abs_axis_0,
  t300rsPs4a_options = abs_axis_1,
  t300rsPs4a_ps = abs_axis_2,
  t300rsPs4a_up = abs_axis_3,
  t300rsPs4a_right = abs_axis_4,
  t300rsPs4a_down = abs_axis_5,
  t300rsPs4a_left = abs_axis_6,
  t300rsPs4a_triangle = abs_axis_7,
  t300rsPs4a_circle = abs_axis_8,
  t300rsPs4a_cross = abs_axis_9,
  t300rsPs4a_square = abs_axis_10,
  t300rsPs4a_l1 = abs_axis_11,
  t300rsPs4a_r1 = abs_axis_12,
  t300rsPs4a_l2 = abs_axis_13,
  t300rsPs4a_r2 = abs_axis_14,
  t300rsPs4a_l3 = abs_axis_15,
  t300rsPs4a_r3 = abs_axis_16,
  t300rsPs4a_gasPedal = abs_axis_17,
  t300rsPs4a_brakePedal = abs_axis_18,
} e_t300rsPs4_axis_index;

/*
 *
 */
typedef struct __attribute__ ((packed))
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
  unsigned short unknown1; //0xFFFF
  unsigned char unknown2; //0x00
  unsigned short unknown3; //0xFFFF
  unsigned char unused1[10];
} s_report_t300rsPs4;

void t300rsPs4_init_report(s_report_t300rsPs4* t300rsPs4);

#endif /* T300RS_H_ */
