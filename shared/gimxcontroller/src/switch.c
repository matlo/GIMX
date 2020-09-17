/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <switch.h>
#include <report.h>
#include <limits.h>
#include <string.h>
#include "../include/controller.h"

static s_axis axes[AXIS_MAX] =
{
  [switcha_lstick_x]  = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_12BITS },
  [switcha_lstick_y]  = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_12BITS },
  [switcha_rstick_x]  = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_12BITS },
  [switcha_rstick_y]  = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_12BITS },

  [switcha_minus]      = { .name = "minus",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_plus]     = { .name = "plus",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_home]     = { .name = "home",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_up]        = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_right]     = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_down]      = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_left]      = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_x]         = { .name = "X",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_a]         = { .name = "A",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_b]         = { .name = "B",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_y]         = { .name = "Y",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_l]        = { .name = "L",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_r]        = { .name = "R",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_zl]        = { .name = "ZL",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_zr]        = { .name = "ZR",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_lclick]        = { .name = "lclick",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_rclick]        = { .name = "rclick",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [switcha_capture]        = { .name = "capture",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "rstick x",     {.axis = switcha_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = switcha_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = switcha_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = switcha_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = switcha_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = switcha_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = switcha_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = switcha_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = switcha_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = switcha_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = switcha_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = switcha_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "ZL",           {.axis = switcha_zl,       .props = AXIS_PROP_TOGGLE}},
  {.name = "ZR",           {.axis = switcha_zr,       .props = AXIS_PROP_TOGGLE}},

  {.name = "minus",         {.axis = switcha_minus,     .props = AXIS_PROP_TOGGLE}},
  {.name = "plus",        {.axis = switcha_plus,    .props = AXIS_PROP_TOGGLE}},
  {.name = "home",        {.axis = switcha_home,    .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = switcha_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = switcha_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = switcha_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = switcha_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "L",           {.axis = switcha_l,       .props = AXIS_PROP_TOGGLE}},
  {.name = "R",           {.axis = switcha_r,       .props = AXIS_PROP_TOGGLE}},
  {.name = "lclick",           {.axis = switcha_lclick,       .props = AXIS_PROP_TOGGLE}},
  {.name = "rclick",           {.axis = switcha_rclick,       .props = AXIS_PROP_TOGGLE}},
  {.name = "B",            {.axis = switcha_b,        .props = AXIS_PROP_TOGGLE}},
  {.name = "A",            {.axis = switcha_a,        .props = AXIS_PROP_TOGGLE}},
  {.name = "Y",            {.axis = switcha_y,        .props = AXIS_PROP_TOGGLE}},
  {.name = "X",            {.axis = switcha_x,        .props = AXIS_PROP_TOGGLE}},
};

static s_report_switch default_report =
{
  .connection_info = 1,
  .battery_level = BATTERY_FULL | BATTERY_CHARGING,
  .buttons = SWITCH_CHARGING_GRIP,
  .analog = { 0x0, 0x8, 0x80, 0x0, 0x8, 0x80 },
  .vibrator_input_report = 0x0c,
  .imu = { 0 },
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

#define axis2button(axis, index, buttons, button_mask) \
    if (axis[index]) { buttons |= button_mask; }

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_switch);
  s_report_switch* ns = &report[index].value.ns;

  ns->connection_info = 1; // Pro Controller + USB connected
  ns->battery_level = BATTERY_FULL | BATTERY_CHARGING;

  ns->buttons = SWITCH_CHARGING_GRIP;

  axis2button(axis, switcha_up, ns->buttons, SWITCH_DPADUP);
  axis2button(axis, switcha_down, ns->buttons, SWITCH_DPADDOWN);
  axis2button(axis, switcha_left, ns->buttons, SWITCH_DPADLEFT);
  axis2button(axis, switcha_right, ns->buttons, SWITCH_DPADRIGHT);

  axis2button(axis, switcha_plus, ns->buttons, SWITCH_PLUS);
  axis2button(axis, switcha_minus, ns->buttons, SWITCH_MINUS);
  axis2button(axis, switcha_lclick, ns->buttons, SWITCH_LCLICK);
  axis2button(axis, switcha_rclick, ns->buttons, SWITCH_RCLICK);

  axis2button(axis, switcha_l, ns->buttons, SWITCH_L);
  axis2button(axis, switcha_r, ns->buttons, SWITCH_R);
  axis2button(axis, switcha_home, ns->buttons, SWITCH_HOME);
  axis2button(axis, switcha_capture, ns->buttons, SWITCH_CAPTURE);

  axis2button(axis, switcha_a, ns->buttons, SWITCH_A);
  axis2button(axis, switcha_b, ns->buttons, SWITCH_B);
  axis2button(axis, switcha_x, ns->buttons, SWITCH_X);
  axis2button(axis, switcha_y, ns->buttons, SWITCH_Y);

  axis2button(axis, switcha_zl, ns->buttons, SWITCH_ZL);
  axis2button(axis, switcha_zr, ns->buttons, SWITCH_ZR);

  // Left stick
  unsigned long lx = clamp(0, axis[switcha_lstick_x] + CENTER_AXIS_VALUE_12BITS, MAX_AXIS_VALUE_12BITS);
  unsigned long ly = clamp(0, -axis[switcha_lstick_y] + CENTER_AXIS_VALUE_12BITS, MAX_AXIS_VALUE_12BITS);
  ns->analog[0] = lx & 0xFF;
  ns->analog[1] = ((ly & 0x0F) << 4) | ((lx & 0xF00) >> 8);
  ns->analog[2] = (ly & 0xFF0) >> 4;

  // Right stick
  unsigned long rx = clamp(0, axis[switcha_rstick_x] + CENTER_AXIS_VALUE_12BITS, MAX_AXIS_VALUE_12BITS);
  unsigned long ry = clamp(0, -axis[switcha_rstick_y] + CENTER_AXIS_VALUE_12BITS, MAX_AXIS_VALUE_12BITS);
  ns->analog[3] = rx & 0xFF;
  ns->analog[4] = ((ry & 0x0F) << 4) | ((rx & 0xF00) >> 8);
  ns->analog[5] = (ry & 0xFF0) >> 4;

  return index;
}

static s_controller controller =
{
  .name = "switchpad",
  .vid = 0x0000,
  .pid = 0x0000,
  .refresh_period = { .min_value = 2000, .default_value = 8000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void switch_init(void) __attribute__((constructor));
void switch_init(void)
{
  controller_register(C_TYPE_SWITCH, &controller);
}

