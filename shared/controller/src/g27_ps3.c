/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <g27_ps3.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

#define G27_CROSS_MASK      0x10
#define G27_SQUARE_MASK     0x20
#define G27_CIRCLE_MASK     0x40
#define G27_TRIANGLE_MASK   0x80

#define G27_R1_MASK         0x0001
#define G27_L1_MASK         0x0002
#define G27_R2_MASK         0x0004
#define G27_L2_MASK         0x0008

#define G27_SELECT_MASK     0x0010
#define G27_START_MASK      0x0020
#define G27_R3_MASK         0x0040
#define G27_L3_MASK         0x0080

#define G27_GEAR_1_MASK     0x0100
#define G27_GEAR_2_MASK     0x0200
#define G27_GEAR_3_MASK     0x0400
#define G27_GEAR_4_MASK     0x0800
#define G27_GEAR_5_MASK     0x1000
#define G27_GEAR_6_MASK     0x2000

#define G27_R4_MASK         0x4000
#define G27_R5_MASK         0x8000

#define G27_L4_MASK         0x0001
#define G27_L5_MASK         0x0002

static const char *g27Ps3_axis_name[AXIS_MAX] =
{
  [g27Ps3a_wheel] = "wheel",
  [g27Ps3a_gasPedal] = "gas",
  [g27Ps3a_brakePedal] = "brake",
  [g27Ps3a_clutchPedal] = "clutch",
  [g27Ps3a_select] = "select",
  [g27Ps3a_start] = "start",
  [g27Ps3a_up] = "up",
  [g27Ps3a_right] = "right",
  [g27Ps3a_down] = "down",
  [g27Ps3a_left] = "left",
  [g27Ps3a_triangle] = "triangle",
  [g27Ps3a_circle] = "circle",
  [g27Ps3a_cross] = "cross",
  [g27Ps3a_square] = "square",
  [g27Ps3a_l1] = "l1",
  [g27Ps3a_r1] = "r1",
  [g27Ps3a_l2] = "l2",
  [g27Ps3a_r2] = "r2",
  [g27Ps3a_l3] = "l3",
  [g27Ps3a_r3] = "r3",
  [g27Ps3a_l4] = "l4",
  [g27Ps3a_r4] = "r4",
  [g27Ps3a_l5] = "l5",
  [g27Ps3a_r5] = "r5",
};

static s_axis_name_dir axis_names[] =
{
  {.name = "wheel",        {.axis = g27Ps3a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = g27Ps3a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = g27Ps3a_brakePedal, .props = AXIS_PROP_POSITIVE}},
  {.name = "clutch",       {.axis = g27Ps3a_clutchPedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = g27Ps3a_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = g27Ps3a_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "select",       {.axis = g27Ps3a_select,     .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = g27Ps3a_start,      .props = AXIS_PROP_TOGGLE}},

  {.name = "up",           {.axis = g27Ps3a_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = g27Ps3a_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = g27Ps3a_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = g27Ps3a_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = g27Ps3a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = g27Ps3a_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = g27Ps3a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = g27Ps3a_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = g27Ps3a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = g27Ps3a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = g27Ps3a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = g27Ps3a_triangle,   .props = AXIS_PROP_TOGGLE}},

  {.name = "l4",           {.axis = g27Ps3a_l4,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l5",           {.axis = g27Ps3a_l5,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r4",           {.axis = g27Ps3a_r4,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r5",           {.axis = g27Ps3a_r5,         .props = AXIS_PROP_TOGGLE}},
};

static int g27Ps3_max_unsigned_axis_value[AXIS_MAX] =
{
  [g27Ps3a_wheel] = MAX_AXIS_VALUE_14BITS,

  [g27Ps3a_gasPedal] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_brakePedal] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_clutchPedal] = MAX_AXIS_VALUE_8BITS,

  [g27Ps3a_up] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_right] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_down] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_left] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_square] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_cross] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_circle] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_triangle] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_l1] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_r1] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_l2] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_r2] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_select] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_start] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_l3] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_r3] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_ps] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_l4] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_r4] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_l5] = MAX_AXIS_VALUE_8BITS,
  [g27Ps3a_r5] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params g27Ps3_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 10000,
    .max_unsigned_axis_value = g27Ps3_max_unsigned_axis_value
};

static s_report_g27Ps3 init_report_g27Ps3 =
{
  .hatAndButtons = 0x08,
  .buttons = 0x0000,
  .buttonsAndWheel = (CENTER_AXIS_VALUE_14BITS << 2),
  .gasPedal = MAX_AXIS_VALUE_8BITS,
  .brakePedal = MAX_AXIS_VALUE_8BITS,
  .clutchPedal = MAX_AXIS_VALUE_8BITS,
  .unknown = { 0x7B, 0x8A, 0x18 },
};

