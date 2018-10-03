/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef G920_XONE_H_
#define G920_XONE_H_

#include <stdint.h>
#include "defs.h"

typedef enum
{
  g920Xonea_wheel = rel_axis_lstick_x,

  g920Xonea_view = abs_axis_0,
  g920Xonea_menu = abs_axis_1,
  g920Xonea_guide = abs_axis_2,
  g920Xonea_up = abs_axis_3,
  g920Xonea_right = abs_axis_4,
  g920Xonea_down = abs_axis_5,
  g920Xonea_left = abs_axis_6,
  g920Xonea_Y = abs_axis_7,
  g920Xonea_B = abs_axis_8,
  g920Xonea_A = abs_axis_9,
  g920Xonea_X = abs_axis_10,
  g920Xonea_LB = abs_axis_11,
  g920Xonea_RB = abs_axis_12,
  g920Xonea_LT = abs_axis_13,
  g920Xonea_RT = abs_axis_14,

  g920Xonea_gasPedal = abs_axis_17,
  g920Xonea_brakePedal = abs_axis_18,
  g920Xonea_clutchPedal = abs_axis_19,

  g920Xonea_gearShifter1 = abs_axis_20,
  g920Xonea_gearShifter2 = abs_axis_21,
  g920Xonea_gearShifter3 = abs_axis_22,
  g920Xonea_gearShifter4 = abs_axis_23,
  g920Xonea_gearShifter5 = abs_axis_24,
  g920Xonea_gearShifter6 = abs_axis_25,
  g920Xonea_gearShifterR = abs_axis_26,

} e_g920Xone_axis_index;

/*
 *
 */
typedef struct GIMX_PACKED {
    union {
        uint8_t type;
        struct GIMX_PACKED {
            uint8_t type; // 0x20
            uint8_t unknown1; // 0x00
            uint8_t counter; // 0x01 to 0xff
            uint8_t size; // 0x11
            uint8_t buttons1;
            uint8_t buttons2;
            uint16_t wheel;
            uint8_t gas;
            uint8_t unknown2;
            uint8_t brake;
            uint8_t unknown3;
            uint8_t clutch;
            uint8_t unknown4;
            uint8_t unknown5;
            uint8_t unknown6; // 0xe6, 0x06 when enabling/disabling Xbox UI
            uint8_t shifter;
            uint16_t range;
            uint8_t unknown7; // 0xff
            uint8_t unknown8; // 0xf8, 0x00 when enabling/disabling Xbox UI
        } input;
        struct GIMX_PACKED {
            uint8_t type; // 0x07
            uint8_t unknown1; // 0x20
            uint8_t counter;
            uint8_t size; // 0x02
            uint8_t button;
            uint8_t unknown2; // 0x5b
        } guide;
    };
} s_report_g920Xone;

#endif /* G920_XONE_H_ */
