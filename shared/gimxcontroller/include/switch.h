/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#include "defs.h"

// Battery levels
#define BATTERY_FULL        0x08
#define BATTERY_MEDIUM      0x06
#define BATTERY_LOW         0x04
#define BATTERY_CRITICAL    0x02
#define BATTERY_EMPTY       0x00
#define BATTERY_CHARGING    0x01 // Can be OR'ed

#define SWITCH_Y  0x01
#define SWITCH_X  0x02
#define SWITCH_B  0x04
#define SWITCH_A  0x08
#define SWITCH_R 0x40
#define SWITCH_ZR  0x80
#define SWITCH_MINUS  0x100
#define SWITCH_PLUS  0x200
#define SWITCH_RCLICK  0x400
#define SWITCH_LCLICK  0x800
#define SWITCH_HOME  0x1000
#define SWITCH_CAPTURE  0x2000
#define SWITCH_CHARGING_GRIP 0x8000
#define SWITCH_DPADDOWN 0x10000
#define SWITCH_DPADUP   0x20000
#define SWITCH_DPADRIGHT  0x40000
#define SWITCH_DPADLEFT   0x80000
#define SWITCH_L  0x400000
#define SWITCH_ZL  0x800000

typedef enum
{
  switcha_lstick_x = rel_axis_lstick_x,
  switcha_lstick_y = rel_axis_lstick_y,
  switcha_rstick_x = rel_axis_rstick_x,
  switcha_rstick_y = rel_axis_rstick_y,
  switcha_minus = abs_axis_0,
  switcha_plus = abs_axis_1,
  switcha_home  = abs_axis_2,
  switcha_up = abs_axis_3,
  switcha_right = abs_axis_4,
  switcha_down = abs_axis_5,
  switcha_left = abs_axis_6,
  switcha_x = abs_axis_7,
  switcha_a = abs_axis_8,
  switcha_b = abs_axis_9,
  switcha_y = abs_axis_10,
  switcha_l = abs_axis_11,
  switcha_r = abs_axis_12,
  switcha_zl = abs_axis_13,
  switcha_zr = abs_axis_14,
  switcha_lclick = abs_axis_15,
  switcha_rclick = abs_axis_16,
  switcha_capture = abs_axis_17,
} e_switch_axis_index;

typedef struct GIMX_PACKED
{
  unsigned long connection_info : 4;
  unsigned long  battery_level : 4;
  unsigned long buttons: 24;
  unsigned char analog[6];
  unsigned char vibrator_input_report;
  unsigned short imu[3 * 2 * 3]; // each axis is uint16_t, 3 axis per sensor, 2 sensors (accel and gyro), 3 reports
} s_report_switch;

typedef struct
{
  // TODO MLA
} s_gc_state_switch;

#endif /* SWITCH_H_ */
