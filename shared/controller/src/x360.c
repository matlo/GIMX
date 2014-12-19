/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <x360.h>
#include <report.h>
#include <controller2.h>

static const char *x360_axis_name[AXIS_MAX] =
{
  [x360a_lstick_x] = "lstick x",
  [x360a_lstick_y] = "lstick y",
  [x360a_rstick_x] = "rstick x",
  [x360a_rstick_y] = "rstick y",
  [x360a_back] = "back",
  [x360a_start] = "start",
  [x360a_guide] = "guide",
  [x360a_up] = "up",
  [x360a_right] = "right",
  [x360a_down] = "down",
  [x360a_left] = "left",
  [x360a_Y] = "Y",
  [x360a_B] = "B",
  [x360a_A] = "A",
  [x360a_X] = "X",
  [x360a_LB] = "LB",
  [x360a_RB] = "RB",
  [x360a_LT] = "LT",
  [x360a_RT] = "RT",
  [x360a_LS] = "LS",
  [x360a_RS] = "RS",
};

static s_axis_name_dir axis_names[] =
{
  {.name = "rstick x",     {.axis = x360a_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = x360a_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = x360a_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = x360a_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = x360a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = x360a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = x360a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = x360a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = x360a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = x360a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = x360a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = x360a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "LT",           {.axis = x360a_LT,       .props = AXIS_PROP_POSITIVE}},
  {.name = "RT",           {.axis = x360a_RT,       .props = AXIS_PROP_POSITIVE}},

  {.name = "back",         {.axis = x360a_back,     .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = x360a_start,    .props = AXIS_PROP_TOGGLE}},
  {.name = "guide",        {.axis = x360a_guide,    .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = x360a_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = x360a_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = x360a_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = x360a_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "LB",           {.axis = x360a_LB,       .props = AXIS_PROP_TOGGLE}},
  {.name = "RB",           {.axis = x360a_RB,       .props = AXIS_PROP_TOGGLE}},
  {.name = "LS",           {.axis = x360a_LS,       .props = AXIS_PROP_TOGGLE}},
  {.name = "RS",           {.axis = x360a_RS,       .props = AXIS_PROP_TOGGLE}},
  {.name = "A",            {.axis = x360a_A,        .props = AXIS_PROP_TOGGLE}},
  {.name = "B",            {.axis = x360a_B,        .props = AXIS_PROP_TOGGLE}},
  {.name = "X",            {.axis = x360a_X,        .props = AXIS_PROP_TOGGLE}},
  {.name = "Y",            {.axis = x360a_Y,        .props = AXIS_PROP_TOGGLE}},
};

static int x360_max_unsigned_axis_value[AXIS_MAX] =
{
  [x360a_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [x360a_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [x360a_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [x360a_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [x360a_back] = MAX_AXIS_VALUE_8BITS,
  [x360a_start] = MAX_AXIS_VALUE_8BITS,
  [x360a_guide] = MAX_AXIS_VALUE_8BITS,
  [x360a_up] = MAX_AXIS_VALUE_8BITS,
  [x360a_right] = MAX_AXIS_VALUE_8BITS,
  [x360a_down] = MAX_AXIS_VALUE_8BITS,
  [x360a_left] = MAX_AXIS_VALUE_8BITS,
  [x360a_Y] = MAX_AXIS_VALUE_8BITS,
  [x360a_B] = MAX_AXIS_VALUE_8BITS,
  [x360a_A] = MAX_AXIS_VALUE_8BITS,
  [x360a_X] = MAX_AXIS_VALUE_8BITS,
  [x360a_LB] = MAX_AXIS_VALUE_8BITS,
  [x360a_RB] = MAX_AXIS_VALUE_8BITS,
  [x360a_LT] = MAX_AXIS_VALUE_8BITS,
  [x360a_RT] = MAX_AXIS_VALUE_8BITS,
  [x360a_LS] = MAX_AXIS_VALUE_8BITS,
  [x360a_RS] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params x360_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 8000,
    .max_unsigned_axis_value = x360_max_unsigned_axis_value
};

static unsigned int x360_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_x360* x360 = &report->value.x360;

  int axis_value;

  x360->type = 0x00;
  x360->size = 0x14;

  x360->buttons = 0x0000;

  if (axis[x360a_up])
  {
    x360->buttons |= 0x0001;
  }
  if (axis[x360a_down])
  {
    x360->buttons |= 0x0002;
  }
  if (axis[x360a_left])
  {
    x360->buttons |= 0x0004;
  }
  if (axis[x360a_right])
  {
    x360->buttons |= 0x0008;
  }

  if (axis[x360a_start])
  {
    x360->buttons |= 0x0010;
  }
  if (axis[x360a_back])
  {
    x360->buttons |= 0x0020;
  }
  if (axis[x360a_LS])
  {
    x360->buttons |= 0x0040;
  }
  if (axis[x360a_RS])
  {
    x360->buttons |= 0x0080;
  }

  if (axis[x360a_LB])
  {
    x360->buttons |= 0x0100;
  }
  if (axis[x360a_RB])
  {
    x360->buttons |= 0x0200;
  }
  if (axis[x360a_guide])
  {
    x360->buttons |= 0x0400;
  }

  if (axis[x360a_A])
  {
    x360->buttons |= 0x1000;
  }
  if (axis[x360a_B])
  {
    x360->buttons |= 0x2000;
  }
  if (axis[x360a_X])
  {
    x360->buttons |= 0x4000;
  }
  if (axis[x360a_Y])
  {
    x360->buttons |= 0x8000;
  }

  x360->ltrigger = clamp(0, axis[x360a_LT], MAX_AXIS_VALUE_8BITS);
  x360->rtrigger = clamp(0, axis[x360a_RT], MAX_AXIS_VALUE_8BITS);

  axis_value = axis[x360a_lstick_x];
  x360->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    x360->xaxis |= 0xFF;
  }
  axis_value = - axis[x360a_lstick_y];
  x360->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    x360->yaxis |= 0xFF;
  }
  axis_value = axis[x360a_rstick_x];
  x360->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    x360->zaxis |= 0xFF;
  }
  axis_value = -axis[x360a_rstick_y];
  x360->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    x360->taxis |= 0xFF;
  }

  return sizeof(*x360);
}

void x360_init(void) __attribute__((constructor (101)));
void x360_init(void)
{
  controller_register_axis_names(C_TYPE_360_PAD, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_360_PAD, &x360_params);

  control_register_names(C_TYPE_360_PAD, x360_axis_name);

  report_register_builder(C_TYPE_360_PAD, x360_report_build);
}
