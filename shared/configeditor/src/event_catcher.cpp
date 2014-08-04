/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "event_catcher.h"
#include <unistd.h>
#include <sstream>
#include <GE.h>
#include <stdlib.h>
#include <string.h>

#define PERIOD 10000//microseconds

event_catcher* event_catcher::_singleton = NULL;

event_catcher::event_catcher()
{
    //ctor
    wevents = false;
}

event_catcher::~event_catcher()
{
    //dtor
}

int event_catcher::init()
{
    unsigned char src = GE_MKB_SOURCE_PHYSICAL;
    
    if(wevents)
    {
      src = GE_MKB_SOURCE_WINDOW_SYSTEM;
    }
    
    if(!GE_initialize(src))
    {
      return -1;
    }

    return 0;
}

bool event_catcher::check_device(string device_type, string device_name, string device_id)
{
    int i = 0;
    int nb = 0;
    stringstream ss;
    int did;
    char* name;

    ss << device_id;
    ss >> did;

    if(device_type == "keyboard")
    {
      while((name = GE_KeyboardName(i)))
      {
        if(name == device_name)
        {
          if(nb == did)
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
        i++;
      }
    }
    else if(device_type == "mouse")
    {
      while((name = GE_MouseName(i)))
      {
        if(name == device_name)
        {
          if(nb == did)
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
        i++;
      }
    }
    else if(device_type == "joystick")
    {
      while((name = GE_JoystickName(i)))
      {
        if(name == device_name)
        {
          if(nb == did)
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
        i++;
      }
    }
    return false;
}

void event_catcher::clean()
{
    GE_quit();
}

typedef struct
{
  unsigned char which;
  unsigned char axis;
  short value;
} s_joystick_axis_first;

/*
 * This function records the first value for each axis of each joystick.
 * If a first value was already recorded, it returns the absolute difference
 * between the actual value and the first value.
 * If the first value is recorded, it returns 0.
 */
static int process_joystick_axis(s_joystick_axis_first* axis_first, unsigned int* axis_first_nb, GE_Event* event)
{
  int diff = 0;
  s_joystick_axis_first* last;
  for(last=axis_first; last<axis_first+*axis_first_nb; ++last)
  {
    if(event->jaxis.which == last->which
        && event->jaxis.axis == last->axis)
    {
      diff = abs(event->jaxis.value - last->value);
      break;
    }
  }
  if(last == axis_first+*axis_first_nb)
  {
    if(*axis_first_nb < EVENT_BUFFER_SIZE)
    {
      axis_first[*axis_first_nb].which = event->jaxis.which;
      axis_first[*axis_first_nb].axis = event->jaxis.axis;
      axis_first[*axis_first_nb].value = event->jaxis.value;
      ++(*axis_first_nb);
    }
  }
  return diff;
}

static unsigned int axis_first_nb;
static s_joystick_axis_first axis_first[EVENT_BUFFER_SIZE];

int process_event(GE_Event* event)
{
  const char* event_id;

  event_catcher* evcatch = event_catcher::getInstance();

  if(evcatch->GetDone())
  {
    return 0;
  }

  switch (event->type)
  {
  case GE_KEYDOWN:
      if(evcatch->GetDeviceType() != "" && evcatch->GetDeviceType() != "keyboard")
      {
          break;
      }
      if(evcatch->GetEventType() == "button")
      {
          evcatch->SetDeviceType("keyboard");
          stringstream ss1;
          ss1 << GE_KeyboardVirtualId(event->key.which);
          evcatch->SetDeviceId(ss1.str());
          evcatch->SetDeviceName(GE_KeyboardName(event->key.which));
          evcatch->SetEventType("button");
          event_id = GE_KeyName(event->key.keysym);
          evcatch->SetEventId(event_id);
          evcatch->SetDone();
      }
      break;
  case GE_MOUSEBUTTONDOWN:
      if(evcatch->GetDeviceType() != "" && evcatch->GetDeviceType() != "mouse")
      {
          break;
      }
      if(evcatch->GetEventType() == "button")
      {
          evcatch->SetDeviceType("mouse");
          stringstream ss1;
          ss1 << GE_MouseVirtualId(event->button.which);
          evcatch->SetDeviceId(ss1.str());
          evcatch->SetDeviceName(GE_MouseName(event->button.which));
          evcatch->SetEventType("button");
          event_id = GE_MouseButtonName(event->button.button);
          evcatch->SetEventId(event_id);
          evcatch->SetDone();
      }
      break;
  case GE_JOYBUTTONDOWN:
      if(evcatch->GetDeviceType() != "" && evcatch->GetDeviceType() != "joystick")
      {
          break;
      }
      if(evcatch->GetEventType() == "button")
      {
          evcatch->SetDeviceType("joystick");
          stringstream ss1;
          ss1 << GE_JoystickVirtualId(event->jbutton.which);
          evcatch->SetDeviceId(ss1.str());
          evcatch->SetDeviceName(GE_JoystickName(event->jbutton.which));
          evcatch->SetEventType("button");
          int ib = event->jbutton.button;
          stringstream ss2;
          ss2 << ib;
          evcatch->SetEventId(ss2.str());
          evcatch->SetDone();
      }
      break;
  case GE_MOUSEMOTION:
      if(evcatch->GetDeviceType() != "" && evcatch->GetDeviceType() != "mouse")
      {
          break;
      }
      if(evcatch->GetEventType() == "axis")
      {
          if(abs(event->motion.xrel) > 5 || abs(event->motion.yrel) > 5)
          {
              evcatch->SetDeviceType("mouse");
              stringstream ss1;
              ss1 << GE_MouseVirtualId(event->motion.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_MouseName(event->motion.which));
              evcatch->SetEventType("axis");
              evcatch->SetDone();
              if(abs(event->motion.xrel) > abs(event->motion.yrel))
              {
                  evcatch->SetEventId("x");
              }
              else
              {
                  evcatch->SetEventId("y");
              }
          }
      }
      else if(evcatch->GetEventType() == "axis up")
      {
          if(event->motion.xrel > 5 || event->motion.yrel > 5)
          {
              evcatch->SetDeviceType("mouse");
              stringstream ss1;
              ss1 << GE_MouseVirtualId(event->motion.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_MouseName(event->motion.which));
              evcatch->SetEventType("axis");
              evcatch->SetDone();
              if(event->motion.xrel > event->motion.yrel)
              {
                  evcatch->SetEventId("x");
              }
              else
              {
                  evcatch->SetEventId("y");
              }
          }
      }
      else if(evcatch->GetEventType() == "axis down")
      {
          if(event->motion.xrel < -5 || event->motion.yrel < -5)
          {
              evcatch->SetDeviceType("mouse");
              stringstream ss1;
              ss1 << GE_MouseVirtualId(event->motion.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_MouseName(event->motion.which));
              evcatch->SetEventType("axis");
              evcatch->SetDone();
              if(event->motion.xrel < event->motion.yrel)
              {
                  evcatch->SetEventId("x");
              }
              else
              {
                  evcatch->SetEventId("y");
              }
          }
      }
      break;
  case GE_JOYAXISMOTION:
      if(evcatch->GetDeviceType() != "" && evcatch->GetDeviceType() != "joystick")
      {
          break;
      }
      if(evcatch->GetEventType() == "axis")
      {
          if(process_joystick_axis(axis_first, &axis_first_nb, event) > 1000)
          {
              evcatch->SetDeviceType("joystick");
              stringstream ss1;
              ss1 << GE_JoystickVirtualId(event->jaxis.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_JoystickName(event->jaxis.which));
              evcatch->SetEventType("axis");
              int ia = event->jaxis.axis;
              stringstream ss2;
              ss2 << ia;
              evcatch->SetEventId(ss2.str());
              evcatch->SetDone();
          }
      }
      else if(evcatch->GetEventType() == "axis up")
      {
          if(event->jaxis.value > 10000)
          {
              evcatch->SetDeviceType("joystick");
              stringstream ss1;
              ss1 << GE_JoystickVirtualId(event->jaxis.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_JoystickName(event->jaxis.which));
              evcatch->SetEventType("axis");
              int ia = event->jaxis.axis;
              stringstream ss2;
              ss2 << ia;
              evcatch->SetEventId(ss2.str());
              evcatch->SetDone();
          }
      }
      else if(evcatch->GetEventType() == "axis down")
      {
          if(event->jaxis.value < -10000)
          {
              evcatch->SetDeviceType("joystick");
              stringstream ss1;
              ss1 << GE_JoystickVirtualId(event->jaxis.which);
              evcatch->SetDeviceId(ss1.str());
              evcatch->SetDeviceName(GE_JoystickName(event->jaxis.which));
              evcatch->SetEventType("axis");
              int ia = event->jaxis.axis;
              stringstream ss2;
              ss2 << ia;
              evcatch->SetEventId(ss2.str());
              evcatch->SetDone();
          }
      }
      break;
  }

  return 0;
}

void event_catcher::run(string device_type, string event_type)
{
    axis_first_nb = 0;
    memset(axis_first, 0x00, sizeof(axis_first));

    m_DeviceType = device_type;
    m_DeviceId = "";
    m_DeviceName = "";
    m_EventType = event_type;
    m_EventId = "";

    init();

    GE_grab();

    done = 0;

    GE_TimerStart(PERIOD);

    GE_SetCallback(process_event);
	
    while (!done)
    {
        GE_PumpEvents();
    }

    GE_TimerClose();

    clean();
}
