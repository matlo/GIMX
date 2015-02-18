/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <report2event/ds42event.h>
#include <adapter.h>
#include <controller2.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

void ds42event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*))
{
  GE_Event event = { .jbutton.which = joystick_id };

  s_report_ds4* ds4_current = &current->ds4;
  s_report_ds4* ds4_previous = &previous->ds4;

  /*
   * Buttons
   */

  unsigned short value;

  unsigned char hatAndButtons = ds4_current->HatAndButtons;
  unsigned char prevHatAndButtons = ds4_previous->HatAndButtons;

  unsigned short buttonsAndCounter = ds4_current->ButtonsAndCounter;
  unsigned short prevButtonsAndCounter = ds4_previous->ButtonsAndCounter;

  unsigned char dirButtons = hatToButtons(hatAndButtons & 0x0F);
  unsigned char prevDirButtons = hatToButtons(prevHatAndButtons & 0x0F);

  if((value = (dirButtons & DS4_UP_MASK)) ^ (prevDirButtons & DS4_UP_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_UP_ID;
    callback(&event);
  }
  if((value = (dirButtons & DS4_RIGHT_MASK)) ^ (prevDirButtons & DS4_RIGHT_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_RIGHT_ID;
    callback(&event);
  }
  if((value = (dirButtons & DS4_DOWN_MASK)) ^ (prevDirButtons & DS4_DOWN_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_DOWN_ID;
    callback(&event);
  }
  if((value = (dirButtons & DS4_LEFT_MASK)) ^ (prevDirButtons & DS4_LEFT_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_LEFT_ID;
    callback(&event);
  }

  if((value = (hatAndButtons & DS4_SQUARE_MASK)) ^ (prevHatAndButtons & DS4_SQUARE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_SQUARE_ID;
    callback(&event);
  }
  if((value = (hatAndButtons & DS4_CROSS_MASK)) ^ (prevHatAndButtons & DS4_CROSS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_CROSS_ID;
    callback(&event);
  }
  if((value = (hatAndButtons & DS4_CIRCLE_MASK)) ^ (prevHatAndButtons & DS4_CIRCLE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_CIRCLE_ID;
    callback(&event);
  }
  if((value = (hatAndButtons & DS4_TRIANGLE_MASK)) ^ (prevHatAndButtons & DS4_TRIANGLE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_TRIANGLE_ID;
    callback(&event);
  }

  if((value = (buttonsAndCounter & DS4_L1_MASK)) ^ (prevButtonsAndCounter & DS4_L1_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L1_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R1_MASK)) ^ (prevButtonsAndCounter & DS4_R1_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R1_ID;
    callback(&event);
  }
#ifndef WIN32
  if((value = (buttonsAndCounter & DS4_L2_MASK)) ^ (prevButtonsAndCounter & DS4_L2_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L2_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R2_MASK)) ^ (prevButtonsAndCounter & DS4_R2_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R2_ID;
    callback(&event);
  }
#endif

  if((value = (buttonsAndCounter & DS4_SHARE_MASK)) ^ (prevButtonsAndCounter & DS4_SHARE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_SHARE_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_OPTIONS_MASK)) ^ (prevButtonsAndCounter & DS4_OPTIONS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_OPTIONS_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_L3_MASK)) ^ (prevButtonsAndCounter & DS4_L3_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L3_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R3_MASK)) ^ (prevButtonsAndCounter & DS4_R3_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R3_ID;
    callback(&event);
  }

  if((value = (buttonsAndCounter & DS4_PS_MASK)) ^ (prevButtonsAndCounter & DS4_PS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_PS_ID;
    callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_TOUCHPAD_MASK)) ^ (prevButtonsAndCounter & DS4_TOUCHPAD_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_TOUCHPAD_ID;
    callback(&event);
  }

  /*
   * Axes
   */

  event.type = GE_JOYAXISMOTION;

  int axisValue;
  int prevAxisValue;

  axisValue = ds4_current->X - CENTER_AXIS_VALUE_8BITS;
  prevAxisValue = ds4_previous->X - CENTER_AXIS_VALUE_8BITS;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_X_ID;
      axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / (MAX_AXIS_VALUE_8BITS/2);
      event.jaxis.value = clamp(-(MAX_AXIS_VALUE_16BITS/2+1), axisValue, (MAX_AXIS_VALUE_16BITS/2));
      callback(&event);
    }
  }
  axisValue = ds4_current->Y - CENTER_AXIS_VALUE_8BITS;
  prevAxisValue = ds4_previous->Y - CENTER_AXIS_VALUE_8BITS;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_Y_ID;
      axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / (MAX_AXIS_VALUE_8BITS/2);
      event.jaxis.value = clamp(-(MAX_AXIS_VALUE_16BITS/2+1), axisValue, (MAX_AXIS_VALUE_16BITS/2));
      callback(&event);
    }
  }
  axisValue = ds4_current->Z- CENTER_AXIS_VALUE_8BITS;
  prevAxisValue = ds4_previous->Z - CENTER_AXIS_VALUE_8BITS;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_Z_ID;
      axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / (MAX_AXIS_VALUE_8BITS/2);
      event.jaxis.value = clamp(-(MAX_AXIS_VALUE_16BITS/2+1), axisValue, (MAX_AXIS_VALUE_16BITS/2));
      callback(&event);
    }
  }
  axisValue = ds4_current->Rz - CENTER_AXIS_VALUE_8BITS;
  prevAxisValue = ds4_previous->Rz - CENTER_AXIS_VALUE_8BITS;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_RZ_ID;
      axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / (MAX_AXIS_VALUE_8BITS/2);
      event.jaxis.value = clamp(-(MAX_AXIS_VALUE_16BITS/2+1), axisValue, (MAX_AXIS_VALUE_16BITS/2));
      callback(&event);
    }
  }
  axisValue = ds4_current->Rx;
  prevAxisValue = ds4_previous->Rx;
  if(axisValue != prevAxisValue)
  {
    event.jaxis.axis = DS4_AXIS_L2_ID;
    axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / MAX_AXIS_VALUE_8BITS;
    event.jaxis.value = clamp(0, axisValue, MAX_AXIS_VALUE_16BITS/2);
    callback(&event);
  }
  axisValue = ds4_current->Ry;
  prevAxisValue = ds4_previous->Ry;
  if(axisValue != prevAxisValue)
  {
    event.jaxis.axis = DS4_AXIS_R2_ID;
    axisValue = axisValue * (MAX_AXIS_VALUE_16BITS/2) / MAX_AXIS_VALUE_8BITS;
    event.jaxis.value = clamp(0, axisValue, MAX_AXIS_VALUE_16BITS/2);
    callback(&event);
  }

  //TODO MLA: refactor this

  s_adapter* adapter = adapter_get(adapter_id);

  if(adapter->type == C_TYPE_DS4)
  {
    // battery level
    adapter->report.value.ds4.battery_level = ds4_current->battery_level;
    //we don't forward mic and phone state
    //as we don't support mic and phone
    adapter->report.value.ds4.ext = ds4_current->ext & 0x1F;

    // forward touchpad and motion sensing in bluetooth mode only
    if(adapter->bdaddr_dst)
    {
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
      prevFinger = &adapter->report.value.ds4.packet1.finger1;
      presence = &adapter->axis[ds4a_finger1];
      axis_x = &adapter->axis[ds4a_finger1_x];
      axis_y = &adapter->axis[ds4a_finger1_y];
      update_finger(finger, prevFinger, presence, axis_x, axis_y);

      if(*presence == 1)
      {
        send_command = 1;
      }

      finger = &ds4_current->packet1.finger2;
      prevFinger = &adapter->report.value.ds4.packet1.finger2;
      presence = &adapter->axis[ds4a_finger2];
      axis_x = &adapter->axis[ds4a_finger2_x];
      axis_y = &adapter->axis[ds4a_finger2_y];
      update_finger(finger, prevFinger, presence, axis_x, axis_y);

      if(*presence == 1)
      {
        send_command = 1;
      }

      /*
       * Motion sensing
       */

      /*
       * TODO MLA: make motion sensing updates send a command
       * without interfering with the inactivity timeout...
       */
      adapter->report.value.ds4._time = ds4_current->_time;
      adapter->report.value.ds4.motion_acc = ds4_current->motion_acc;
      adapter->report.value.ds4.motion_gyro = ds4_current->motion_gyro;

      // remember to send a report if the touchpad status changed
      if(send_command)
      {
        adapter->send_command = 1;
      }
    }
  }
}