void g27Ps3_init_report(s_report_g27Ps3* g27Ps3)
{
  memcpy(g27Ps3, &init_report_g27Ps3, sizeof(s_report_g27Ps3));
}

/*
 * Work in progress...
 * Do not assume the code in the following function is right!
 */
static unsigned int g27Ps3_report_build(int axis[AXIS_MAX], s_report_packet* report)
{
  s_report_g27Ps3* g27Ps3 = &report->value.g27Ps3;

  g27Ps3->buttonsAndWheel = clamp(0, axis[g27Ps3a_wheel] + CENTER_AXIS_VALUE_14BITS, MAX_AXIS_VALUE_14BITS) << 2;

  g27Ps3->gasPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g27Ps3a_gasPedal], MAX_AXIS_VALUE_8BITS);
  g27Ps3->brakePedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g27Ps3a_brakePedal], MAX_AXIS_VALUE_8BITS);
  g27Ps3->clutchPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[g27Ps3a_clutchPedal], MAX_AXIS_VALUE_8BITS);

  if (axis[g27Ps3a_right])
  {
    if (axis[g27Ps3a_down])
    {
      g27Ps3->hatAndButtons = 0x03;
    }
    else if (axis[g27Ps3a_up])
    {
      g27Ps3->hatAndButtons = 0x01;
    }
    else
    {
      g27Ps3->hatAndButtons = 0x02;
    }
  }
  else if (axis[g27Ps3a_left])
  {
    if (axis[g27Ps3a_down])
    {
      g27Ps3->hatAndButtons = 0x05;
    }
    else if (axis[g27Ps3a_up])
    {
      g27Ps3->hatAndButtons = 0x07;
    }
    else
    {
      g27Ps3->hatAndButtons = 0x06;
    }
  }
  else if (axis[g27Ps3a_down])
  {
    g27Ps3->hatAndButtons = 0x04;
  }
  else if (axis[g27Ps3a_up])
  {
    g27Ps3->hatAndButtons = 0x00;
  }
  else
  {
    g27Ps3->hatAndButtons = 0x08;
  }

  if (axis[g27Ps3a_square])
  {
    g27Ps3->hatAndButtons |= G27_SQUARE_MASK;
  }
  if (axis[g27Ps3a_cross])
  {
    g27Ps3->hatAndButtons |= G27_CROSS_MASK;
  }
  if (axis[g27Ps3a_circle])
  {
    g27Ps3->hatAndButtons |= G27_CIRCLE_MASK;
  }
  if (axis[g27Ps3a_triangle])
  {
    g27Ps3->hatAndButtons |= G27_TRIANGLE_MASK;
  }

  g27Ps3->buttons = 0x0000;

  if (axis[g27Ps3a_l1])
  {
    g27Ps3->buttons |= G27_L1_MASK;
  }
  if (axis[g27Ps3a_r1])
  {
    g27Ps3->buttons |= G27_R1_MASK;
  }
  if (axis[g27Ps3a_l2])
  {
    g27Ps3->buttons |= G27_L2_MASK;
  }
  if (axis[g27Ps3a_r2])
  {
    g27Ps3->buttons |= G27_R2_MASK;
  }
  if (axis[g27Ps3a_select])
  {
    g27Ps3->buttons |= G27_SELECT_MASK;
  }
  if (axis[g27Ps3a_start])
  {
    g27Ps3->buttons |= G27_START_MASK;
  }
  if (axis[g27Ps3a_l3])
  {
    g27Ps3->buttons |= G27_L3_MASK;
  }
  if (axis[g27Ps3a_r3])
  {
    g27Ps3->buttons |= G27_R3_MASK;
  }
  if (axis[g27Ps3a_r4])
  {
    g27Ps3->buttons |= G27_R4_MASK;
  }
  if (axis[g27Ps3a_r5])
  {
    g27Ps3->buttons |= G27_R5_MASK;
  }
  if (axis[g27Ps3a_l4])
  {
    g27Ps3->buttonsAndWheel |= G27_L4_MASK;
  }
  if (axis[g27Ps3a_l5])
  {
    g27Ps3->buttonsAndWheel |= G27_L5_MASK;
  }

  return sizeof(*g27Ps3);
}

void g27Ps3_init(void) __attribute__((constructor (101)));
void g27Ps3_init(void)
{
  controller_register_axis_names(C_TYPE_G27_PS3, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_G27_PS3, &g27Ps3_params);

  control_register_names(C_TYPE_G27_PS3, g27Ps3_axis_name);

  report_register_builder(C_TYPE_G27_PS3, g27Ps3_report_build);
}
