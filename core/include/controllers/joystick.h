/*
 * joystick.h
 *
 *  Created on: 13 nov. 2013
 *      Author: matlo
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "controllers/controller.h"

typedef enum
{
  jsa_lstick_x = sa_lstick_x,
  jsa_lstick_y = sa_lstick_y,
  jsa_rstick_x = sa_rstick_x,
  jsa_rstick_y = sa_rstick_y,
  jsa_up = sa_up,
  jsa_right = sa_right,
  jsa_down = sa_down,
  jsa_left = sa_left,
  jsa_B0 = sa_square,
  jsa_B1 = sa_cross,
  jsa_B2 = sa_circle,
  jsa_B3 = sa_triangle,
  jsa_B4 = sa_l1,
  jsa_B5 = sa_r1,
  jsa_B6 = sa_l2,
  jsa_B7 = sa_r2,
  jsa_B8 = sa_select,
  jsa_B9 = sa_start,
  jsa_B10 = sa_l3,
  jsa_B11 = sa_r3,
  jsa_B12 = sa_ps,
} e_joystick_axis_index;

typedef struct
{
  short X;
  short Y;
  short Z;
  short Rz;
  unsigned short Hat;
  unsigned short Bt;
} s_report_joystick;

#endif /* JOYSTICK_H_ */
