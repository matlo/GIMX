/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "controllers/controller.h"
#include "report.h"

int joystick_max_unsigned_axis_value[AXIS_MAX] =
{
  [jsa_lstick_x] = MAX_AXIS_VALUE_16BITS,
  [jsa_lstick_y] = MAX_AXIS_VALUE_16BITS,
  [jsa_rstick_x] = MAX_AXIS_VALUE_16BITS,
  [jsa_rstick_y] = MAX_AXIS_VALUE_16BITS,
  [jsa_up] = MAX_AXIS_VALUE_8BITS,
  [jsa_right] = MAX_AXIS_VALUE_8BITS,
  [jsa_down] = MAX_AXIS_VALUE_8BITS,
  [jsa_left] = MAX_AXIS_VALUE_8BITS,
  [jsa_B0] = MAX_AXIS_VALUE_8BITS,
  [jsa_B1] = MAX_AXIS_VALUE_8BITS,
  [jsa_B2] = MAX_AXIS_VALUE_8BITS,
  [jsa_B3] = MAX_AXIS_VALUE_8BITS,
  [jsa_B4] = MAX_AXIS_VALUE_8BITS,
  [jsa_B5] = MAX_AXIS_VALUE_8BITS,
  [jsa_B6] = MAX_AXIS_VALUE_8BITS,
  [jsa_B7] = MAX_AXIS_VALUE_8BITS,
  [jsa_B8] = MAX_AXIS_VALUE_8BITS,
  [jsa_B9] = MAX_AXIS_VALUE_8BITS,
  [jsa_B10] = MAX_AXIS_VALUE_8BITS,
  [jsa_B11] = MAX_AXIS_VALUE_8BITS,
  [jsa_B12] = MAX_AXIS_VALUE_8BITS,
};

unsigned int joystick_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_joystick* report_js = &report->value.js;

  report_js->X = clamp(0, axis[jsa_lstick_x] + 32768, 65535);
  report_js->Y = clamp(0, axis[jsa_lstick_y] + 32768, 65535);
  report_js->Z = clamp(0, axis[jsa_rstick_x] + 32768, 65535);
  report_js->Rz = clamp(0, axis[jsa_rstick_y] + 32768, 65535);

  report_js->Bt = 0x0000;

  if (axis[jsa_B0])
  {
    report_js->Bt |= 0x0001;
  }
  if (axis[jsa_B1])
  {
    report_js->Bt |= 0x0002;
  }
  if (axis[jsa_B2])
  {
    report_js->Bt |= 0x0004;
  }
  if (axis[jsa_B3])
  {
    report_js->Bt |= 0x0008;
  }

  if (axis[jsa_B4])
  {
    report_js->Bt |= 0x0010;
  }
  if (axis[jsa_B5])
  {
    report_js->Bt |= 0x0020;
  }
  if (axis[jsa_B6])
  {
    report_js->Bt |= 0x0040;
  }
  if (axis[jsa_B7])
  {
    report_js->Bt |= 0x0080;
  }

  if (axis[jsa_B8])
  {
    report_js->Bt |= 0x0100;
  }
  if (axis[jsa_B9])
  {
    report_js->Bt |= 0x0200;
  }
  if (axis[jsa_B10])
  {
    report_js->Bt |= 0x0400;
  }
  if (axis[jsa_B11])
  {
    report_js->Bt |= 0x0800;
  }

  if (axis[jsa_B12])
  {
    report_js->Bt |= 0x1000;
  }

  if (axis[jsa_right])
  {
    if (axis[jsa_down])
    {
      report_js->Hat = 0x0003;
    }
    else if (axis[jsa_up])
    {
      report_js->Hat = 0x0001;
    }
    else
    {
      report_js->Hat = 0x0002;
    }
  }
  else if (axis[jsa_left])
  {
    if (axis[jsa_down])
    {
      report_js->Hat = 0x0005;
    }
    else if (axis[jsa_up])
    {
      report_js->Hat = 0x0007;
    }
    else
    {
      report_js->Hat = 0x0006;
    }
  }
  else if (axis[jsa_down])
  {
    report_js->Hat = 0x0004;
  }
  else if (axis[jsa_up])
  {
    report_js->Hat = 0x0000;
  }
  else
  {
    report_js->Hat = 0x0008;
  }

  return sizeof(*report_js);
}

