/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <joystick.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static s_axis axes[AXIS_MAX] =
{
  [jsa_lstick_x]  = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [jsa_lstick_y]  = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [jsa_rstick_x]  = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [jsa_rstick_y]  = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  
  [jsa_B8]        = { .name = "select",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B9]        = { .name = "start",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_up]        = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_right]     = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_down]      = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_left]      = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B3]        = { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B2]        = { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B1]        = { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B0]        = { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B4]        = { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B5]        = { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B6]        = { .name = "l2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B7]        = { .name = "r2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B10]       = { .name = "l3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [jsa_B11]       = { .name = "r3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "rstick x",     {.axis = jsa_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = jsa_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = jsa_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = jsa_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = jsa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = jsa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = jsa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = jsa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = jsa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = jsa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = jsa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = jsa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "select",       {.axis = jsa_B8,       .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = jsa_B9,       .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = jsa_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = jsa_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = jsa_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = jsa_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = jsa_B5,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r2",           {.axis = jsa_B7,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = jsa_B11,      .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = jsa_B4,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = jsa_B6,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = jsa_B10,      .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = jsa_B2,       .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = jsa_B0,       .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = jsa_B1,       .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = jsa_B3,       .props = AXIS_PROP_TOGGLE}},
};

static s_report_joystick default_report =
{
  .X = CENTER_AXIS_VALUE_16BITS,
  .Y = CENTER_AXIS_VALUE_16BITS,
  .Z = CENTER_AXIS_VALUE_16BITS,
  .Rz = CENTER_AXIS_VALUE_16BITS,
  .Hat = 0x0008,
  .Bt = 0x0000,
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_joystick);
  s_report_joystick* js = &report[index].value.js;

  js->X = clamp(0, axis[jsa_lstick_x] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);
  js->Y = clamp(0, axis[jsa_lstick_y] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);
  js->Z = clamp(0, axis[jsa_rstick_x] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);
  js->Rz = clamp(0, axis[jsa_rstick_y] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

  js->Bt = 0x0000;

  if (axis[jsa_B0])
  {
    js->Bt |= 0x0001;
  }
  if (axis[jsa_B1])
  {
    js->Bt |= 0x0002;
  }
  if (axis[jsa_B2])
  {
    js->Bt |= 0x0004;
  }
  if (axis[jsa_B3])
  {
    js->Bt |= 0x0008;
  }

  if (axis[jsa_B4])
  {
    js->Bt |= 0x0010;
  }
  if (axis[jsa_B5])
  {
    js->Bt |= 0x0020;
  }
  if (axis[jsa_B6])
  {
    js->Bt |= 0x0040;
  }
  if (axis[jsa_B7])
  {
    js->Bt |= 0x0080;
  }

  if (axis[jsa_B8])
  {
    js->Bt |= 0x0100;
  }
  if (axis[jsa_B9])
  {
    js->Bt |= 0x0200;
  }
  if (axis[jsa_B10])
  {
    js->Bt |= 0x0400;
  }
  if (axis[jsa_B11])
  {
    js->Bt |= 0x0800;
  }

  if (axis[jsa_B12])
  {
    js->Bt |= 0x1000;
  }

  if (axis[jsa_right])
  {
    if (axis[jsa_down])
    {
      js->Hat = 0x0003;
    }
    else if (axis[jsa_up])
    {
      js->Hat = 0x0001;
    }
    else
    {
      js->Hat = 0x0002;
    }
  }
  else if (axis[jsa_left])
  {
    if (axis[jsa_down])
    {
      js->Hat = 0x0005;
    }
    else if (axis[jsa_up])
    {
      js->Hat = 0x0007;
    }
    else
    {
      js->Hat = 0x0006;
    }
  }
  else if (axis[jsa_down])
  {
    js->Hat = 0x0004;
  }
  else if (axis[jsa_up])
  {
    js->Hat = 0x0000;
  }
  else
  {
    js->Hat = 0x0008;
  }

  return index;
}

static s_controller controller =
{
  .name = "joystick",
  .refresh_period = { .min_value = 1000, .default_value = 4000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void joystick_init(void) __attribute__((constructor));
void joystick_init(void)
{
  controller_register(C_TYPE_JOYSTICK, &controller);
}

