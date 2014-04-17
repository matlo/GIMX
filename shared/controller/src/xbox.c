/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <xbox.h>
#include <report.h>
#include <controller.h>

static const char *xbox_axis_name[AXIS_MAX] =
{
  [xboxa_lstick_x] = "lstick x",
  [xboxa_lstick_y] = "lstick y",
  [xboxa_rstick_x] = "rstick x",
  [xboxa_rstick_y] = "rstick y",
  [xboxa_back] = "back",
  [xboxa_start] = "start",
  [xboxa_up] = "up",
  [xboxa_right] = "right",
  [xboxa_down] = "down",
  [xboxa_left] = "left",
  [xboxa_Y] = "Y",
  [xboxa_B] = "B",
  [xboxa_A] = "A",
  [xboxa_X] = "X",
  [xboxa_white] = "white",
  [xboxa_black] = "black",
  [xboxa_LT] = "LT",
  [xboxa_RT] = "RT",
  [xboxa_LS] = "LS",
  [xboxa_RS] = "RS",
};

static s_axis_name_dir axis_names[] =
{
  {.name = "rstick x",     {.axis = xboxa_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = xboxa_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = xboxa_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = xboxa_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = xboxa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = xboxa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = xboxa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = xboxa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = xboxa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = xboxa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = xboxa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = xboxa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "white",        {.axis = xboxa_white,    .props = AXIS_PROP_POSITIVE}},
  {.name = "black",        {.axis = xboxa_black,    .props = AXIS_PROP_POSITIVE}},
  {.name = "LT",           {.axis = xboxa_LT,       .props = AXIS_PROP_POSITIVE}},
  {.name = "RT",           {.axis = xboxa_RT,       .props = AXIS_PROP_POSITIVE}},
  {.name = "A",            {.axis = xboxa_A,        .props = AXIS_PROP_POSITIVE}},
  {.name = "B",            {.axis = xboxa_B,        .props = AXIS_PROP_POSITIVE}},
  {.name = "X",            {.axis = xboxa_X,        .props = AXIS_PROP_POSITIVE}},
  {.name = "Y",            {.axis = xboxa_Y,        .props = AXIS_PROP_POSITIVE}},

  {.name = "back",         {.axis = xboxa_back,     .props = AXIS_PROP_TOGGLE}},
  {.name = "start",        {.axis = xboxa_start,    .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = xboxa_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = xboxa_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = xboxa_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = xboxa_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "LS",           {.axis = xboxa_LS,       .props = AXIS_PROP_TOGGLE}},
  {.name = "RS",           {.axis = xboxa_RS,       .props = AXIS_PROP_TOGGLE}},
};

static int xbox_max_unsigned_axis_value[AXIS_MAX] =
{
  [xboxa_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [xboxa_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [xboxa_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [xboxa_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [xboxa_back] = MAX_AXIS_VALUE_8BITS,
  [xboxa_start] = MAX_AXIS_VALUE_8BITS,
  [xboxa_up] = MAX_AXIS_VALUE_8BITS,
  [xboxa_right] = MAX_AXIS_VALUE_8BITS,
  [xboxa_down] = MAX_AXIS_VALUE_8BITS,
  [xboxa_left] = MAX_AXIS_VALUE_8BITS,
  [xboxa_Y] = MAX_AXIS_VALUE_8BITS,
  [xboxa_B] = MAX_AXIS_VALUE_8BITS,
  [xboxa_A] = MAX_AXIS_VALUE_8BITS,
  [xboxa_X] = MAX_AXIS_VALUE_8BITS,
  [xboxa_white] = MAX_AXIS_VALUE_8BITS,
  [xboxa_black] = MAX_AXIS_VALUE_8BITS,
  [xboxa_LT] = MAX_AXIS_VALUE_8BITS,
  [xboxa_RT] = MAX_AXIS_VALUE_8BITS,
  [xboxa_LS] = MAX_AXIS_VALUE_8BITS,
  [xboxa_RS] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params xbox_params =
{
    .min_refresh_period = 4000,
    .default_refresh_period = 8000,
    .max_unsigned_axis_value = xbox_max_unsigned_axis_value
};

static unsigned int xbox_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_xbox* xbox = &report->value.xbox;

  int axis_value;

  xbox->type = 0x00;
  xbox->size = 0x14;

  xbox->buttons = 0x00;

  if (axis[xboxa_up])
  {
    xbox->buttons |= 0x01;
  }
  if (axis[xboxa_down])
  {
    xbox->buttons |= 0x02;
  }
  if (axis[xboxa_left])
  {
    xbox->buttons |= 0x04;
  }
  if (axis[xboxa_right])
  {
    xbox->buttons |= 0x08;
  }

  if (axis[xboxa_start])
  {
    xbox->buttons |= 0x10;
  }
  if (axis[xboxa_back])
  {
    xbox->buttons |= 0x20;
  }
  if (axis[xboxa_LS])
  {
    xbox->buttons |= 0x40;
  }
  if (axis[xboxa_RS])
  {
    xbox->buttons |= 0x80;
  }

  xbox->ltrigger = clamp(0, axis[xboxa_LT], 255);
  xbox->rtrigger = clamp(0, axis[xboxa_RT], 255);
  xbox->btnA = clamp(0, axis[xboxa_A], 255);
  xbox->btnB = clamp(0, axis[xboxa_B], 255);
  xbox->btnX = clamp(0, axis[xboxa_X], 255);
  xbox->btnY = clamp(0, axis[xboxa_Y], 255);
  xbox->btnWhite = clamp(0, axis[xboxa_white], 255);
  xbox->btnBlack = clamp(0, axis[xboxa_black], 255);

  axis_value = axis[xboxa_lstick_x];
  xbox->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xbox->xaxis |= 0xFF;
  }
  axis_value = - axis[xboxa_lstick_y];
  xbox->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xbox->yaxis |= 0xFF;
  }
  axis_value = axis[xboxa_rstick_x];
  xbox->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xbox->zaxis |= 0xFF;
  }
  axis_value = -axis[xboxa_rstick_y];
  xbox->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xbox->taxis |= 0xFF;
  }

  return sizeof(*xbox);
}

void xbox_init(void) __attribute__((constructor (101)));
void xbox_init(void)
{
  controller_register_axis_names(C_TYPE_XBOX_PAD, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_XBOX_PAD, &xbox_params);

  control_register_names(C_TYPE_XBOX_PAD, xbox_axis_name);

  report_register_builder(C_TYPE_XBOX_PAD, xbox_report_build);
}
