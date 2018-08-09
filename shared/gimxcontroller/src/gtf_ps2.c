/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gtf_ps2.h>
#include <report.h>
#include <string.h>
#include "../include/controller.h"

#define GTF_SQUARE_MASK     0x0800
#define GTF_CROSS_MASK      0x0400
#define GTF_TRIANGLE_MASK   0x2000
#define GTF_CIRCLE_MASK     0x1000
#define GTF_R1_MASK         0x4000
#define GTF_L1_MASK         0x8000


static s_axis axes[AXIS_MAX] =
{
  [gtfPs2a_wheel]        = { .name = "wheel",    .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
  
  [gtfPs2a_gasPedal]     = { .name = "gas",      .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_brakePedal]   = { .name = "brake",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_triangle]     = { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_circle]       = { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_cross]        = { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_square]       = { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_l1]           = { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [gtfPs2a_r1]           = { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = gtfPs2a_wheel,      .props = AXIS_PROP_CENTERED}},
  {.name = "wheel left",   {.axis = gtfPs2a_wheel,      .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "wheel right",  {.axis = gtfPs2a_wheel,      .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "gas",          {.axis = gtfPs2a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = gtfPs2a_brakePedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r1",           {.axis = gtfPs2a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = gtfPs2a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = gtfPs2a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = gtfPs2a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = gtfPs2a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = gtfPs2a_triangle,   .props = AXIS_PROP_TOGGLE}},
};

static s_report_gtfPs2 default_report =
{
  .buttonsAndWheel = CENTER_AXIS_VALUE_10BITS,
  .pedals = CENTER_AXIS_VALUE_8BITS,
  .gasPedal = MAX_AXIS_VALUE_8BITS,
  .brakePedal = MAX_AXIS_VALUE_8BITS,
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_gtfPs2);
  s_report_gtfPs2* gtfPs2 = &report[index].value.gtfPs2;

  gtfPs2->buttonsAndWheel = clamp(0, axis[gtfPs2a_wheel] + CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  gtfPs2->gasPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[gtfPs2a_gasPedal], MAX_AXIS_VALUE_8BITS);
  gtfPs2->brakePedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[gtfPs2a_brakePedal], MAX_AXIS_VALUE_8BITS);
  
  gtfPs2->pedals = clamp(0, (axis[gtfPs2a_gasPedal] - axis[gtfPs2a_brakePedal]) / 2 + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);

  if (axis[gtfPs2a_square])
  {
    gtfPs2->buttonsAndWheel |= GTF_SQUARE_MASK;
  }
  if (axis[gtfPs2a_cross])
  {
    gtfPs2->buttonsAndWheel |= GTF_CROSS_MASK;
  }
  if (axis[gtfPs2a_triangle])
  {
    gtfPs2->buttonsAndWheel |= GTF_TRIANGLE_MASK;
  }
  if (axis[gtfPs2a_circle])
  {
    gtfPs2->buttonsAndWheel |= GTF_CIRCLE_MASK;
  }
  if (axis[gtfPs2a_r1])
  {
    gtfPs2->buttonsAndWheel |= GTF_R1_MASK;
  }
  if (axis[gtfPs2a_l1])
  {
    gtfPs2->buttonsAndWheel |= GTF_L1_MASK;
  }

  return index;
}

static s_controller controller =
{
  .name = "GTF PS2",
  .vid = 0x046d,
  .pid = 0xc293,
  .refresh_period = { .min_value = 10000, .default_value = 10000 },
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void gtfPs2_init(void) __attribute__((constructor (101)));
void gtfPs2_init(void)
{
  controller_register(C_TYPE_GTF_PS2, &controller);
}

