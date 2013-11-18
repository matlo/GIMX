/*
 * xbox.h
 *
 *  Created on: 14 nov. 2013
 *      Author: matlo
 */

#ifndef XBOX_H_
#define XBOX_H_

#include <controllers/controller.h>

typedef enum
{
  xboxa_lstick_x = rel_axis_lstick_x,
  xboxa_lstick_y = rel_axis_lstick_y,
  xboxa_rstick_x = rel_axis_rstick_x,
  xboxa_rstick_y = rel_axis_rstick_y,
  xboxa_back = abs_axis_0,
  xboxa_start = abs_axis_1,
  xboxa_up = abs_axis_3,
  xboxa_right = abs_axis_4,
  xboxa_down = abs_axis_5,
  xboxa_left = abs_axis_6,
  xboxa_Y = abs_axis_7,
  xboxa_B = abs_axis_8,
  xboxa_A = abs_axis_9,
  xboxa_X = abs_axis_10,
  xboxa_white = abs_axis_11,
  xboxa_black = abs_axis_12,
  xboxa_LT = abs_axis_13,
  xboxa_RT = abs_axis_14,
  xboxa_LS = abs_axis_15,
  xboxa_RS = abs_axis_16,
} e_xbox_axis_index;

typedef struct
{
  unsigned char type; //0x00
  unsigned char size;     //0x14
  unsigned char buttons;
  unsigned char ununsed2; //0x00
  unsigned char btnA;
  unsigned char btnB;
  unsigned char btnX;
  unsigned char btnY;
  unsigned char btnBlack;
  unsigned char btnWhite;
  unsigned char ltrigger;
  unsigned char rtrigger;
  short xaxis;
  short yaxis;
  short zaxis;
  short taxis;
} s_report_xbox;

#endif /* XBOX_H_ */
