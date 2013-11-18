/*
 * xbox.c
 *
 *  Created on: 14 nov. 2013
 *      Author: matlo
 */

#include "controllers/xbox.h"
#include "report.h"

int xbox_max_unsigned_axis_value[AXIS_MAX] =
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

unsigned int xbox_report_build(s_controller* controller, s_report* report)
{
  int axis_value;

  s_report_xbox* report_xbox = &report->value.xbox;

  report_xbox->type = 0x00;
  report_xbox->size = 0x14;

  report_xbox->buttons = 0x00;

  if (controller->axis[xboxa_up])
  {
    report_xbox->buttons |= 0x01;
  }
  if (controller->axis[xboxa_down])
  {
    report_xbox->buttons |= 0x02;
  }
  if (controller->axis[xboxa_left])
  {
    report_xbox->buttons |= 0x04;
  }
  if (controller->axis[xboxa_right])
  {
    report_xbox->buttons |= 0x08;
  }

  if (controller->axis[xboxa_start])
  {
    report_xbox->buttons |= 0x10;
  }
  if (controller->axis[xboxa_back])
  {
    report_xbox->buttons |= 0x20;
  }
  if (controller->axis[xboxa_LS])
  {
    report_xbox->buttons |= 0x40;
  }
  if (controller->axis[xboxa_RS])
  {
    report_xbox->buttons |= 0x80;
  }

  report_xbox->ltrigger = clamp(0, controller->axis[xboxa_LT], 255);
  report_xbox->rtrigger = clamp(0, controller->axis[xboxa_RT], 255);
  report_xbox->btnA = clamp(0, controller->axis[xboxa_A], 255);
  report_xbox->btnB = clamp(0, controller->axis[xboxa_B], 255);
  report_xbox->btnX = clamp(0, controller->axis[xboxa_X], 255);
  report_xbox->btnY = clamp(0, controller->axis[xboxa_Y], 255);
  report_xbox->btnWhite = clamp(0, controller->axis[xboxa_white], 255);
  report_xbox->btnBlack = clamp(0, controller->axis[xboxa_black], 255);

  axis_value = controller->axis[xboxa_lstick_x];
  report_xbox->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->xaxis |= 0xFF;
  }
  axis_value = - controller->axis[xboxa_lstick_y];
  report_xbox->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->yaxis |= 0xFF;
  }
  axis_value = controller->axis[xboxa_rstick_x];
  report_xbox->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->zaxis |= 0xFF;
  }
  axis_value = -controller->axis[xboxa_rstick_y];
  report_xbox->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->taxis |= 0xFF;
  }

  return sizeof(*report_xbox);
}
