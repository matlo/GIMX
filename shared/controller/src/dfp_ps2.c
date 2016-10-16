/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <dfp_ps2.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

#define DFP_CROSS_MASK           0x4000
#define DFP_SQUARE_MASK          0x8000

#define DFP_CIRCLE_MASK          0x0001
#define DFP_TRIANGLE_MASK        0x0002
#define DFP_R1_MASK              0x0004
#define DFP_L1_MASK              0x0008
#define DFP_R2_MASK              0x0010
#define DFP_L2_MASK              0x0020
#define DFP_SELECT_MASK          0x0040
#define DFP_START_MASK           0x0080
#define DFP_R3_MASK              0x0100
#define DFP_L3_MASK              0x0200
#define DFP_SHIFTER_BACK_MASK    0x0400
#define DFP_SHIFTER_FORWARD_MASK 0x0800

static s_axis axes[AXIS_MAX] =
{
  [dfpPs2a_wheel]        = { .name = "wheel",          .max_unsigned_value = MAX_AXIS_VALUE_14BITS },
  
  [dfpPs2a_gasPedal]     = { .name = "gas",            .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_brakePedal]   = { .name = "brake",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_select]       = { .name = "select",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_start]        = { .name = "start",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_up]           = { .name = "up",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_right]        = { .name = "right",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_down]         = { .name = "down",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_left]         = { .name = "left",           .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_triangle]     = { .name = "triangle",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_circle]       = { .name = "circle",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_cross]        = { .name = "cross",          .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_square]       = { .name = "square",         .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_l1]           = { .name = "l1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_r1]           = { .name = "r1",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_l2]           = { .name = "l2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_r2]           = { .name = "r2",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_l3]           = { .name = "l3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_r3]           = { .name = "r3",             .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

  [dfpPs2a_shifter_forward] = { .name = "shifter forward", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [dfpPs2a_shifter_back]    = { .name = "shifter back",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "wheel",        {.axis = dfpPs2a_wheel,      .props = AXIS_PROP_CENTERED}},

  {.name = "gas",          {.axis = dfpPs2a_gasPedal,   .props = AXIS_PROP_POSITIVE}},
  {.name = "brake",        {.axis = dfpPs2a_brakePedal, .props = AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = dfpPs2a_r2,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = dfpPs2a_l2,         .props = AXIS_PROP_TOGGLE}},

  {.name = "select",       {.axis = dfpPs2a_select,     .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = dfpPs2a_start,      .props = AXIS_PROP_TOGGLE}},

  {.name = "up",           {.axis = dfpPs2a_up,         .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = dfpPs2a_down,       .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = dfpPs2a_right,      .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = dfpPs2a_left,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = dfpPs2a_r1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = dfpPs2a_r3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = dfpPs2a_l1,         .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = dfpPs2a_l3,         .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = dfpPs2a_circle,     .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = dfpPs2a_square,     .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = dfpPs2a_cross,      .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = dfpPs2a_triangle,   .props = AXIS_PROP_TOGGLE}},

  {.name = "shifter forward", {.axis = dfpPs2a_shifter_forward, .props = AXIS_PROP_TOGGLE}},
  {.name = "shifter back",    {.axis = dfpPs2a_shifter_back,    .props = AXIS_PROP_TOGGLE}},
};

static s_report_dfpPs2 default_report =
{
  .buttonsAndWheel = CENTER_AXIS_VALUE_14BITS,
  .hatAndButtons = 0x8000,
  .unknown1 = 0x7f,
  .gasPedal = MAX_AXIS_VALUE_8BITS,
  .brakePedal = MAX_AXIS_VALUE_8BITS,
  .unknown2 = 0xff,
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_dfpPs2);
  s_report_dfpPs2* dfpPs2 = &report[index].value.dfpPs2;

  dfpPs2->buttonsAndWheel = clamp(0, axis[dfpPs2a_wheel] + CENTER_AXIS_VALUE_14BITS, MAX_AXIS_VALUE_14BITS);
  dfpPs2->gasPedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[dfpPs2a_gasPedal], MAX_AXIS_VALUE_8BITS);
  dfpPs2->brakePedal = clamp(0, MAX_AXIS_VALUE_8BITS - axis[dfpPs2a_brakePedal], MAX_AXIS_VALUE_8BITS);

  dfpPs2->hatAndButtons = 0x0000;
  
  if (axis[dfpPs2a_right])
  {
    if (axis[dfpPs2a_down])
    {
      dfpPs2->hatAndButtons = 0x3000;
    }
    else if (axis[dfpPs2a_up])
    {
      dfpPs2->hatAndButtons = 0x1000;
    }
    else
    {
      dfpPs2->hatAndButtons = 0x2000;
    }
  }
  else if (axis[dfpPs2a_left])
  {
    if (axis[dfpPs2a_down])
    {
      dfpPs2->hatAndButtons = 0x5000;
    }
    else if (axis[dfpPs2a_up])
    {
      dfpPs2->hatAndButtons = 0x7000;
    }
    else
    {
      dfpPs2->hatAndButtons = 0x6000;
    }
  }
  else if (axis[dfpPs2a_down])
  {
    dfpPs2->hatAndButtons = 0x4000;
  }
  else if (axis[dfpPs2a_up])
  {
    dfpPs2->hatAndButtons = 0x0000;
  }
  else
  {
    dfpPs2->hatAndButtons = 0x8000;
  }

  if (axis[dfpPs2a_cross])
  {
    dfpPs2->buttonsAndWheel |= DFP_CROSS_MASK;
  }
  if (axis[dfpPs2a_square])
  {
    dfpPs2->buttonsAndWheel |= DFP_SQUARE_MASK;
  }

  if (axis[dfpPs2a_circle])
  {
    dfpPs2->hatAndButtons |= DFP_CIRCLE_MASK;
  }
  if (axis[dfpPs2a_triangle])
  {
    dfpPs2->hatAndButtons |= DFP_TRIANGLE_MASK;
  }
  if (axis[dfpPs2a_r1])
  {
    dfpPs2->hatAndButtons |= DFP_R1_MASK;
  }
  if (axis[dfpPs2a_l1])
  {
    dfpPs2->hatAndButtons |= DFP_L1_MASK;
  }
  if (axis[dfpPs2a_l2])
  {
    dfpPs2->hatAndButtons |= DFP_L2_MASK;
  }
  if (axis[dfpPs2a_r2])
  {
    dfpPs2->hatAndButtons |= DFP_R2_MASK;
  }
  if (axis[dfpPs2a_start])
  {
    dfpPs2->hatAndButtons |= DFP_START_MASK;
  }
  if (axis[dfpPs2a_select])
  {
    dfpPs2->hatAndButtons |= DFP_SELECT_MASK;
  }
  if (axis[dfpPs2a_l3])
  {
    dfpPs2->hatAndButtons |= DFP_L3_MASK;
  }
  if (axis[dfpPs2a_r3])
  {
    dfpPs2->hatAndButtons |= DFP_R3_MASK;
  }
  if (axis[dfpPs2a_shifter_forward])
  {
    dfpPs2->hatAndButtons |= DFP_SHIFTER_FORWARD_MASK;
  }
  if (axis[dfpPs2a_shifter_back])
  {
    dfpPs2->hatAndButtons |= DFP_SHIFTER_BACK_MASK;
  }

  return index;
}

static s_controller controller =
{
  .name = "DFP PS2",
  .vid = 0x046d,
  .pid = 0xc298,
  .refresh_period = { .min_value = 10000, .default_value = 10000 },
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void dfpPs2_init(void) __attribute__((constructor (101)));
void dfpPs2_init(void)
{
  controller_register(C_TYPE_DFP_PS2, &controller);
}

