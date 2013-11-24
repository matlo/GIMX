/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "controllers/controller.h"
#include "report.h"

int ds2_max_unsigned_axis_value[AXIS_MAX] =
{
  [ds2a_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds2a_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds2a_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds2a_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds2a_select] = MAX_AXIS_VALUE_8BITS,
  [ds2a_start] = MAX_AXIS_VALUE_8BITS,
  [ds2a_up] = MAX_AXIS_VALUE_8BITS,
  [ds2a_right] = MAX_AXIS_VALUE_8BITS,
  [ds2a_down] = MAX_AXIS_VALUE_8BITS,
  [ds2a_left] = MAX_AXIS_VALUE_8BITS,
  [ds2a_triangle] = MAX_AXIS_VALUE_8BITS,
  [ds2a_circle] = MAX_AXIS_VALUE_8BITS,
  [ds2a_cross] = MAX_AXIS_VALUE_8BITS,
  [ds2a_square] = MAX_AXIS_VALUE_8BITS,
  [ds2a_l1] = MAX_AXIS_VALUE_8BITS,
  [ds2a_r1] = MAX_AXIS_VALUE_8BITS,
  [ds2a_l2] = MAX_AXIS_VALUE_8BITS,
  [ds2a_r2] = MAX_AXIS_VALUE_8BITS,
  [ds2a_l3] = MAX_AXIS_VALUE_8BITS,
  [ds2a_r3] = MAX_AXIS_VALUE_8BITS,
};

unsigned int ds2_report_build(s_controller* controller, s_report* report)
{
  s_report_ds2* report_ds2 = &report->value.ds2;

  report_ds2->head = 0x5A;
  report_ds2->Bt1 = 0xFF;
  report_ds2->Bt2 = 0xFF;

  report_ds2->X = clamp(0, controller->axis[ds2a_lstick_x] + 128, 255);
  report_ds2->Y = clamp(0, controller->axis[ds2a_lstick_y] + 128, 255);
  report_ds2->Z = clamp(0, controller->axis[ds2a_rstick_x] + 128, 255);
  report_ds2->Rz = clamp(0, controller->axis[ds2a_rstick_y] + 128, 255);

  if (controller->axis[ds2a_square])
  {
    report_ds2->Bt2 &= ~0x80;
  }
  if (controller->axis[ds2a_cross])
  {
    report_ds2->Bt2 &= ~0x40;
  }
  if (controller->axis[ds2a_circle])
  {
    report_ds2->Bt2 &= ~0x20;
  }
  if (controller->axis[ds2a_triangle])
  {
    report_ds2->Bt2 &= ~0x10;
  }

  if (controller->axis[ds2a_select])
  {
    report_ds2->Bt1 &= ~0x01;
  }
  if (controller->axis[ds2a_start])
  {
    report_ds2->Bt1 &= ~0x08;
  }
  if (controller->axis[ds2a_l3])
  {
    report_ds2->Bt1 &= ~0x02;
  }
  if (controller->axis[ds2a_r3])
  {
    report_ds2->Bt1 &= ~0x04;
  }

  if (controller->axis[ds2a_l1])
  {
    report_ds2->Bt2 &= ~0x04;
  }
  if (controller->axis[ds2a_r1])
  {
    report_ds2->Bt2 &= ~0x08;
  }
  if (controller->axis[ds2a_l2])
  {
    report_ds2->Bt2 &= ~0x01;
  }
  if (controller->axis[ds2a_r2])
  {
    report_ds2->Bt2 &= ~0x02;
  }

  if (controller->axis[ds2a_up])
  {
    report_ds2->Bt1 &= ~0x10;
  }
  if (controller->axis[ds2a_right])
  {
    report_ds2->Bt1 &= ~0x20;
  }
  if (controller->axis[ds2a_down])
  {
    report_ds2->Bt1 &= ~0x40;
  }
  if (controller->axis[ds2a_left])
  {
    report_ds2->Bt1 &= ~0x80;
  }

  return sizeof(*report_ds2);
}
