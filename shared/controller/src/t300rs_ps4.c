/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <t300rs_ps4.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static s_axis axes[AXIS_MAX] =
{
  [t300rsPs4a_wheel] =      { .name = "wheel",    .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [t300rsPs4a_gasPedal] =   { .name = "gas",      .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [t300rsPs4a_brakePedal] = { .name = "brake",    .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  
  [t300rsPs4a_share] =      { .name = "share",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_options] =    { .name = "options",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_up] =         { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_right] =      { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_down] =       { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_left] =       { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_triangle] =   { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_circle] =     { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_cross] =      { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_square] =     { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_l1] =         { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_r1] =         { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_l2] =         { .name = "l2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_r2] =         { .name = "r2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_l3] =         { .name = "l3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_r3] =         { .name = "r3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [t300rsPs4a_ps] =         { .name = "PS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = t300rsPs4a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = t300rsPs4a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = t300rsPs4a_brakePedal, .props = AXIS_PROP_POSITIVE}},

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

static s_report_t300rsPs4 default_report =
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

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_t300rsPs4);
  s_report_t300rsPs4* t300rsPs4 = &report[index].value.t300rsPs4;

  t300rsPs4->wheel = clamp(0, axis[t300rsPs4a_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  t300rsPs4->gasPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[t300rsPs4a_gasPedal], MAX_AXIS_VALUE_16BITS);
  t300rsPs4->brakePedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[t300rsPs4a_brakePedal], MAX_AXIS_VALUE_16BITS);

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

  return index;
}

static s_controller controller =
{
  .name = "T300RS PS4",
  .vid = 0x044F,
  .pid = 0xB66D,
  .refresh_period = { .min_value = 1000, .default_value = 5000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void t300rsPs4_init(void) __attribute__((constructor));
void t300rsPs4_init(void)
{
  controller_register(C_TYPE_T300RS_PS4, &controller);
}

