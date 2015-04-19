/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <report2event/xOnePad2event.h>
#include <controller2.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static inline void axis2event(int (*callback)(GE_Event*), GE_Event* event,
    short axis, short paxis, char invert, uint8_t axis_id)
{
  if (axis != paxis)
  {
    event->jaxis.axis = axis_id;
    event->jaxis.value = invert ? ~axis : axis;
    callback(event);
  }
}

static inline void trigger2event(int (*callback)(GE_Event*), GE_Event* event, unsigned short trigger,
    unsigned short ptrigger, uint8_t axis_id)
{
  int axisValue;
  if (trigger != ptrigger)
  {
    event->jaxis.axis = axis_id;
    axisValue = trigger * (MAX_AXIS_VALUE_16BITS / 2) / MAX_AXIS_VALUE_10BITS;
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

void xOnePad2event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*))
{
  GE_Event event = { .jbutton.which = joystick_id };

  if(current->xone.input.type == XONE_USB_HID_IN_REPORT_ID)
  {
    s_report_xone* xone_current = &current->xone;
    s_report_xone* xone_previous = &previous->xone;

    /*
     * Buttons
     */

    unsigned short buttons = xone_current->input.buttons;
    unsigned short prevButtons = xone_previous->input.buttons;

    button2event(callback, &event, buttons, prevButtons, XONE_UP_MASK, X360_UP_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_DOWN_MASK, X360_DOWN_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_LEFT_MASK, X360_LEFT_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_RIGHT_MASK, X360_RIGHT_ID);

    button2event(callback, &event, buttons, prevButtons, XONE_MENU_MASK, X360_START_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_VIEW_MASK, X360_BACK_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_LS_MASK, X360_LS_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_RS_MASK, X360_RS_ID);

    button2event(callback, &event, buttons, prevButtons, XONE_LB_MASK, X360_LB_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_RB_MASK, X360_RB_ID);

    button2event(callback, &event, buttons, prevButtons, XONE_A_MASK, X360_A_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_B_MASK, X360_B_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_X_MASK, X360_X_ID);
    button2event(callback, &event, buttons, prevButtons, XONE_Y_MASK, X360_Y_ID);

    /*
     * Axes
     */

    event.type = GE_JOYAXISMOTION;

    axis2event(callback, &event, xone_current->input.xaxis, xone_previous->input.xaxis, 0, X360_AXIS_X_ID);
    axis2event(callback, &event, xone_current->input.yaxis, xone_previous->input.yaxis, 1, X360_AXIS_Y_ID);
    axis2event(callback, &event, xone_current->input.zaxis, xone_previous->input.zaxis, 0, X360_AXIS_Z_ID);
    axis2event(callback, &event, xone_current->input.taxis, xone_previous->input.taxis, 1, X360_AXIS_RZ_ID);

    trigger2event(callback, &event, xone_current->input.ltrigger, xone_previous->input.ltrigger, X360_LT_ID);
    trigger2event(callback, &event, xone_current->input.rtrigger, xone_previous->input.rtrigger, X360_RT_ID);
  }
  else if(current->xone.input.type == XONE_USB_HID_IN_GUIDE_REPORT_ID)
  {
    s_report_xone* xone_current = &current->xone;
    s_report_xone* xone_previous = &previous->xone;

    unsigned short buttons = xone_current->guide.button;
    unsigned short prevButtons = xone_previous->guide.button;

    button2event(callback, &event, buttons, prevButtons, XONE_GUIDE_MASK, X360_GUIDE_ID);
  }
}
