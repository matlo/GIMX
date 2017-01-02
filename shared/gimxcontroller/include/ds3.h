/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DS3_H_
#define DS3_H_

#include "defs.h"

typedef enum
{
  sa_lstick_x = rel_axis_lstick_x,
  sa_lstick_y = rel_axis_lstick_y,
  sa_rstick_x = rel_axis_rstick_x,
  sa_rstick_y = rel_axis_rstick_y,
  sa_acc_x = rel_axis_4,
  sa_acc_y = rel_axis_5,
  sa_acc_z = rel_axis_6,
  sa_gyro = rel_axis_7,
  sa_select = abs_axis_0,
  sa_start = abs_axis_1,
  sa_ps = abs_axis_2,
  sa_up = abs_axis_3,
  sa_right = abs_axis_4,
  sa_down = abs_axis_5,
  sa_left = abs_axis_6,
  sa_triangle = abs_axis_7,
  sa_circle = abs_axis_8,
  sa_cross = abs_axis_9,
  sa_square = abs_axis_10,
  sa_l1 = abs_axis_11,
  sa_r1 = abs_axis_12,
  sa_l2 = abs_axis_13,
  sa_r2 = abs_axis_14,
  sa_l3 = abs_axis_15,
  sa_r3 = abs_axis_16
} e_ds3_abs_axis_index;

typedef struct GIMX_PACKED
{
  unsigned char report_id;
  unsigned char unused1;
  unsigned char buttons1;
  unsigned char buttons2;
  unsigned char buttons3;
  unsigned char unused2;
  char X;
  char Y;
  char Z;
  char Rz;
  unsigned char unknown1[4];
  unsigned char up;
  unsigned char right;
  unsigned char down;
  unsigned char left;
  unsigned char l2;
  unsigned char r2;
  unsigned char l1;
  unsigned char r1;
  unsigned char triangle;
  unsigned char circle;
  unsigned char cross;
  unsigned char square;
  unsigned char unknown2[15];//25
  unsigned char acc_x[2];
  unsigned char acc_y[2];
  unsigned char acc_z[2];
  unsigned char gyro[2];
} s_report_ds3;

#endif /* DS3_H_ */
