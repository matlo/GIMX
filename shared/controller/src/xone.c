/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <xone.h>
#include <report.h>
#include <controller.h>

static const char *xone_axis_name[AXIS_MAX] =
{
  [xonea_lstick_x] = "lstick x",
  [xonea_lstick_y] = "lstick y",
  [xonea_rstick_x] = "rstick x",
  [xonea_rstick_y] = "rstick y",
  [xonea_view] = "view",
  [xonea_menu] = "menu",
  [xonea_guide] = "guide",
  [xonea_up] = "up",
  [xonea_right] = "right",
  [xonea_down] = "down",
  [xonea_left] = "left",
  [xonea_Y] = "Y",
  [xonea_B] = "B",
  [xonea_A] = "A",
  [xonea_X] = "X",
  [xonea_LB] = "LB",
  [xonea_RB] = "RB",
  [xonea_LT] = "LT",
  [xonea_RT] = "RT",
  [xonea_LS] = "LS",
  [xonea_RS] = "RS",
};

static int xone_max_unsigned_axis_value[AXIS_MAX] =
{
  [xonea_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [xonea_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [xonea_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [xonea_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [xonea_view] = MAX_AXIS_VALUE_8BITS,
  [xonea_menu] = MAX_AXIS_VALUE_8BITS,
  [xonea_guide] = MAX_AXIS_VALUE_8BITS,
  [xonea_up] = MAX_AXIS_VALUE_8BITS,
  [xonea_right] = MAX_AXIS_VALUE_8BITS,
  [xonea_down] = MAX_AXIS_VALUE_8BITS,
  [xonea_left] = MAX_AXIS_VALUE_8BITS,
  [xonea_Y] = MAX_AXIS_VALUE_8BITS,
  [xonea_B] = MAX_AXIS_VALUE_8BITS,
  [xonea_A] = MAX_AXIS_VALUE_8BITS,
  [xonea_X] = MAX_AXIS_VALUE_8BITS,
  [xonea_LB] = MAX_AXIS_VALUE_8BITS,
  [xonea_RB] = MAX_AXIS_VALUE_8BITS,
  [xonea_LT] = MAX_AXIS_VALUE_8BITS,
  [xonea_RT] = MAX_AXIS_VALUE_8BITS,
  [xonea_LS] = MAX_AXIS_VALUE_8BITS,
  [xonea_RS] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params xone_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 4000,
    .max_unsigned_axis_value = xone_max_unsigned_axis_value
};

static unsigned int xone_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_xone* xone = &report->value.xone;

  int axis_value;

  /*
   * TODO XONE
   */
  
  xone->type = 0x00;
  xone->size = 0x14;

  xone->buttons = 0x0000;

  if (axis[xonea_up])
  {
    xone->buttons |= 0x0001;
  }
  if (axis[xonea_down])
  {
    xone->buttons |= 0x0002;
  }
  if (axis[xonea_left])
  {
    xone->buttons |= 0x0004;
  }
  if (axis[xonea_right])
  {
    xone->buttons |= 0x0008;
  }

  if (axis[xonea_menu])
  {
    xone->buttons |= 0x0010;
  }
  if (axis[xonea_view])
  {
    xone->buttons |= 0x0020;
  }
  if (axis[xonea_LS])
  {
    xone->buttons |= 0x0040;
  }
  if (axis[xonea_RS])
  {
    xone->buttons |= 0x0080;
  }

  if (axis[xonea_LB])
  {
    xone->buttons |= 0x0100;
  }
  if (axis[xonea_RB])
  {
    xone->buttons |= 0x0200;
  }
  if (axis[xonea_guide])
  {
    xone->buttons |= 0x0400;
  }

  if (axis[xonea_A])
  {
    xone->buttons |= 0x1000;
  }
  if (axis[xonea_B])
  {
    xone->buttons |= 0x2000;
  }
  if (axis[xonea_X])
  {
    xone->buttons |= 0x4000;
  }
  if (axis[xonea_Y])
  {
    xone->buttons |= 0x8000;
  }

  xone->ltrigger = clamp(0, axis[xonea_LT], 255);
  xone->rtrigger = clamp(0, axis[xonea_RT], 255);

  axis_value = axis[xonea_lstick_x];
  xone->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xone->xaxis |= 0xFF;
  }
  axis_value = - axis[xonea_lstick_y];
  xone->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xone->yaxis |= 0xFF;
  }
  axis_value = axis[xonea_rstick_x];
  xone->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xone->zaxis |= 0xFF;
  }
  axis_value = -axis[xonea_rstick_y];
  xone->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    xone->taxis |= 0xFF;
  }

  return sizeof(*xone);
}

void xone_init(void) __attribute__((constructor (101)));
void xone_init(void)
{
  controller_register_params(C_TYPE_XONE_PAD, &xone_params);

  control_register_names(C_TYPE_XONE_PAD, xone_axis_name);

  report_register_builder(C_TYPE_XONE_PAD, xone_report_build);
}
