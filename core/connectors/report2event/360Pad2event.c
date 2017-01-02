/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <report2event/360Pad2event.h>
#include <gimxcontroller/include/controller2.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static inline void axis2event(int (*callback)(GE_Event*), GE_Event* event, short axis,
    short paxis, char invert, uint8_t axis_id)
{
  if (axis != paxis)
  {
    event->jaxis.axis = axis_id;
    event->jaxis.value = invert ? ~axis : axis;
    callback(event);
  }
}

static inline void trigger2event(int (*callback)(GE_Event*), GE_Event* event, unsigned char trigger,
    unsigned char ptrigger, uint8_t axis_id)
{
  int axisValue;
  if (trigger != ptrigger)
  {
    event->jaxis.axis = axis_id;
    axisValue = trigger * (MAX_AXIS_VALUE_16BITS / 2) / MAX_AXIS_VALUE_8BITS;
    event->jaxis.value = clamp(0, axisValue, SHRT_MAX);
    callback(event);
  }
}

static inline void button2event(int (*callback)(GE_Event*), GE_Event* event, unsigned short buttons,
    unsigned short pbuttons, unsigned short button_mask, uint8_t button_id)
{
  unsigned short value;
  if((value = (buttons & button_mask)) ^ (pbuttons & button_mask))
  {
    event->type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event->jbutton.button = button_id;
    callback(event);
  }
}

void _360Pad2event(int adapter_id __attribute__((unused)), s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*))
{
  GE_Event event = { .jbutton = { .which = joystick_id } };

  s_report_x360* x360_current = &current->x360;
  s_report_x360* x360_previous = &previous->x360;

  /*
   * Buttons
   */

  unsigned short buttons = x360_current->buttons;
  unsigned short prevButtons = x360_previous->buttons;

  button2event(callback, &event, buttons, prevButtons, X360_UP_MASK, X360_UP_ID);
  button2event(callback, &event, buttons, prevButtons, X360_DOWN_MASK, X360_DOWN_ID);
  button2event(callback, &event, buttons, prevButtons, X360_LEFT_MASK, X360_LEFT_ID);
  button2event(callback, &event, buttons, prevButtons, X360_RIGHT_MASK, X360_RIGHT_ID);

  button2event(callback, &event, buttons, prevButtons, X360_START_MASK, X360_START_ID);
  button2event(callback, &event, buttons, prevButtons, X360_BACK_MASK, X360_BACK_ID);
  button2event(callback, &event, buttons, prevButtons, X360_LS_MASK, X360_LS_ID);
  button2event(callback, &event, buttons, prevButtons, X360_RS_MASK, X360_RS_ID);

  button2event(callback, &event, buttons, prevButtons, X360_LB_MASK, X360_LB_ID);
  button2event(callback, &event, buttons, prevButtons, X360_RB_MASK, X360_RB_ID);
  button2event(callback, &event, buttons, prevButtons, X360_GUIDE_MASK, X360_GUIDE_ID);

  button2event(callback, &event, buttons, prevButtons, X360_A_MASK, X360_A_ID);
  button2event(callback, &event, buttons, prevButtons, X360_B_MASK, X360_B_ID);
  button2event(callback, &event, buttons, prevButtons, X360_X_MASK, X360_X_ID);
  button2event(callback, &event, buttons, prevButtons, X360_Y_MASK, X360_Y_ID);

  /*
   * Axes
   */

  event.type = GE_JOYAXISMOTION;

  axis2event(callback, &event, x360_current->xaxis, x360_previous->xaxis, 0, X360_AXIS_X_ID);
  axis2event(callback, &event, x360_current->yaxis, x360_previous->yaxis, 1, X360_AXIS_Y_ID);
  axis2event(callback, &event, x360_current->zaxis, x360_previous->zaxis, 0, X360_AXIS_Z_ID);
  axis2event(callback, &event, x360_current->taxis, x360_previous->taxis, 1, X360_AXIS_RZ_ID);

  trigger2event(callback, &event, x360_current->ltrigger, x360_previous->ltrigger, X360_LT_ID);
  trigger2event(callback, &event, x360_current->rtrigger, x360_previous->rtrigger, X360_RT_ID);
}
