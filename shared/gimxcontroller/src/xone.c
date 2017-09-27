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

static inline void axis2button(int axis[AXIS_MAX], e_xone_axis_index index,
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
  unsigned int index;

  unsigned char guide_button = axis[xonea_guide] ? XONE_GUIDE_MASK : 0x00;

  if(guide_button ^ report[1].value.xone.guide.button)
  {
    index = 1;
    report[index].length = sizeof(report->value.xone.guide);
    s_report_xone* xone = &report[index].value.xone;

    xone->guide.type = XONE_USB_HID_IN_GUIDE_REPORT_ID;
    xone->guide.unknown1 = 0x20;
    xone->guide.counter++;
    xone->guide.size = 0x02;
    xone->guide.button = guide_button;
    xone->guide.unknown2 = 0x5b;
  }
  else
  {
    index = 0;
    report[index].length = sizeof(report->value.xone.input);
    s_report_xone* xone = &report[index].value.xone;

    xone->input.type = XONE_USB_HID_IN_REPORT_ID;
    xone->input.counter++;
    xone->input.size = 0x0e;

    xone->input.buttons = 0x0000;

    axis2button(axis, xonea_up, &xone->input.buttons, XONE_UP_MASK);
    axis2button(axis, xonea_down, &xone->input.buttons, XONE_DOWN_MASK);
    axis2button(axis, xonea_left, &xone->input.buttons, XONE_LEFT_MASK);
    axis2button(axis, xonea_right, &xone->input.buttons, XONE_RIGHT_MASK);

    axis2button(axis, xonea_view, &xone->input.buttons, XONE_VIEW_MASK);
    axis2button(axis, xonea_menu, &xone->input.buttons, XONE_MENU_MASK);
    axis2button(axis, xonea_LS, &xone->input.buttons, XONE_LS_MASK);
    axis2button(axis, xonea_RS, &xone->input.buttons, XONE_RS_MASK);

    axis2button(axis, xonea_LB, &xone->input.buttons, XONE_LB_MASK);
    axis2button(axis, xonea_RB, &xone->input.buttons, XONE_RB_MASK);
    axis2button(axis, xonea_guide, &xone->input.buttons, XONE_GUIDE_MASK);

    axis2button(axis, xonea_A, &xone->input.buttons, XONE_A_MASK);
    axis2button(axis, xonea_B, &xone->input.buttons, XONE_B_MASK);
    axis2button(axis, xonea_X, &xone->input.buttons, XONE_X_MASK);
    axis2button(axis, xonea_Y, &xone->input.buttons, XONE_Y_MASK);

    xone->input.ltrigger = clamp(0, axis[xonea_LT], MAX_AXIS_VALUE_10BITS);
    xone->input.rtrigger = clamp(0, axis[xonea_RT], MAX_AXIS_VALUE_10BITS);

    axis2axis(axis[xonea_lstick_x], &xone->input.xaxis);
    axis2axis(-axis[xonea_lstick_y], &xone->input.yaxis);
    axis2axis(axis[xonea_rstick_x], &xone->input.zaxis);
    axis2axis(-axis[xonea_rstick_y], &xone->input.taxis);
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
  .activation_button = xonea_guide,
};

void xone_init(void) __attribute__((constructor));
void xone_init(void)
{
  controller_register(C_TYPE_XONE_PAD, &controller);
}

