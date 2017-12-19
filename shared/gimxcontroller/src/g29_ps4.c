/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <g29_ps4.h>
#include <report.h>
#include <string.h>
#include "../include/controller.h"

#define G29_SQUARE_MASK     0x10
#define G29_CROSS_MASK      0x20
#define G29_CIRCLE_MASK     0x40
#define G29_TRIANGLE_MASK   0x80

#define G29_L1_MASK         0x0001
#define G29_R1_MASK         0x0002
#define G29_L2_MASK         0x0004
#define G29_R2_MASK         0x0008

#define G29_SHARE_MASK      0x0010
#define G29_OPTIONS_MASK    0x0020
#define G29_L3_MASK         0x0040
#define G29_R3_MASK         0x0080

#define G29_PS_MASK         0x0100

#define G29_GEAR_SHIFTER_1_MASK  0x01
#define G29_GEAR_SHIFTER_2_MASK  0x02
#define G29_GEAR_SHIFTER_3_MASK  0x04
#define G29_GEAR_SHIFTER_4_MASK  0x08
#define G29_GEAR_SHIFTER_5_MASK  0x10
#define G29_GEAR_SHIFTER_6_MASK  0x20
#define G29_GEAR_SHIFTER_R_MASK  0x80

#define G29_ENTER_MASK      0x01
#define G29_DIAL_DOWN_MASK  0x02
#define G29_DIAL_UP_MASK    0x04
#define G29_MINUS_MASK      0x08
#define G29_PLUS_MASK       0x10

static s_axis axes[AXIS_MAX] =
{
  [g29Ps4a_wheel]       = { .name = "wheel",    .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [g29Ps4a_gasPedal]    = { .name = "gas",      .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [g29Ps4a_brakePedal]  = { .name = "brake",    .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [g29Ps4a_clutchPedal] = { .name = "clutch",   .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  
  [g29Ps4a_share]       = { .name = "share",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_options]     = { .name = "options",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_up]          = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_right]       = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_down]        = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_left]        = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_triangle]    = { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_circle]      = { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_cross]       = { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_square]      = { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_l1]          = { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_r1]          = { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_l2]          = { .name = "l2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_r2]          = { .name = "r2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_l3]          = { .name = "l3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_r3]          = { .name = "r3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_ps]          = { .name = "PS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

  [g29Ps4a_gearShifter1] = {.name = "gear shifter 1", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifter2] = {.name = "gear shifter 2", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifter3] = {.name = "gear shifter 3", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifter4] = {.name = "gear shifter 4", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifter5] = {.name = "gear shifter 5", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifter6] = {.name = "gear shifter 6", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_gearShifterR] = {.name = "gear shifter R", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

  [g29Ps4a_plus]        = { .name = "plus",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_minus]       = { .name = "minus",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_dialUp]      = { .name = "dial up",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_dialDown]    = { .name = "dial down",.max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g29Ps4a_enter]       = { .name = "enter",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = g29Ps4a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = g29Ps4a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = g29Ps4a_brakePedal, .props = AXIS_PROP_POSITIVE}},
  {.name = "clutch",        {.axis = g29Ps4a_clutchPedal, .props = AXIS_PROP_POSITIVE}},

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

  {.name = "gear shifter 1",    {.axis = g29Ps4a_gearShifter1,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 2",    {.axis = g29Ps4a_gearShifter2,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 3",    {.axis = g29Ps4a_gearShifter3,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 4",    {.axis = g29Ps4a_gearShifter4,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 5",    {.axis = g29Ps4a_gearShifter5,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 6",    {.axis = g29Ps4a_gearShifter6,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter R",    {.axis = g29Ps4a_gearShifterR,   .props = AXIS_PROP_TOGGLE}},

  {.name = "plus",      {.axis = g29Ps4a_plus,     .props = AXIS_PROP_TOGGLE}},
  {.name = "minus",     {.axis = g29Ps4a_minus,    .props = AXIS_PROP_TOGGLE}},
  {.name = "dial up",   {.axis = g29Ps4a_dialUp,   .props = AXIS_PROP_TOGGLE}},
  {.name = "dial down", {.axis = g29Ps4a_dialDown, .props = AXIS_PROP_TOGGLE}},
  {.name = "enter",     {.axis = g29Ps4a_enter,    .props = AXIS_PROP_TOGGLE}},
};

static s_report_g29Ps4 default_report =
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
  .clutchPedal = MAX_AXIS_VALUE_16BITS,
  .Buttons2 = 0x00,
  .unknown3 = MAX_AXIS_VALUE_16BITS,
  .Buttons3 = 0x00,
  .unused1 = {},
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_g29Ps4);
  s_report_g29Ps4* g29Ps4 = &report[index].value.g29Ps4;

  g29Ps4->wheel = clamp(0, axis[g29Ps4a_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  g29Ps4->gasPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[g29Ps4a_gasPedal], MAX_AXIS_VALUE_16BITS);
  g29Ps4->brakePedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[g29Ps4a_brakePedal], MAX_AXIS_VALUE_16BITS);
  g29Ps4->clutchPedal = clamp(0, MAX_AXIS_VALUE_16BITS - axis[g29Ps4a_clutchPedal], MAX_AXIS_VALUE_16BITS);

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

  g29Ps4->Buttons2 = 0x00;

  if (axis[g29Ps4a_gearShifter1])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_1_MASK;
  }
  if (axis[g29Ps4a_gearShifter2])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_2_MASK;
  }
  if (axis[g29Ps4a_gearShifter3])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_3_MASK;
  }
  if (axis[g29Ps4a_gearShifter4])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_4_MASK;
  }
  if (axis[g29Ps4a_gearShifter5])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_5_MASK;
  }
  if (axis[g29Ps4a_gearShifter6])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_6_MASK;
  }
  if (axis[g29Ps4a_gearShifterR])
  {
    g29Ps4->Buttons2 |= G29_GEAR_SHIFTER_R_MASK;
  }

  g29Ps4->Buttons3 = 0x00;

  if (axis[g29Ps4a_plus])
  {
    g29Ps4->Buttons3 |= G29_PLUS_MASK;
  }
  if (axis[g29Ps4a_minus])
  {
    g29Ps4->Buttons3 |= G29_MINUS_MASK;
  }
  if (axis[g29Ps4a_dialUp])
  {
    g29Ps4->Buttons3 |= G29_DIAL_UP_MASK;
  }
  if (axis[g29Ps4a_dialDown])
  {
    g29Ps4->Buttons3 |= G29_DIAL_DOWN_MASK;
  }
  if (axis[g29Ps4a_enter])
  {
    g29Ps4->Buttons3 |= G29_ENTER_MASK;
  }

  return index;
}

static s_controller controller =
{
  .name = "G29 PS4",
  .vid = 0x046d,
  .pid = 0xc24f,
  .refresh_period = { .min_value = 1000, .default_value = 5000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
  .activation_button = g29Ps4a_ps,
};

void g29Ps4_init(void) __attribute__((constructor));
void g29Ps4_init(void)
{
  controller_register(C_TYPE_G29_PS4, &controller);
}

