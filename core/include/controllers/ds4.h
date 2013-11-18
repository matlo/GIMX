/*
 * ds4.h
 *
 *  Created on: 14 nov. 2013
 *      Author: matlo
 */

#ifndef DS4_H_
#define DS4_H_

typedef enum
{
  ds4a_lstick_x = rel_axis_lstick_x,
  ds4a_lstick_y = rel_axis_lstick_y,
  ds4a_rstick_x = rel_axis_rstick_x,
  ds4a_rstick_y = rel_axis_rstick_y,
  ds4a_up = abs_axis_3,
  ds4a_right = abs_axis_4,
  ds4a_down = abs_axis_5,
  ds4a_left = abs_axis_6,
  ds4a_square = abs_axis_10,
  ds4a_cross = abs_axis_9,
  ds4a_circle = abs_axis_8,
  ds4a_triangle = abs_axis_7,
  ds4a_l1 = abs_axis_11,
  ds4a_r1 = abs_axis_12,
  ds4a_l2 = abs_axis_13,
  ds4a_r2 = abs_axis_14,
  ds4a_select = abs_axis_0,
  ds4a_start = abs_axis_1,
  ds4a_l3 = abs_axis_15,
  ds4a_r3 = abs_axis_16,
  ds4a_ps = abs_axis_2,
  ds4a_touchpad = abs_axis_17,
} e_ds4_axis_index;

typedef struct
{
  unsigned char report_id; //0x01
  unsigned char X;
  unsigned char Y;
  unsigned char Z;
  unsigned char Rz;
  unsigned char HatAndButtons;
  unsigned short ButtonsAndVendor;
  unsigned char Rx;
  unsigned char Ry;
  unsigned char unknown[54];
} s_report_ds4;

#endif /* DS4_H_ */
