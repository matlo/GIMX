/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <t300rs_ps4.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static const char *t300rsPs4_axis_name[AXIS_MAX] =
{
  [t300rsPs4a_wheel] = "wheel",
  [t300rsPs4a_gasPedal] = "gas pedal",
  [t300rsPs4a_breakPedal] = "break pedal",
  [t300rsPs4a_share] = "share",
  [t300rsPs4a_options] = "options",
  [t300rsPs4a_up] = "up",
  [t300rsPs4a_right] = "right",
  [t300rsPs4a_down] = "down",
  [t300rsPs4a_left] = "left",
  [t300rsPs4a_triangle] = "triangle",
  [t300rsPs4a_circle] = "circle",
  [t300rsPs4a_cross] = "cross",
  [t300rsPs4a_square] = "square",
  [t300rsPs4a_l1] = "l1",
  [t300rsPs4a_r1] = "r1",
  [t300rsPs4a_l2] = "l2",
  [t300rsPs4a_r2] = "r2",
  [t300rsPs4a_l3] = "l3",
  [t300rsPs4a_r3] = "r3",
  [t300rsPs4a_ps] = "PS",
};

static s_axis_name_dir axis_names[] =
{
  {.name = "wheel",        {.axis = t300rsPs4a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas pedal",    {.axis = t300rsPs4a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "break pedal",  {.axis = t300rsPs4a_breakPedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = t300rsPs4a_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = t300rsPs4a_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "share",        {.axis = t300rsPs4a_share,      .props = AXIS_PROP_TOGGLE}},
  {.name = "options",      {.axis = t300rsPs4a_options,    .props = AXIS_PROP_TOGGLE}},
  {.name = "PS",           {.axis = t300rsPs4a_ps,         .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = t300rsPs4a_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = t300rsPs4a_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = t300rsPs4a_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = t300rsPs4a_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = t300rsPs4a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = t300rsPs4a_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = t300rsPs4a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = t300rsPs4a_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = t300rsPs4a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = t300rsPs4a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = t300rsPs4a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = t300rsPs4a_triangle,   .props = AXIS_PROP_TOGGLE}},
};

static int t300rsPs4_max_unsigned_axis_value[AXIS_MAX] =
{
  [t300rsPs4a_wheel] = MAX_AXIS_VALUE_16BITS,
  [t300rsPs4a_gasPedal] = MAX_AXIS_VALUE_16BITS,
  [t300rsPs4a_breakPedal] = MAX_AXIS_VALUE_16BITS,

  [t300rsPs4a_up] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_right] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_down] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_left] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_square] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_cross] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_circle] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_triangle] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_l1] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_r1] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_l2] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_r2] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_share] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_options] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_l3] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_r3] = MAX_AXIS_VALUE_8BITS,
  [t300rsPs4a_ps] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params t300rsPs4_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 5000,
    .max_unsigned_axis_value = t300rsPs4_max_unsigned_axis_value
};

static s_report_t300rsPs4 init_report_t300rsPs4 =
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
  .breakPedal = MAX_AXIS_VALUE_16BITS,
  .unknown1 = MAX_AXIS_VALUE_16BITS,
  .unknown2 = 0x00,
  .unknown3 = MAX_AXIS_VALUE_16BITS,
  .unused1 = {},
};

void t300rsPs4_init_report(s_report_t300rsPs4* t300rsPs4)
{
  memcpy(t300rsPs4, &init_report_t300rsPs4, sizeof(s_report_t300rsPs4));
}

/*
 * Work in progress...
 * Do not assume the code in the following function is right!
 */
static unsigned int t300rsPs4_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_t300rsPs4* t300rsPs4 = &report->value.t300rsPs4;

  t300rsPs4->wheel = clamp(0, axis[t300rsPs4a_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  t300rsPs4->gasPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[t300rsPs4a_gasPedal], MAX_AXIS_VALUE_16BITS);
  t300rsPs4->breakPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[t300rsPs4a_breakPedal], MAX_AXIS_VALUE_16BITS);

  if (axis[t300rsPs4a_right])
  {
    if (axis[t300rsPs4a_down])
    {
      t300rsPs4->HatAndButtons = 0x03;
    }
    else if (axis[t300rsPs4a_up])
    {
      t300rsPs4->HatAndButtons = 0x01;
    }
    else
    {
      t300rsPs4->HatAndButtons = 0x02;
    }
  }
  else if (axis[t300rsPs4a_left])
  {
    if (axis[t300rsPs4a_down])
    {
      t300rsPs4->HatAndButtons = 0x05;
    }
    else if (axis[t300rsPs4a_up])
    {
      t300rsPs4->HatAndButtons = 0x07;
    }
    else
    {
      t300rsPs4->HatAndButtons = 0x06;
    }
  }
  else if (axis[t300rsPs4a_down])
  {
    t300rsPs4->HatAndButtons = 0x04;
  }
  else if (axis[t300rsPs4a_up])
  {
    t300rsPs4->HatAndButtons = 0x00;
  }
  else
  {
    t300rsPs4->HatAndButtons = 0x08;
  }

  if (axis[t300rsPs4a_square])
  {
    t300rsPs4->HatAndButtons |= T300RS_SQUARE_MASK;
  }
  if (axis[t300rsPs4a_cross])
  {
    t300rsPs4->HatAndButtons |= T300RS_CROSS_MASK;
  }
  if (axis[t300rsPs4a_circle])
  {
    t300rsPs4->HatAndButtons |= T300RS_CIRCLE_MASK;
  }
  if (axis[t300rsPs4a_triangle])
  {
    t300rsPs4->HatAndButtons |= T300RS_TRIANGLE_MASK;
  }

  t300rsPs4->Buttons = 0x0000;

  if (axis[t300rsPs4a_l1])
  {
    t300rsPs4->Buttons |= T300RS_L1_MASK;
  }
  if (axis[t300rsPs4a_r1])
  {
    t300rsPs4->Buttons |= T300RS_R1_MASK;
  }
  if (axis[t300rsPs4a_l2])
  {
    t300rsPs4->Buttons |= T300RS_L2_MASK;
  }
  if (axis[t300rsPs4a_r2])
  {
    t300rsPs4->Buttons |= T300RS_R2_MASK;
  }
  if (axis[t300rsPs4a_share])
  {
    t300rsPs4->Buttons |= T300RS_SHARE_MASK;
  }
  if (axis[t300rsPs4a_options])
  {
    t300rsPs4->Buttons |= T300RS_OPTIONS_MASK;
  }
  if (axis[t300rsPs4a_l3])
  {
    t300rsPs4->Buttons |= T300RS_L3_MASK;
  }
  if (axis[t300rsPs4a_r3])
  {
    t300rsPs4->Buttons |= T300RS_R3_MASK;
  }
  if (axis[t300rsPs4a_ps])
  {
    t300rsPs4->Buttons |= T300RS_PS_MASK;
  }

  return sizeof(*t300rsPs4);
}

void t300rsPs4_init(void) __attribute__((constructor (101)));
void t300rsPs4_init(void)
{
  controller_register_axis_names(C_TYPE_T300RS_PS4, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_T300RS_PS4, &t300rsPs4_params);

  control_register_names(C_TYPE_T300RS_PS4, t300rsPs4_axis_name);

  report_register_builder(C_TYPE_T300RS_PS4, t300rsPs4_report_build);
}
