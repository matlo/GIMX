/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <report2event/ds42event.h>
#include <controller.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <gimxcontroller/include/controller.h>

#define STICK_THRESHOLD 8

static inline unsigned char hatToButtons(const unsigned char hat)
{
  unsigned char buttons = 0x00;
  switch(hat)
  {
    case 0x00:
      buttons = DS4_UP_MASK;
      break;
    case 0x01:
      buttons = DS4_UP_MASK | DS4_RIGHT_MASK;
      break;
    case 0x02:
      buttons = DS4_RIGHT_MASK;
      break;
    case 0x03:
      buttons = DS4_DOWN_MASK | DS4_RIGHT_MASK;
      break;
    case 0x04:
      buttons = DS4_DOWN_MASK;
      break;
    case 0x05:
      buttons = DS4_DOWN_MASK | DS4_LEFT_MASK;
      break;
    case 0x06:
      buttons = DS4_LEFT_MASK;
      break;
    case 0x07:
      buttons = DS4_UP_MASK | DS4_LEFT_MASK;
      break;
    case 0x08:
      break;
  }
  return buttons;
}

static inline void update_finger(s_trackpad_finger* current, s_trackpad_finger* previous, int* presence, int* axis_x, int* axis_y)
{
  unsigned char* coords = current->coords;
  unsigned char* prevCoords = previous->coords;

  if(current->id & 0x80)
  {
    /*
     * TODO MLA: make a better fix for #286
     */
    if(*presence == 1)
    {
      *presence = 0;
    }
  }
  else
  {
    *presence = 1;
    unsigned short prev_x;
    unsigned short prev_y;
    if(previous->id & 0x80)
    {
      //new touch: relative to center
      prev_x = DS4_TRACKPAD_MAX_X/2;
      prev_y = DS4_TRACKPAD_MAX_Y/2;
    }
    else
    {
      //relative to last position
      prev_x = ((prevCoords[1] & 0x0F) << 8) | prevCoords[0];
      prev_y = prevCoords[2] << 4 | (prevCoords[1]>>4);
    }
    *axis_x = (((coords[1] & 0x0F) << 8) | coords[0]) - prev_x;
    *axis_y = (coords[2] << 4 | (coords[1]>>4)) - prev_y;
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

static inline void axis2event(int (*callback)(GE_Event*), GE_Event* event, unsigned char axis,
    unsigned char paxis, uint8_t axis_id)
{
  int axisValue;
  int prevAxisValue;

  axisValue = axis - CENTER_AXIS_VALUE_8BITS;
  prevAxisValue = paxis - CENTER_AXIS_VALUE_8BITS;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event->jaxis.axis = axis_id;
      axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / (MAX_AXIS_VALUE_8BITS/2);
      event->jaxis.value = clamp(SHRT_MIN, axisValue, SHRT_MAX);
      callback(event);
    }
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

void ds42event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*))
{
  GE_Event event = { .jbutton = { .which = joystick_id } };

  s_report_ds4* ds4_current = &current->ds4;
  s_report_ds4* ds4_previous = &previous->ds4;

  /*
   * Buttons
   */

  unsigned char hatAndButtons = ds4_current->HatAndButtons;
  unsigned char prevHatAndButtons = ds4_previous->HatAndButtons;

  unsigned short buttonsAndCounter = ds4_current->ButtonsAndCounter;
  unsigned short prevButtonsAndCounter = ds4_previous->ButtonsAndCounter;

  unsigned char dirButtons = hatToButtons(hatAndButtons & 0x0F);
  unsigned char prevDirButtons = hatToButtons(prevHatAndButtons & 0x0F);

  button2event(callback, &event, dirButtons, prevDirButtons, DS4_UP_MASK, DS4_UP_ID);
  button2event(callback, &event, dirButtons, prevDirButtons, DS4_RIGHT_MASK, DS4_RIGHT_ID);
  button2event(callback, &event, dirButtons, prevDirButtons, DS4_DOWN_MASK, DS4_DOWN_ID);
  button2event(callback, &event, dirButtons, prevDirButtons, DS4_LEFT_MASK, DS4_LEFT_ID);

  button2event(callback, &event, hatAndButtons, prevHatAndButtons, DS4_SQUARE_MASK, DS4_SQUARE_ID);
  button2event(callback, &event, hatAndButtons, prevHatAndButtons, DS4_CROSS_MASK, DS4_CROSS_ID);
  button2event(callback, &event, hatAndButtons, prevHatAndButtons, DS4_CIRCLE_MASK, DS4_CIRCLE_ID);
  button2event(callback, &event, hatAndButtons, prevHatAndButtons, DS4_TRIANGLE_MASK, DS4_TRIANGLE_ID);

  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_L1_MASK, DS4_L1_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_R1_MASK, DS4_R1_ID);
