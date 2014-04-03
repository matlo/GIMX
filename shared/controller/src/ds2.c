/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds2.h>
#include <report.h>
#include <controller.h>

static const char *ds2_axis_name[AXIS_MAX] =
{
  [ds2a_lstick_x] = "lstick x",
  [ds2a_lstick_y] = "lstick y",
  [ds2a_rstick_x] = "rstick x",
  [ds2a_rstick_y] = "rstick y",
  [ds2a_select] = "select",
  [ds2a_start] = "start",
  [ds2a_up] = "up",
  [ds2a_right] = "right",
  [ds2a_down] = "down",
  [ds2a_left] = "left",
  [ds2a_triangle] = "triangle",
  [ds2a_circle] = "circle",
  [ds2a_cross] = "cross",
  [ds2a_square] = "square",
  [ds2a_l1] = "l1",
  [ds2a_r1] = "r1",
  [ds2a_l2] = "l2",
  [ds2a_r2] = "r2",
  [ds2a_l3] = "l3",
  [ds2a_r3] = "r3",
};

static int ds2_max_unsigned_axis_value[AXIS_MAX] =
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

static s_controller_params ds2_params =
{
    .min_refresh_period = 16000,
    .default_refresh_period = 16000,
    .max_unsigned_axis_value = ds2_max_unsigned_axis_value
};

static unsigned int ds2_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_ds2* ds2 = &report->value.ds2;

  ds2->head = 0x5A;
  ds2->Bt1 = 0xFF;
  ds2->Bt2 = 0xFF;

  ds2->X = clamp(0, axis[ds2a_lstick_x] + 128, 255);
  ds2->Y = clamp(0, axis[ds2a_lstick_y] + 128, 255);
  ds2->Z = clamp(0, axis[ds2a_rstick_x] + 128, 255);
  ds2->Rz = clamp(0, axis[ds2a_rstick_y] + 128, 255);

  if (axis[ds2a_square])
  {
    ds2->Bt2 &= ~0x80;
  }
  if (axis[ds2a_cross])
  {
    ds2->Bt2 &= ~0x40;
  }
  if (axis[ds2a_circle])
  {
    ds2->Bt2 &= ~0x20;
  }
  if (axis[ds2a_triangle])
  {
    ds2->Bt2 &= ~0x10;
  }

  if (axis[ds2a_select])
  {
    ds2->Bt1 &= ~0x01;
  }
  if (axis[ds2a_start])
  {
    ds2->Bt1 &= ~0x08;
  }
  if (axis[ds2a_l3])
  {
    ds2->Bt1 &= ~0x02;
  }
  if (axis[ds2a_r3])
  {
    ds2->Bt1 &= ~0x04;
  }

  if (axis[ds2a_l1])
  {
    ds2->Bt2 &= ~0x04;
  }
  if (axis[ds2a_r1])
  {
    ds2->Bt2 &= ~0x08;
  }
  if (axis[ds2a_l2])
  {
    ds2->Bt2 &= ~0x01;
  }
  if (axis[ds2a_r2])
  {
    ds2->Bt2 &= ~0x02;
  }

  if (axis[ds2a_up])
  {
    ds2->Bt1 &= ~0x10;
  }
  if (axis[ds2a_right])
  {
    ds2->Bt1 &= ~0x20;
  }
  if (axis[ds2a_down])
  {
    ds2->Bt1 &= ~0x40;
  }
  if (axis[ds2a_left])
  {
    ds2->Bt1 &= ~0x80;
  }

  return sizeof(*ds2);
}

void ds2_init(void) __attribute__((constructor (101)));
void ds2_init(void)
{
  controller_register_params(C_TYPE_PS2_PAD, &ds2_params);

  control_register_names(C_TYPE_PS2_PAD, ds2_axis_name);

  report_register_builder(C_TYPE_PS2_PAD, ds2_report_build);
}
