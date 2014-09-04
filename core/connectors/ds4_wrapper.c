/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <ds4_wrapper.h>
#include <stddef.h>
#include <stdlib.h>

#define STICK_THRESHOLD 4

static int (*event_callback)(GE_Event*) = NULL;

void ds4_wrapper_set_event_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

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

void ds4_wrapper(s_report_ds4* current, s_report_ds4* previous, int device_id)
{
  GE_Event event = { .jbutton.which = device_id };

  unsigned short value;

  unsigned char hatAndButtons = current->HatAndButtons;
  unsigned char prevHatAndButtons = previous->HatAndButtons;

  unsigned short buttonsAndCounter = current->ButtonsAndCounter;
  unsigned short prevButtonsAndCounter = previous->ButtonsAndCounter;

  unsigned char dirButtons = hatToButtons(hatAndButtons & 0x0F);
  unsigned char prevDirButtons = hatToButtons(prevHatAndButtons & 0x0F);

  if((value = (dirButtons & DS4_UP_MASK)) ^ (prevDirButtons & DS4_UP_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_UP_ID;
    event_callback(&event);
  }
  if((value = (dirButtons & DS4_RIGHT_MASK)) ^ (prevDirButtons & DS4_RIGHT_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_RIGHT_ID;
    event_callback(&event);
  }
  if((value = (dirButtons & DS4_DOWN_MASK)) ^ (prevDirButtons & DS4_DOWN_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_DOWN_ID;
    event_callback(&event);
  }
  if((value = (dirButtons & DS4_LEFT_MASK)) ^ (prevDirButtons & DS4_LEFT_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_LEFT_ID;
    event_callback(&event);
  }

  if((value = (hatAndButtons & DS4_SQUARE_MASK)) ^ (prevHatAndButtons & DS4_SQUARE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_SQUARE_ID;
    event_callback(&event);
  }
  if((value = (hatAndButtons & DS4_CROSS_MASK)) ^ (prevHatAndButtons & DS4_CROSS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_CROSS_ID;
    event_callback(&event);
  }
  if((value = (hatAndButtons & DS4_CIRCLE_MASK)) ^ (prevHatAndButtons & DS4_CIRCLE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_CIRCLE_ID;
    event_callback(&event);
  }
  if((value = (hatAndButtons & DS4_TRIANGLE_MASK)) ^ (prevHatAndButtons & DS4_TRIANGLE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_TRIANGLE_ID;
    event_callback(&event);
  }

  if((value = (buttonsAndCounter & DS4_L1_MASK)) ^ (prevButtonsAndCounter & DS4_L1_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L1_ID;
    event_callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R1_MASK)) ^ (prevButtonsAndCounter & DS4_R1_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R1_ID;
    event_callback(&event);
  }
#ifndef WIN32
  if((value = (buttonsAndCounter & DS4_L2_MASK)) ^ (prevButtonsAndCounter & DS4_L2_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L2_ID;
    event_callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R2_MASK)) ^ (prevButtonsAndCounter & DS4_R2_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R2_ID;
    event_callback(&event);
  }
#endif

  if((value = (buttonsAndCounter & DS4_SHARE_MASK)) ^ (prevButtonsAndCounter & DS4_SHARE_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_SHARE_ID;
    event_callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_OPTIONS_MASK)) ^ (prevButtonsAndCounter & DS4_OPTIONS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_OPTIONS_ID;
    event_callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_L3_MASK)) ^ (prevButtonsAndCounter & DS4_L3_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_L3_ID;
    event_callback(&event);
  }
  if((value = (buttonsAndCounter & DS4_R3_MASK)) ^ (prevButtonsAndCounter & DS4_R3_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_R3_ID;
    event_callback(&event);
  }

  if((value = (buttonsAndCounter & DS4_PS_MASK)) ^ (prevButtonsAndCounter & DS4_PS_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_PS_ID;
    event_callback(&event);
  }
#ifndef WIN32
  if((value = (buttonsAndCounter & DS4_TOUCHPAD_MASK)) ^ (prevButtonsAndCounter & DS4_TOUCHPAD_MASK))
  {
    event.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    event.jbutton.button = DS4_TOUCHPAD_ID;
    event_callback(&event);
  }
#endif

  event.type = GE_JOYAXISMOTION;

  int axisValue;
  int prevAxisValue;

  axisValue = current->X - 0x80;
  prevAxisValue = previous->X - 0x80;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_X_ID;
      axisValue = axisValue * 32765 / 127;
      event.jaxis.value = clamp(-32768, axisValue, 32767);
      event_callback(&event);
    }
  }
  axisValue = current->Y - 0x80;
  prevAxisValue = previous->Y - 0x80;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_Y_ID;
      axisValue = axisValue * 32765 / 127;
      event.jaxis.value = clamp(-32768, axisValue, 32767);
      event_callback(&event);
    }
  }
  axisValue = current->Z- 0x80;
  prevAxisValue = previous->Z - 0x80;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_Z_ID;
      axisValue = axisValue * 32765 / 127;
      event.jaxis.value = clamp(-32768, axisValue, 32767);
      event_callback(&event);
    }
  }
  axisValue = current->Rz - 0x80;
  prevAxisValue = previous->Rz - 0x80;
  if(axisValue != prevAxisValue)
  {
    if(abs(axisValue) > STICK_THRESHOLD || abs(prevAxisValue) > STICK_THRESHOLD)
    {
      event.jaxis.axis = DS4_AXIS_RZ_ID;
      axisValue = axisValue * 32765 / 127;
      event.jaxis.value = clamp(-32768, axisValue, 32767);
      event_callback(&event);
    }
  }
  axisValue = current->Rx;
  prevAxisValue = previous->Rx;
  if(axisValue != prevAxisValue)
  {
    event.jaxis.axis = DS4_AXIS_L2_ID;
    axisValue = (axisValue - 0x80) * 32765 / 127;
    event.jaxis.value = clamp(-32768, axisValue, 32767);
    event_callback(&event);
  }
  axisValue = current->Ry;
  prevAxisValue = previous->Ry;
  if(axisValue || prevAxisValue)
  {
    event.jaxis.axis = DS4_AXIS_R2_ID;
    axisValue = (axisValue - 0x80) * 32765 / 127;
    event.jaxis.value = clamp(-32768, axisValue, 32767);
    event_callback(&event);
  }

  //TODO: touchpad + motion sensing
}
