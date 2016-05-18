/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GTF_PS2_H_
#define GTF_PS2_H_

#include <defs.h>
#include <stdint.h>

typedef enum
{
  gtfPs2a_wheel = rel_axis_lstick_x,
  gtfPs2a_triangle = abs_axis_7,
  gtfPs2a_circle = abs_axis_8,
  gtfPs2a_cross = abs_axis_9,
  gtfPs2a_square = abs_axis_10,
  gtfPs2a_l1 = abs_axis_11,
  gtfPs2a_r1 = abs_axis_12,
  gtfPs2a_gasPedal = abs_axis_17,
  gtfPs2a_brakePedal = abs_axis_18,
} e_gtfPs2_axis_index;

/*
 *
 */
typedef struct GIMX_PACKED
{
  uint16_t buttonsAndWheel; // 10 LSB = wheel, 6 MSB = buttons
  uint8_t : 8;
  uint8_t pedals; // combined pedals
  uint8_t gasPedal;
  uint8_t brakePedal;
  uint8_t : 8;
} s_report_gtfPs2;

#endif /* GTF_PS2_H_ */
