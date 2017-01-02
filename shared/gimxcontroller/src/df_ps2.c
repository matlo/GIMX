/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <df_ps2.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

#define DF_CROSS_MASK      0x0400
#define DF_SQUARE_MASK     0x0800
#define DF_CIRCLE_MASK     0x1000
#define DF_TRIANGLE_MASK   0x2000
#define DF_R1_MASK         0x4000
#define DF_L1_MASK         0x8000

#define DF_R2_MASK         0x01
#define DF_L2_MASK         0x02
#define DF_SELECT_MASK     0x04
#define DF_START_MASK      0x08
#define DF_R3_MASK         0x10
#define DF_L3_MASK         0x20

static s_axis axes[AXIS_MAX] =
{
  [dfPs2a_wheel]        = { .name = "wheel",          .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
  
  [dfPs2a_gasPedal]     = { .name = "gas",            .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_brakePedal]   = { .name = "brake",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_select]       = { .name = "select",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_start]        = { .name = "start",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_up]           = { .name = "up",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_right]        = { .name = "right",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_down]         = { .name = "down",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_left]         = { .name = "left",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_triangle]     = { .name = "triangle",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_circle]       = { .name = "circle",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_cross]        = { .name = "cross",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_square]       = { .name = "square",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_l1]           = { .name = "l1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_r1]           = { .name = "r1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_l2]           = { .name = "l2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_r2]           = { .name = "r2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_l3]           = { .name = "l3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfPs2a_r3]           = { .name = "r3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = dfPs2a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = dfPs2a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = dfPs2a_brakePedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = dfPs2a_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = dfPs2a_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "select",       {.axis = dfPs2a_select,     .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = dfPs2a_start,      .props = AXIS_PROP_TOGGLE}},

  {.name = "up",           {.axis = dfPs2a_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = dfPs2a_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = dfPs2a_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = dfPs2a_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = dfPs2a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = dfPs2a_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = dfPs2a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = dfPs2a_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = dfPs2a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = dfPs2a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = dfPs2a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = dfPs2a_triangle,   .props = AXIS_PROP_TOGGLE}},
};

static s_report_dfPs2 default_report =
{
  .buttonsAndWheel = CENTER_AXIS_VALUE_10BITS,
  .buttons = 0x00,
  .unknown = 0x7f,
  .hat = 0x08,
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
  report[index].length = sizeof(s_report_dfPs2);
  s_report_dfPs2* dfPs2 = &report[index].value.dfPs2;

  dfPs2->buttonsAndWheel = clamp(0, axis[dfPs2a_wheel] + CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  dfPs2->gasPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[dfPs2a_gasPedal], MAX_AXIS_VALUE_8BITS);
  dfPs2->brakePedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[dfPs2a_brakePedal], MAX_AXIS_VALUE_8BITS);

  dfPs2->buttons = 0x00;
  
  if (axis[dfPs2a_right])
  {
    if (axis[dfPs2a_down])
    {
      dfPs2->hat = 0x03;
    }
    else if (axis[dfPs2a_up])
    {
      dfPs2->hat = 0x01;
    }
    else
    {
      dfPs2->hat = 0x02;
    }
  }
  else if (axis[dfPs2a_left])
  {
    if (axis[dfPs2a_down])
    {
      dfPs2->hat = 0x05;
    }
    else if (axis[dfPs2a_up])
    {
      dfPs2->hat = 0x07;
    }
    else
    {
      dfPs2->hat = 0x06;
    }
  }
  else if (axis[dfPs2a_down])
  {
    dfPs2->hat = 0x04;
  }
  else if (axis[dfPs2a_up])
  {
    dfPs2->hat = 0x0;
  }
  else
  {
    dfPs2->hat = 0x08;
  }

  if (axis[dfPs2a_square])
  {
    dfPs2->buttonsAndWheel |= DF_SQUARE_MASK;
  }
  if (axis[dfPs2a_cross])
  {
    dfPs2->buttonsAndWheel |= DF_CROSS_MASK;
  }
  if (axis[dfPs2a_triangle])
  {
    dfPs2->buttonsAndWheel |= DF_TRIANGLE_MASK;
  }
  if (axis[dfPs2a_circle])
  {
    dfPs2->buttonsAndWheel |= DF_CIRCLE_MASK;
  }
  if (axis[dfPs2a_r1])
  {
    dfPs2->buttonsAndWheel |= DF_R1_MASK;
  }
  if (axis[dfPs2a_l1])
  {
    dfPs2->buttonsAndWheel |= DF_L1_MASK;
  }

  if (axis[dfPs2a_l3])
  {
    dfPs2->buttons |= DF_L3_MASK;
  }
  if (axis[dfPs2a_r3])
  {
    dfPs2->buttons |= DF_R3_MASK;
  }
  if (axis[dfPs2a_start])
  {
    dfPs2->buttons |= DF_START_MASK;
  }
  if (axis[dfPs2a_select])
  {
    dfPs2->buttons |= DF_SELECT_MASK;
  }
  if (axis[dfPs2a_l2])
  {
    dfPs2->buttons |= DF_L2_MASK;
  }
  if (axis[dfPs2a_r2])
  {
    dfPs2->buttons |= DF_R2_MASK;
  }

  return index;
}

static s_controller controller =
{
  .name = "DF PS2",
  .vid = 0x046d,
  .pid = 0xc294,
  .refresh_period = { .min_value = 10000, .default_value = 10000 },
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void dfPs2_init(void) __attribute__((constructor (101)));
void dfPs2_init(void)
{
  controller_register(C_TYPE_DF_PS2, &controller);
}

