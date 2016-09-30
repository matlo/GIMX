/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <adapter.h>
#include <stddef.h>
#include <stdio.h>
#include <report2event/ds42event.h>
#include <report2event/360Pad2event.h>
#include <report2event/xOnePad2event.h>

static struct
{
  void (*r2e)(int adapter_id, s_report* current, s_report* previous,
      int joystick_id, int (*callback)(GE_Event*));
} controllers[C_TYPE_MAX] =
{
  [C_TYPE_DS4] =
  {
    .r2e = ds42event
  },
  [C_TYPE_T300RS_PS4] =
  {
    .r2e = ds42event
  },
  [C_TYPE_G29_PS4] =
  {
    .r2e = ds42event
  },
  [C_TYPE_360_PAD] =
  {
    .r2e = _360Pad2event
  },
  [C_TYPE_XONE_PAD] =
  {
    .r2e = xOnePad2event
  }
};

int (*event_callback)(GE_Event*) = NULL;

void report2event_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

void report2event(e_controller_type type, int adapter_id, s_report* current,
    s_report* previous, int joystick_id)
{
  if (controllers[type].r2e)
  {
    if (event_callback)
    {
      controllers[type].r2e(adapter_id, current, previous, joystick_id, event_callback);
    }
  }
  else
  {
    fprintf(stderr, "Controller is missing a report2event function: %s\n", controller_get_name(type));
  }
}
