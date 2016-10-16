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

#define G27_SHIFTER_1_MASK     0x0100
#define G27_SHIFTER_2_MASK     0x0200
#define G27_SHIFTER_3_MASK     0x0400
#define G27_SHIFTER_4_MASK     0x0800
#define G27_SHIFTER_5_MASK     0x1000
#define G27_SHIFTER_6_MASK     0x2000

#define G27_SHIFTER_CENTER 0x80
#define G27_SHIFTER_LEFT   0x55
#define G27_SHIFTER_RIGHT  0xab
#define G27_SHIFTER_DOWN   0x00
#define G27_SHIFTER_UP     0xff

#define G27_SHIFTER_DEFAULT_BITS    0x9c
#define G27_SHIFTER_REVERSE_ENGAGED 0x40
#define G27_SHIFTER_STICK_DOWN      0x01

#define G27_R4_MASK         0x4000
#define G27_R5_MASK         0x8000

#define G27_L4_MASK         0x0001
#define G27_L5_MASK         0x0002

static s_axis axes[AXIS_MAX] =
{
  [g27Ps3a_wheel]        = { .name = "wheel",          .max_unsigned_value = MAX_AXIS_VALUE_14BITS },
  
  [g27Ps3a_gasPedal]     = { .name = "gas",            .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_brakePedal]   = { .name = "brake",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_clutchPedal]  = { .name = "clutch",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_select]       = { .name = "select",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_start]        = { .name = "start",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_up]           = { .name = "up",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_right]        = { .name = "right",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_down]         = { .name = "down",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_left]         = { .name = "left",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_triangle]     = { .name = "triangle",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_circle]       = { .name = "circle",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_cross]        = { .name = "cross",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_square]       = { .name = "square",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_l1]           = { .name = "l1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_r1]           = { .name = "r1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_l2]           = { .name = "l2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_r2]           = { .name = "r2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_l3]           = { .name = "l3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_r3]           = { .name = "r3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_l4]           = { .name = "l4",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_r4]           = { .name = "r4",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_l5]           = { .name = "l5",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_r5]           = { .name = "r5",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter1] = { .name = "gear shifter 1", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter2] = { .name = "gear shifter 2", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter3] = { .name = "gear shifter 3", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter4] = { .name = "gear shifter 4", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter5] = { .name = "gear shifter 5", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifter6] = { .name = "gear shifter 6", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [g27Ps3a_gearShifterR] = { .name = "gear shifter R", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
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

  {.name = "gear shifter 1",    {.axis = g27Ps3a_gearShifter1,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 2",    {.axis = g27Ps3a_gearShifter2,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 3",    {.axis = g27Ps3a_gearShifter3,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 4",    {.axis = g27Ps3a_gearShifter4,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 5",    {.axis = g27Ps3a_gearShifter5,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter 6",    {.axis = g27Ps3a_gearShifter6,   .props = AXIS_PROP_TOGGLE}},
  {.name = "gear shifter R",    {.axis = g27Ps3a_gearShifterR,   .props = AXIS_PROP_TOGGLE}},
};

static s_report_g27Ps3 default_report =
{
  .hatAndButtons = 0x08,
  .buttons = 0x0000,
  .buttonsAndWheel = (CENTER_AXIS_VALUE_14BITS << 2),
  .gasPedal = MAX_AXIS_VALUE_8BITS,
  .brakePedal = MAX_AXIS_VALUE_8BITS,
  .clutchPedal = MAX_AXIS_VALUE_8BITS,
  .shifter = { G27_SHIFTER_CENTER, G27_SHIFTER_CENTER, G27_SHIFTER_DEFAULT_BITS },
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_g27Ps3);
  s_report_g27Ps3* g27Ps3 = &report[index].value.g27Ps3;

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
  g27Ps3->shifter.x = G27_SHIFTER_CENTER;
  g27Ps3->shifter.y = G27_SHIFTER_CENTER;
  g27Ps3->shifter.b = G27_SHIFTER_DEFAULT_BITS;
  if (axis[g27Ps3a_gearShifter1])
  {
    g27Ps3->shifter.x = G27_SHIFTER_LEFT;
    g27Ps3->shifter.y = G27_SHIFTER_UP;
    g27Ps3->buttons |= G27_SHIFTER_1_MASK;
  }
  if (axis[g27Ps3a_gearShifter2])
  {
    g27Ps3->shifter.x = G27_SHIFTER_LEFT;
    g27Ps3->shifter.y = G27_SHIFTER_DOWN;
    g27Ps3->buttons |= G27_SHIFTER_2_MASK;
  }
  if (axis[g27Ps3a_gearShifter3])
  {
    g27Ps3->shifter.x = G27_SHIFTER_CENTER;
    g27Ps3->shifter.y = G27_SHIFTER_UP;
    g27Ps3->buttons |= G27_SHIFTER_3_MASK;
  }
  if (axis[g27Ps3a_gearShifter4])
  {
    g27Ps3->shifter.x = G27_SHIFTER_CENTER;
    g27Ps3->shifter.y = G27_SHIFTER_DOWN;
    g27Ps3->buttons |= G27_SHIFTER_4_MASK;
  }
  if (axis[g27Ps3a_gearShifter5])
  {
    g27Ps3->shifter.x = G27_SHIFTER_RIGHT;
    g27Ps3->shifter.y = G27_SHIFTER_UP;
    g27Ps3->buttons |= G27_SHIFTER_5_MASK;
  }
  if (axis[g27Ps3a_gearShifter6])
  {
    g27Ps3->shifter.x = G27_SHIFTER_RIGHT;
    g27Ps3->shifter.y = G27_SHIFTER_DOWN;
    g27Ps3->buttons |= G27_SHIFTER_6_MASK;
  }
  if (axis[g27Ps3a_gearShifterR])
  {
    g27Ps3->shifter.x = G27_SHIFTER_RIGHT;
    g27Ps3->shifter.y = G27_SHIFTER_DOWN;
    g27Ps3->shifter.b |= (G27_SHIFTER_REVERSE_ENGAGED | G27_SHIFTER_STICK_DOWN);
  }
  if (axis[g27Ps3a_l4])
  {
    g27Ps3->buttonsAndWheel |= G27_L4_MASK;
  }
  if (axis[g27Ps3a_l5])
  {
    g27Ps3->buttonsAndWheel |= G27_L5_MASK;
  }

  return index;
}

static s_controller controller =
{
  .name = "G27 PS3",
  .vid = 0x046d,
  .pid = 0xc29b,
  .refresh_period = { .min_value = 1000, .default_value = 10000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void g27Ps3_init(void) __attribute__((constructor));
void g27Ps3_init(void)
{
  controller_register(C_TYPE_G27_PS3, &controller);
}

