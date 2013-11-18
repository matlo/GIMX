/*
 * ds4.c
 *
 *  Created on: 14 nov. 2013
 *      Author: matlo
 */

#include "controllers/ds3.h"
#include <report.h>

int ds4_max_unsigned_axis_value[AXIS_MAX] =
{
  [ds4a_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds4a_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds4a_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds4a_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds4a_up] = MAX_AXIS_VALUE_8BITS,
  [ds4a_right] = MAX_AXIS_VALUE_8BITS,
  [ds4a_down] = MAX_AXIS_VALUE_8BITS,
  [ds4a_left] = MAX_AXIS_VALUE_8BITS,
  [ds4a_square] = MAX_AXIS_VALUE_8BITS,
  [ds4a_cross] = MAX_AXIS_VALUE_8BITS,
  [ds4a_circle] = MAX_AXIS_VALUE_8BITS,
  [ds4a_triangle] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l1] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r1] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l2] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r2] = MAX_AXIS_VALUE_8BITS,
  [ds4a_select] = MAX_AXIS_VALUE_8BITS,
  [ds4a_start] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l3] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r3] = MAX_AXIS_VALUE_8BITS,
  [ds4a_ps] = MAX_AXIS_VALUE_8BITS,
  [ds4a_touchpad] = MAX_AXIS_VALUE_8BITS,
};

/*
 * Work in progress...
 * Do not assume the code in the following function is right!
 */
unsigned int ds4_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_ds4* report_ds4 = &report->value.ds4;

  report_ds4->report_id = 0x01;

  report_ds4->X = clamp(0, axis[ds4a_lstick_x] + 128, 255);
  report_ds4->Y = clamp(0, axis[ds4a_lstick_y] + 128, 255);
  report_ds4->Z = clamp(0, axis[ds4a_rstick_x] + 128, 255);
  report_ds4->Rz = clamp(0, axis[ds4a_rstick_y] + 128, 255);

  if (axis[ds4a_right])
  {
    if (axis[ds4a_down])
    {
      report_ds4->HatAndButtons = 0x03;
    }
    else if (axis[ds4a_up])
    {
      report_ds4->HatAndButtons = 0x01;
    }
    else
    {
      report_ds4->HatAndButtons = 0x02;
    }
  }
  else if (axis[ds4a_left])
  {
    if (axis[ds4a_down])
    {
      report_ds4->HatAndButtons = 0x05;
    }
    else if (axis[ds4a_up])
    {
      report_ds4->HatAndButtons = 0x07;
    }
    else
    {
      report_ds4->HatAndButtons = 0x06;
    }
  }
  else if (axis[ds4a_down])
  {
    report_ds4->HatAndButtons = 0x04;
  }
  else if (axis[ds4a_up])
  {
    report_ds4->HatAndButtons = 0x00;
  }
  else
  {
    report_ds4->HatAndButtons = 0x08;
  }

  if (axis[ds4a_square])
  {
    report_ds4->HatAndButtons |= 0x10;
  }
  if (axis[ds4a_cross])
  {
    report_ds4->HatAndButtons |= 0x20;
  }
  if (axis[ds4a_circle])
  {
    report_ds4->HatAndButtons |= 0x40;
  }
  if (axis[ds4a_triangle])
  {
    report_ds4->HatAndButtons |= 0x80;
  }

  if (axis[ds4a_l1])
  {
    report_ds4->ButtonsAndVendor |= 0x0001;
  }
  if (axis[ds4a_r1])
  {
    report_ds4->ButtonsAndVendor |= 0x0002;
  }
  if (axis[ds4a_l2])
  {
    report_ds4->ButtonsAndVendor |= 0x0004;
  }
  if (axis[ds4a_r2])
  {
    report_ds4->ButtonsAndVendor |= 0x0008;
  }
  if (axis[ds4a_select])
  {
    report_ds4->ButtonsAndVendor |= 0x0010;
  }
  if (axis[ds4a_start])
  {
    report_ds4->ButtonsAndVendor |= 0x0020;
  }
  if (axis[ds4a_l3])
  {
    report_ds4->ButtonsAndVendor |= 0x0040;
  }
  if (axis[ds4a_r3])
  {
    report_ds4->ButtonsAndVendor |= 0x0080;
  }
  if (axis[ds4a_ps])
  {
    report_ds4->ButtonsAndVendor |= 0x0100;
  }
  if (axis[ds4a_touchpad])
  {
    report_ds4->ButtonsAndVendor |= 0x0200;
  }

  report_ds4->Rx = clamp(0, axis[ds4a_l2], 255);
  report_ds4->Ry = clamp(0, axis[ds4a_r2], 255);

  return sizeof(*report_ds4);
}
