/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <report.h>
#include <usb_spoof.h>

extern unsigned int joystick_report_build(int axis[AXIS_MAX], s_report* report);
extern unsigned int ds2_report_build(int axis[AXIS_MAX], s_report* report);
extern unsigned int ds3_report_build(int axis[AXIS_MAX], s_report* report);
extern unsigned int ds4_report_build(int axis[AXIS_MAX], s_report* report);
extern unsigned int xbox_report_build(int axis[AXIS_MAX], s_report* report);
extern unsigned int x360_report_build(int axis[AXIS_MAX], s_report* report);

unsigned int (*func_ptr[C_TYPE_MAX])(int axis[AXIS_MAX], s_report* report) =
{
    [C_TYPE_JOYSTICK] = joystick_report_build,
    [C_TYPE_PS2_PAD] = ds2_report_build,
    [C_TYPE_SIXAXIS] = ds3_report_build,
    [C_TYPE_DS4] = ds4_report_build,
    [C_TYPE_XBOX_PAD] = xbox_report_build,
    [C_TYPE_360_PAD] = x360_report_build,
    [C_TYPE_DEFAULT] = ds3_report_build,
};

unsigned int report_build(s_controller* controller, s_report* report)
{
  unsigned int ret = 0;
  if(func_ptr[controller->type])
  {
    ret = func_ptr[controller->type](controller->axis, report);
  }
  return ret;
}
