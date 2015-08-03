/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <g29_ps4.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static const char *g29Ps4_axis_name[AXIS_MAX] =
{
  [g29Ps4a_wheel] = "wheel",
  [g29Ps4a_gasPedal] = "gas",
  [g29Ps4a_brakePedal] = "brake",
  [g29Ps4a_share] = "share",
  [g29Ps4a_options] = "options",
  [g29Ps4a_up] = "up",
  [g29Ps4a_right] = "right",
  [g29Ps4a_down] = "down",
  [g29Ps4a_left] = "left",
  [g29Ps4a_triangle] = "triangle",
  [g29Ps4a_circle] = "circle",
  [g29Ps4a_cross] = "cross",
  [g29Ps4a_square] = "square",
  [g29Ps4a_l1] = "l1",
  [g29Ps4a_r1] = "r1",
  [g29Ps4a_l2] = "l2",
  [g29Ps4a_r2] = "r2",
  [g29Ps4a_l3] = "l3",
  [g29Ps4a_r3] = "r3",
  [g29Ps4a_ps] = "PS",
};

static s_axis_name_dir axis_names[] =
{
  {.name = "wheel",        {.axis = g29Ps4a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = g29Ps4a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = g29Ps4a_brakePedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = g29Ps4a_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = g29Ps4a_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "share",        {.axis = g29Ps4a_share,      .props = AXIS_PROP_TOGGLE}},
  {.name = "options",      {.axis = g29Ps4a_options,    .props = AXIS_PROP_TOGGLE}},
  {.name = "PS",           {.axis = g29Ps4a_ps,         .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = g29Ps4a_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = g29Ps4a_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = g29Ps4a_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = g29Ps4a_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = g29Ps4a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = g29Ps4a_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = g29Ps4a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = g29Ps4a_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = g29Ps4a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = g29Ps4a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = g29Ps4a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = g29Ps4a_triangle,   .props = AXIS_PROP_TOGGLE}},
};

static int g29Ps4_max_unsigned_axis_value[AXIS_MAX] =
{
  [g29Ps4a_wheel] = MAX_AXIS_VALUE_16BITS,
  [g29Ps4a_gasPedal] = MAX_AXIS_VALUE_16BITS,
  [g29Ps4a_brakePedal] = MAX_AXIS_VALUE_16BITS,

  [g29Ps4a_up] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_right] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_down] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_left] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_square] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_cross] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_circle] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_triangle] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_l1] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_r1] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_l2] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_r2] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_share] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_options] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_l3] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_r3] = MAX_AXIS_VALUE_8BITS,
  [g29Ps4a_ps] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params g29Ps4_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 5000,
    .max_unsigned_axis_value = g29Ps4_max_unsigned_axis_value
};

static s_report_g29Ps4 init_report_g29Ps4 =
{
  .report_id = 0x01,
  .X = 0x80,
  .Y = 0x80,
  .Z = 0x80,
  .Rz = 0x80,
  .HatAndButtons = 0x08,
  .Buttons = 0x00,
  .unused0 = {},
  .wheel = CENTER_AXIS_VALUE_16BITS,
  .gasPedal = MAX_AXIS_VALUE_16BITS,
  .brakePedal = MAX_AXIS_VALUE_16BITS,
  .unknown1 = MAX_AXIS_VALUE_16BITS,
  .unknown2 = 0x00,
  .unknown3 = MAX_AXIS_VALUE_16BITS,
  .unused1 = {},
};

void g29Ps4_init_report(s_report_g29Ps4* g29Ps4)
{
  memcpy(g29Ps4, &init_report_g29Ps4, sizeof(s_report_g29Ps4));
}

/*
 * Work in progress...
 * Do not assume the code in the following function is right!
 */
static unsigned int g29Ps4_report_build(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_g29Ps4);
  s_report_g29Ps4* g29Ps4 = &report[index].value.g29Ps4;

  g29Ps4->wheel = clamp(0, axis[g29Ps4a_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  g29Ps4->gasPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[g29Ps4a_gasPedal], MAX_AXIS_VALUE_16BITS);
  g29Ps4->brakePedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[g29Ps4a_brakePedal], MAX_AXIS_VALUE_16BITS);

  if (axis[g29Ps4a_right])
  {
    if (axis[g29Ps4a_down])
    {
      g29Ps4->HatAndButtons = 0x03;
    }
    else if (axis[g29Ps4a_up])
    {
      g29Ps4->HatAndButtons = 0x01;
    }
    else
    {
      g29Ps4->HatAndButtons = 0x02;
    }
  }
  else if (axis[g29Ps4a_left])
  {
    if (axis[g29Ps4a_down])
    {
      g29Ps4->HatAndButtons = 0x05;
    }
    else if (axis[g29Ps4a_up])
    {
      g29Ps4->HatAndButtons = 0x07;
    }
    else
    {
      g29Ps4->HatAndButtons = 0x06;
    }
  }
  else if (axis[g29Ps4a_down])
  {
    g29Ps4->HatAndButtons = 0x04;
  }
  else if (axis[g29Ps4a_up])
  {
    g29Ps4->HatAndButtons = 0x00;
  }
  else
  {
    g29Ps4->HatAndButtons = 0x08;
  }

  if (axis[g29Ps4a_square])
  {
    g29Ps4->HatAndButtons |= G29_SQUARE_MASK;
  }
  if (axis[g29Ps4a_cross])
  {
    g29Ps4->HatAndButtons |= G29_CROSS_MASK;
  }
  if (axis[g29Ps4a_circle])
  {
    g29Ps4->HatAndButtons |= G29_CIRCLE_MASK;
  }
  if (axis[g29Ps4a_triangle])
  {
    g29Ps4->HatAndButtons |= G29_TRIANGLE_MASK;
  }

  g29Ps4->Buttons = 0x0000;

  if (axis[g29Ps4a_l1])
  {
    g29Ps4->Buttons |= G29_L1_MASK;
  }
  if (axis[g29Ps4a_r1])
  {
    g29Ps4->Buttons |= G29_R1_MASK;
  }
  if (axis[g29Ps4a_l2])
  {
    g29Ps4->Buttons |= G29_L2_MASK;
  }
  if (axis[g29Ps4a_r2])
  {
    g29Ps4->Buttons |= G29_R2_MASK;
  }
  if (axis[g29Ps4a_share])
  {
    g29Ps4->Buttons |= G29_SHARE_MASK;
  }
  if (axis[g29Ps4a_options])
  {
    g29Ps4->Buttons |= G29_OPTIONS_MASK;
  }
  if (axis[g29Ps4a_l3])
  {
    g29Ps4->Buttons |= G29_L3_MASK;
  }
  if (axis[g29Ps4a_r3])
  {
    g29Ps4->Buttons |= G29_R3_MASK;
  }
  if (axis[g29Ps4a_ps])
  {
    g29Ps4->Buttons |= G29_PS_MASK;
  }

  return index;
}

void g29Ps4_init(void) __attribute__((constructor (101)));
void g29Ps4_init(void)
{
  controller_register_axis_names(C_TYPE_G29_PS4, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_G29_PS4, &g29Ps4_params);

  control_register_names(C_TYPE_G29_PS4, g29Ps4_axis_name);

  report_register_builder(C_TYPE_G29_PS4, g29Ps4_report_build);
}