#ifndef WIN32
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_L2_MASK, DS4_L2_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_R2_MASK, DS4_R2_ID);
#endif

  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_SHARE_MASK, DS4_SHARE_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_OPTIONS_MASK, DS4_OPTIONS_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_L3_MASK, DS4_L3_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_R3_MASK, DS4_R3_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_PS_MASK, DS4_PS_ID);
  button2event(callback, &event, buttonsAndCounter, prevButtonsAndCounter, DS4_TOUCHPAD_MASK, DS4_TOUCHPAD_ID);

  /*
   * Axes
   */

  event.type = GE_JOYAXISMOTION;

  axis2event(callback, &event, ds4_current->X, ds4_previous->X, DS4_AXIS_X_ID);
  axis2event(callback, &event, ds4_current->Y, ds4_previous->Y, DS4_AXIS_Y_ID);
  axis2event(callback, &event, ds4_current->Z, ds4_previous->Z, DS4_AXIS_Z_ID);
  axis2event(callback, &event, ds4_current->Rz, ds4_previous->Rz, DS4_AXIS_RZ_ID);

  trigger2event(callback, &event, ds4_current->Rx, ds4_previous->Rx, DS4_AXIS_L2_ID);
  trigger2event(callback, &event, ds4_current->Ry, ds4_previous->Ry, DS4_AXIS_R2_ID);

  //TODO MLA: refactor this

  s_adapter* adapter = adapter_get(adapter_id);

  if(adapter->ctype == C_TYPE_DS4)
  {
    // battery level
    adapter->report[0].value.ds4.battery_level = ds4_current->battery_level;
    // we don't forward mic and phone state
    // as we don't support mic and phone
    adapter->report[0].value.ds4.ext = ds4_current->ext & 0x1F;

    /*
     * Touchpad
     *
     * The touchpad does not generate joystick events.
     * => wrap the touchpad directly to the emulated touchpad.
     */

    int send_command = 0;

    s_trackpad_finger* finger;
    s_trackpad_finger* prevFinger;
    int* presence;
    int* axis_x;
    int* axis_y;

    finger = &ds4_current->packet1.finger1;
    prevFinger = &adapter->report[0].value.ds4.packet1.finger1;
    presence = &adapter->axis[ds4a_finger1];
    axis_x = &adapter->axis[ds4a_finger1_x];
    axis_y = &adapter->axis[ds4a_finger1_y];
    update_finger(finger, prevFinger, presence, axis_x, axis_y);

    if(*presence == 1)
    {
      send_command = 1;
    }

    finger = &ds4_current->packet1.finger2;
    prevFinger = &adapter->report[0].value.ds4.packet1.finger2;
    presence = &adapter->axis[ds4a_finger2];
    axis_x = &adapter->axis[ds4a_finger2_x];
    axis_y = &adapter->axis[ds4a_finger2_y];
    update_finger(finger, prevFinger, presence, axis_x, axis_y);

    if(*presence == 1)
    {
      send_command = 1;
    }

    // forward motion sensing in bluetooth mode only
    if(adapter->bt.bdaddr_dst)
    {
      /*
       * Motion sensing
       */

      /*
       * TODO MLA: make motion sensing updates send a command
       * without interfering with the inactivity timeout...
       */
      adapter->report[0].value.ds4._time = ds4_current->_time;
      adapter->report[0].value.ds4.motion_acc = ds4_current->motion_acc;
      adapter->report[0].value.ds4.motion_gyro = ds4_current->motion_gyro;
    }

    // remember to send a report if the touchpad status changed
    if(send_command)
    {
      adapter->send_command = 1;
    }
  }
}
