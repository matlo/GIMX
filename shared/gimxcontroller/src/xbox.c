/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <xbox.h>
#include <report.h>
#include <limits.h>
#include <string.h>
#include "../include/controller.h"

static s_axis axes[AXIS_MAX] =
{
  [xboxa_lstick_x]  = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_lstick_y]  = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_rstick_x]  = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_rstick_y]  = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_back]      = { .name = "back",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_start]     = { .name = "start",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_up]        = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_right]     = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_down]      = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_left]      = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_Y]         = { .name = "Y",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_B]         = { .name = "B",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_A]         = { .name = "A",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_X]         = { .name = "X",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_white]     = { .name = "white",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_black]     = { .name = "black",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_LT]        = { .name = "LT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_RT]        = { .name = "RT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_LS]        = { .name = "LS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xboxa_RS]        = { .name = "RS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
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

static s_report_xbox default_report =
{
  .type = 0x00,
  .size = 0x14,
  .buttons = 0x00,
  .ununsed2 = 0x00,
  .btnA = 0x00,
  .btnB = 0x00,
  .btnX = 0x00,
  .btnY = 0x00,
  .btnBlack = 0x00,
  .btnWhite = 0x00,
  .ltrigger = 0x00,
  .rtrigger = 0x00,
  .xaxis = 0x0000,
  .yaxis = 0x0000,
  .zaxis = 0x0000,
  .taxis = 0x0000,
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static inline void axis2button(int axis[AXIS_MAX], e_xone_axis_index index,
    unsigned char* buttons, unsigned short button_mask)
{
  if (axis[index])
  {
    (*buttons) |= button_mask;
  }
}

static inline void axis2axis(int from, short * to)
{
  *to = clamp(SHRT_MIN, from, SHRT_MAX);
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_xbox);
  s_report_xbox* xbox = &report[index].value.xbox;

  xbox->buttons = 0x00;

  axis2button(axis, xboxa_up, &xbox->buttons, XBOX_UP_MASK);
  axis2button(axis, xboxa_down, &xbox->buttons, XBOX_DOWN_MASK);
  axis2button(axis, xboxa_left, &xbox->buttons, XBOX_LEFT_MASK);
  axis2button(axis, xboxa_right, &xbox->buttons, XBOX_RIGHT_MASK);

  axis2button(axis, xboxa_start, &xbox->buttons, XBOX_START_MASK);
  axis2button(axis, xboxa_back, &xbox->buttons, XBOX_BACK_MASK);
  axis2button(axis, xboxa_LS, &xbox->buttons, XBOX_LS_MASK);
  axis2button(axis, xboxa_RS, &xbox->buttons, XBOX_RS_MASK);

  xbox->ltrigger = clamp(0, axis[xboxa_LT], MAX_AXIS_VALUE_8BITS);
  xbox->rtrigger = clamp(0, axis[xboxa_RT], MAX_AXIS_VALUE_8BITS);
  xbox->btnA = clamp(0, axis[xboxa_A], MAX_AXIS_VALUE_8BITS);
  xbox->btnB = clamp(0, axis[xboxa_B], MAX_AXIS_VALUE_8BITS);
  xbox->btnX = clamp(0, axis[xboxa_X], MAX_AXIS_VALUE_8BITS);
  xbox->btnY = clamp(0, axis[xboxa_Y], MAX_AXIS_VALUE_8BITS);
  xbox->btnWhite = clamp(0, axis[xboxa_white], MAX_AXIS_VALUE_8BITS);
  xbox->btnBlack = clamp(0, axis[xboxa_black], MAX_AXIS_VALUE_8BITS);

  axis2axis(axis[xboxa_lstick_x], &xbox->xaxis);
  axis2axis(-axis[xboxa_lstick_y], &xbox->yaxis);
  axis2axis(axis[xboxa_rstick_x], &xbox->zaxis);
  axis2axis(-axis[xboxa_rstick_y], &xbox->taxis);

  return index;
}

static s_controller controller =
{
  .name = "XboxPad",
  .vid = 0x045e,
  .pid = 0x0202,
  .refresh_period = { .min_value = 4000, .default_value = 8000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void xbox_init(void) __attribute__((constructor));
void xbox_init(void)
{
  controller_register(C_TYPE_XBOX_PAD, &controller);
}

