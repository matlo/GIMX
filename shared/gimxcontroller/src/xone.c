/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <xone.h>
#include <report.h>
#include <limits.h>
#include <string.h>
#include "../include/controller.h"

static s_axis axes[AXIS_MAX] =
{
  [xonea_lstick_x]  = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [xonea_lstick_y]  = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [xonea_rstick_x]  = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  [xonea_rstick_y]  = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_16BITS },
  
  [xonea_view]      = { .name = "view",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_menu]      = { .name = "menu",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_guide]     = { .name = "guide",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_up]        = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_right]     = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_down]      = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_left]      = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_Y]         = { .name = "Y",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_B]         = { .name = "B",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_A]         = { .name = "A",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_X]         = { .name = "X",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_LB]        = { .name = "LB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_RB]        = { .name = "RB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  
  [xonea_LT]        = { .name = "LT",       .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
  [xonea_RT]        = { .name = "RT",       .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
  
  [xonea_LS]        = { .name = "LS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [xonea_RS]        = { .name = "RS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "rstick x",     {.axis = xonea_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = xonea_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = xonea_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = xonea_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = xonea_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = xonea_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = xonea_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = xonea_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = xonea_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = xonea_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = xonea_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = xonea_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "LT",           {.axis = xonea_LT,       .props = AXIS_PROP_POSITIVE}},
  {.name = "RT",           {.axis = xonea_RT,       .props = AXIS_PROP_POSITIVE}},

  {.name = "view",         {.axis = xonea_view,     .props = AXIS_PROP_TOGGLE}},
  {.name = "menu",         {.axis = xonea_menu,     .props = AXIS_PROP_TOGGLE}},
  {.name = "guide",        {.axis = xonea_guide,    .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = xonea_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = xonea_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = xonea_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = xonea_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "LB",           {.axis = xonea_LB,       .props = AXIS_PROP_TOGGLE}},
  {.name = "RB",           {.axis = xonea_RB,       .props = AXIS_PROP_TOGGLE}},
  {.name = "LS",           {.axis = xonea_LS,       .props = AXIS_PROP_TOGGLE}},
  {.name = "RS",           {.axis = xonea_RS,       .props = AXIS_PROP_TOGGLE}},
  {.name = "A",            {.axis = xonea_A,        .props = AXIS_PROP_TOGGLE}},
  {.name = "B",            {.axis = xonea_B,        .props = AXIS_PROP_TOGGLE}},
  {.name = "X",            {.axis = xonea_X,        .props = AXIS_PROP_TOGGLE}},
  {.name = "Y",            {.axis = xonea_Y,        .props = AXIS_PROP_TOGGLE}},
};

static s_report_xone default_report =
{
  .input = {
    .type = XONE_USB_HID_IN_REPORT_ID,
    .unknown = 0x00,
    .counter = 0x00,
    .size = 0x0e,
    .buttons = 0x00,
    .ltrigger = 0x0000,
    .rtrigger = 0x0000,
    .xaxis = 0x0000,
    .yaxis = 0x0000,
    .zaxis = 0x0000,
    .taxis = 0x0000,
  }
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

#define axis2button(axis, index, buttons, button_mask) \
    if (axis[index]) { buttons |= button_mask; }

#define axis2axis(from, to) \
    to = clamp(SHRT_MIN, from, SHRT_MAX)

#define guide report[1].value.xone.guide
#define input report[0].value.xone.input

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index;

  unsigned char guide_button = axis[xonea_guide] ? XONE_GUIDE_MASK : 0x00;

  if(guide_button ^ guide.button)
  {
    index = 1;
    report[index].length = sizeof(guide);

    guide.type = XONE_USB_HID_IN_GUIDE_REPORT_ID;
    guide.unknown1 = 0x20;
    guide.counter++;
    guide.size = 0x02;
    guide.button = guide_button;
    guide.unknown2 = 0x5b;
  }
  else
  {
    index = 0;
    report[index].length = sizeof(input);

    input.type = XONE_USB_HID_IN_REPORT_ID;
    input.counter++;
    input.size = 0x0e;

    input.buttons = 0x0000;

    axis2button(axis, xonea_up, input.buttons, XONE_UP_MASK);
    axis2button(axis, xonea_down, input.buttons, XONE_DOWN_MASK);
    axis2button(axis, xonea_left, input.buttons, XONE_LEFT_MASK);
    axis2button(axis, xonea_right, input.buttons, XONE_RIGHT_MASK);

    axis2button(axis, xonea_view, input.buttons, XONE_VIEW_MASK);
    axis2button(axis, xonea_menu, input.buttons, XONE_MENU_MASK);
    axis2button(axis, xonea_LS, input.buttons, XONE_LS_MASK);
    axis2button(axis, xonea_RS, input.buttons, XONE_RS_MASK);

    axis2button(axis, xonea_LB, input.buttons, XONE_LB_MASK);
    axis2button(axis, xonea_RB, input.buttons, XONE_RB_MASK);
    axis2button(axis, xonea_guide, input.buttons, XONE_GUIDE_MASK);

    axis2button(axis, xonea_A, input.buttons, XONE_A_MASK);
    axis2button(axis, xonea_B, input.buttons, XONE_B_MASK);
    axis2button(axis, xonea_X, input.buttons, XONE_X_MASK);
    axis2button(axis, xonea_Y, input.buttons, XONE_Y_MASK);

    input.ltrigger = clamp(0, axis[xonea_LT], MAX_AXIS_VALUE_10BITS);
    input.rtrigger = clamp(0, axis[xonea_RT], MAX_AXIS_VALUE_10BITS);

    axis2axis(axis[xonea_lstick_x], input.xaxis);
    axis2axis(-axis[xonea_lstick_y], input.yaxis);
    axis2axis(axis[xonea_rstick_x], input.zaxis);
    axis2axis(-axis[xonea_rstick_y], input.taxis);
  }

  return index;
}

static s_controller controller =
{
  .name = "XOnePad",
  .vid = XONE_VENDOR,
  .pid = XONE_PRODUCT,
  .refresh_period = { .min_value = 1000, .default_value = 4000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
};

void xone_init(void) __attribute__((constructor));
void xone_init(void)
{
  controller_register(C_TYPE_XONE_PAD, &controller);
}

