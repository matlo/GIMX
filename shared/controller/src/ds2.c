/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds2.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static s_axis axes[AXIS_MAX] =
{
    [ds2a_lstick_x] = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_lstick_y] = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_rstick_x] = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_rstick_y] = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_select] =   { .name = "select",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_start] =    { .name = "start",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_up] =       { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_right] =    { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_down] =     { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_left] =     { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_triangle] = { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_circle] =   { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_cross] =    { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_square] =   { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_l1] =       { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_r1] =       { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_l2] =       { .name = "l2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_r2] =       { .name = "r2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_l3] =       { .name = "l3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [ds2a_r3] =       { .name = "r3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "rstick x",     {.axis = ds2a_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = ds2a_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = ds2a_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = ds2a_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = ds2a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = ds2a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = ds2a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = ds2a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = ds2a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = ds2a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = ds2a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = ds2a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "select",       {.axis = ds2a_select,   .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = ds2a_start,    .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = ds2a_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = ds2a_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = ds2a_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = ds2a_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = ds2a_r1,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r2",           {.axis = ds2a_r2,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = ds2a_r3,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = ds2a_l1,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l2",           {.axis = ds2a_l2,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = ds2a_l3,       .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = ds2a_circle,   .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = ds2a_square,   .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = ds2a_cross,    .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = ds2a_triangle, .props = AXIS_PROP_TOGGLE}},
};

static s_report_ds2 default_report =
{
  .head = 0x5A,
  .Bt1 = 0xFF,
  .Bt2 = 0xFF,

  .Z = CENTER_AXIS_VALUE_8BITS,
  .Rz = CENTER_AXIS_VALUE_8BITS,
  .X = CENTER_AXIS_VALUE_8BITS,
  .Y = CENTER_AXIS_VALUE_8BITS,
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_ds2);
  s_report_ds2* ds2 = &report[index].value.ds2;

  ds2->Bt1 = 0xFF;
  ds2->Bt2 = 0xFF;

  ds2->X = clamp(0, axis[ds2a_lstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds2->Y = clamp(0, axis[ds2a_lstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds2->Z = clamp(0, axis[ds2a_rstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds2->Rz = clamp(0, axis[ds2a_rstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);

  if (axis[ds2a_square])
  {
    ds2->Bt2 &= ~0x80;
  }
  if (axis[ds2a_cross])
  {
    ds2->Bt2 &= ~0x40;
  }
  if (axis[ds2a_circle])
  {
    ds2->Bt2 &= ~0x20;
  }
  if (axis[ds2a_triangle])
  {
    ds2->Bt2 &= ~0x10;
  }

  if (axis[ds2a_select])
  {
    ds2->Bt1 &= ~0x01;
  }
  if (axis[ds2a_start])
  {
    ds2->Bt1 &= ~0x08;
  }
  if (axis[ds2a_l3])
  {
    ds2->Bt1 &= ~0x02;
  }
  if (axis[ds2a_r3])
  {
    ds2->Bt1 &= ~0x04;
  }

  if (axis[ds2a_l1])
  {
    ds2->Bt2 &= ~0x04;
  }
  if (axis[ds2a_r1])
  {
    ds2->Bt2 &= ~0x08;
  }
  if (axis[ds2a_l2])
  {
    ds2->Bt2 &= ~0x01;
  }
  if (axis[ds2a_r2])
  {
    ds2->Bt2 &= ~0x02;
  }

  if (axis[ds2a_up])
  {
    ds2->Bt1 &= ~0x10;
  }
  if (axis[ds2a_right])
  {
    ds2->Bt1 &= ~0x20;
  }
  if (axis[ds2a_down])
  {
    ds2->Bt1 &= ~0x40;
  }
  if (axis[ds2a_left])
  {
    ds2->Bt1 &= ~0x80;
  }

  return index;
}

static s_controller controller =
{
    .name = "PS2pad",
    .refresh_period = { .min_value = 16000, .default_value = 16000 },
    .axes = axes,
    .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
    .fp_build_report = build_report,
    .fp_init_report = init_report,
};

void ds2_init(void) __attribute__((constructor (101)));
void ds2_init(void)
{
  controller_register(C_TYPE_PS2_PAD, &controller);
}
