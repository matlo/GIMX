/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <x360.h>
#include <report.h>
#include <controller2.h>
#include <limits.h>
#include <string.h>

static s_axis axes[AXIS_MAX] =
{
  [x360a_lstick_x]  = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [x360a_lstick_y]  = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [x360a_rstick_x]  = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [x360a_rstick_y]  = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  
  [x360a_back]      = { .name = "back",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_start]     = { .name = "start",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_guide]     = { .name = "guide",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_up]        = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_right]     = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_down]      = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_left]      = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_Y]         = { .name = "Y",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_B]         = { .name = "B",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_A]         = { .name = "A",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_X]         = { .name = "X",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_LB]        = { .name = "LB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_RB]        = { .name = "RB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_LT]        = { .name = "LT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_RT]        = { .name = "RT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_LS]        = { .name = "LS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [x360a_RS]        = { .name = "RS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
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

static s_report_x360 default_report =
{
  .type = X360_USB_HID_IN_REPORT_ID,
  .size = 0x14,
  .buttons = 0x0000,
  .ltrigger = 0x00,
  .rtrigger = 0x00,
  .xaxis = 0x0000,
  .yaxis = 0x0000,
  .zaxis = 0x0000,
  .taxis = 0x0000,
  .unused = {},
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static inline void axis2button(int axis[AXIS_MAX], e_x360_axis_index index,
    unsigned short* buttons, unsigned short button_mask)
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
  report[index].length = sizeof(s_report_x360);
  s_report_x360* x360 = &report[index].value.x360;

  x360->buttons = 0x0000;

  axis2button(axis, x360a_up, &x360->buttons, X360_UP_MASK);
  axis2button(axis, x360a_down, &x360->buttons, X360_DOWN_MASK);
  axis2button(axis, x360a_left, &x360->buttons, X360_LEFT_MASK);
  axis2button(axis, x360a_right, &x360->buttons, X360_RIGHT_MASK);

  axis2button(axis, x360a_start, &x360->buttons, X360_START_MASK);
  axis2button(axis, x360a_back, &x360->buttons, X360_BACK_MASK);
  axis2button(axis, x360a_LS, &x360->buttons, X360_LS_MASK);
  axis2button(axis, x360a_RS, &x360->buttons, X360_RS_MASK);

  axis2button(axis, x360a_LB, &x360->buttons, X360_LB_MASK);
  axis2button(axis, x360a_RB, &x360->buttons, X360_RB_MASK);
  axis2button(axis, x360a_guide, &x360->buttons, X360_GUIDE_MASK);

  axis2button(axis, x360a_A, &x360->buttons, X360_A_MASK);
  axis2button(axis, x360a_B, &x360->buttons, X360_B_MASK);
  axis2button(axis, x360a_X, &x360->buttons, X360_X_MASK);
  axis2button(axis, x360a_Y, &x360->buttons, X360_Y_MASK);

  x360->ltrigger = clamp(0, axis[x360a_LT], UCHAR_MAX);
  x360->rtrigger = clamp(0, axis[x360a_RT], UCHAR_MAX);

  axis2axis(axis[x360a_lstick_x], &x360->xaxis);
  axis2axis(-axis[x360a_lstick_y], &x360->yaxis);
  axis2axis(axis[x360a_rstick_x], &x360->zaxis);
  axis2axis(-axis[x360a_rstick_y], &x360->taxis);

  return index;
}

static s_controller controller =
{
  .name = "360pad",
  .vid = X360_VENDOR,
  .pid = X360_PRODUCT,
  .refresh_period = { .min_value = 1000, .default_value = 8000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
  .activation_button = x360a_guide,
};

void x360_init(void) __attribute__((constructor));
void x360_init(void)
{
  controller_register(C_TYPE_360_PAD, &controller);
}

