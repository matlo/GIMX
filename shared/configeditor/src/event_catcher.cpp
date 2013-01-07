/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "event_catcher.h"
#include <unistd.h>
#include <sstream>
#include <conversion.h>
#include <timer.h>
#include <GE.h>
#include <stdlib.h>

event_catcher::event_catcher()
{
    //ctor
}

event_catcher::~event_catcher()
{
    //dtor
}

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

int event_catcher::init()
{
    if(!GE_initialize())
    {
      return -1;
    }
    else
    {
      if(!m_DeviceType.empty())
      {
        string title = "Press a " + m_DeviceType + " button";
        GE_SetCaption(title.c_str(), title.c_str());
      }
      else
      {
        if(m_EventType == "button")
        {
          GE_SetCaption("Press a button...", "Press a button...");
        }
        else
        {
          GE_SetCaption("Move an axis...", "Move an axis...");
        }
      }
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

void event_catcher::run(string device_type, string event_type)
{
    GE_Event events[EVENT_BUFFER_SIZE];
    GE_Event* event;
    int num_evt;
    const char* event_id;

    m_DeviceType = device_type;
    m_DeviceId = "";
    m_DeviceName = "";
    m_EventType = event_type;
    m_EventId = "";

    init();

    GE_grab();

    done = 0;

    GE_SetCallback(GE_PushEvent);

    /*struct timespec period = {.tv_sec = 0, .tv_nsec = 10000000};

    timer_start(&period);*/

#ifdef WIN32
    /*
     * Purge events (launching the event catcher generates a mouse motion).
     */
    GE_PumpEvents();
    num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));
#endif

    while (!done)
    {
        GE_PumpEvents();
        num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));
        if (num_evt > 0)
        {
            for (event = events; event < events + num_evt; ++event)
            {
                switch (event->type)
                {
                case GE_KEYDOWN:
                    if(device_type != "" && device_type != "keyboard")
                    {
                        break;
                    }
                    if(event_type == "button")
                    {
                        m_DeviceType = "keyboard";
                        stringstream ss1;
                        ss1 << GE_KeyboardVirtualId(event->key.which);
                        m_DeviceId = ss1.str();
                        m_DeviceName = GE_KeyboardName(event->key.which);
                        m_EventType = "button";
                        event_id = get_chars_from_key(event->key.keysym);
                        m_EventId = event_id;
                        done = 1;
                    }
                    break;
                case GE_MOUSEBUTTONDOWN:
                    if(device_type != "" && device_type != "mouse")
                    {
                        break;
                    }
                    if(event_type == "button")
                    {
                        m_DeviceType = "mouse";
                        stringstream ss1;
                        ss1 << GE_MouseVirtualId(event->button.which);
                        m_DeviceId = ss1.str();
                        m_DeviceName = GE_MouseName(event->button.which);
                        m_EventType = "button";
                        event_id = get_chars_from_button(event->button.button);
                        m_EventId = event_id;
                        done = 1;
                    }
                    break;
                case GE_JOYBUTTONDOWN:
                    if(device_type != "" && device_type != "joystick")
                    {
                        break;
                    }
                    if(event_type == "button")
                    {
                        m_DeviceType = "joystick";
                        stringstream ss1;
                        ss1 << GE_JoystickVirtualId(event->jbutton.which);
                        m_DeviceId = ss1.str();
                        m_DeviceName = GE_JoystickName(event->jbutton.which);
                        m_EventType = "button";
                        int ib = event->jbutton.button;
                        stringstream ss2;
                        ss2 << ib;
                        m_EventId = ss2.str();
                        done = 1;
                    }
                    break;
                case GE_MOUSEMOTION:
                    if(device_type != "" && device_type != "mouse")
                    {
                        break;
                    }
                    if(event_type == "axis")
                    {
                        if(abs(event->motion.xrel) > 5 || abs(event->motion.yrel) > 5)
                        {
                            m_DeviceType = "mouse";
                            stringstream ss1;
                            ss1 << GE_MouseVirtualId(event->motion.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_MouseName(event->motion.which);
                            m_EventType = "axis";
                            done = 1;
                            if(abs(event->motion.xrel) > abs(event->motion.yrel))
                            {
                                m_EventId = "x";
                            }
                            else
                            {
                                m_EventId = "y";
                            }
                        }
                    }
                    else if(event_type == "axis up")
                    {
                        if(event->motion.xrel > 5 || event->motion.yrel > 5)
                        {
                            m_DeviceType = "mouse";
                            stringstream ss1;
                            ss1 << GE_MouseVirtualId(event->motion.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_MouseName(event->motion.which);
                            m_EventType = "axis";
                            done = 1;
                            if(event->motion.xrel > event->motion.yrel)
                            {
                                m_EventId = "x";
                            }
                            else
                            {
                                m_EventId = "y";
                            }
                        }
                    }
                    else if(event_type == "axis down")
                    {
                        if(event->motion.xrel < -5 || event->motion.yrel < -5)
                        {
                            m_DeviceType = "mouse";
                            stringstream ss1;
                            ss1 << GE_MouseVirtualId(event->motion.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_MouseName(event->motion.which);
                            m_EventType = "axis";
                            done = 1;
                            if(event->motion.xrel < event->motion.yrel)
                            {
                                m_EventId = "x";
                            }
                            else
                            {
                                m_EventId = "y";
                            }
                        }
                    }
                    break;
                case GE_JOYAXISMOTION:
                    if(device_type != "" && device_type != "joystick")
                    {
                        break;
                    }
                    /*
                     * Ugly patch for the sixaxis.
                     */
                    if(GE_IsSixaxis(event->jaxis.which) && event->jaxis.axis > 3)
                    {
                        event->jaxis.value = (event->jaxis.value + 32767) / 2;
                    }
                    if(GE_IsSixaxis(event->jaxis.which) && event->jaxis.axis >= 23)
                    {
                        break;
                    }
                    if(event_type == "axis")
                    {
                        if(abs(event->jaxis.value) > 10000)
                        {
                            m_DeviceType = "joystick";
                            stringstream ss1;
                            ss1 << GE_JoystickVirtualId(event->jaxis.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_JoystickName(event->jaxis.which);
                            m_EventType = "axis";
                            int ia = event->jaxis.axis;
                            stringstream ss2;
                            ss2 << ia;
                            m_EventId = ss2.str();
                            done = 1;
                        }
                    }
                    else if(event_type == "axis up")
                    {
                        if(event->jaxis.value > 10000)
                        {
                            m_DeviceType = "joystick";
                            stringstream ss1;
                            ss1 << GE_JoystickVirtualId(event->jaxis.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_JoystickName(event->jaxis.which);
                            m_EventType = "axis";
                            int ia = event->jaxis.axis;
                            stringstream ss2;
                            ss2 << ia;
                            m_EventId = ss2.str();
                            done = 1;
                        }
                    }
                    else if(event_type == "axis down")
                    {
                        if(event->jaxis.value < -10000)
                        {
                            m_DeviceType = "joystick";
                            stringstream ss1;
                            ss1 << GE_JoystickVirtualId(event->jaxis.which);
                            m_DeviceId = ss1.str();
                            m_DeviceName = GE_JoystickName(event->jaxis.which);
                            m_EventType = "axis";
                            int ia = event->jaxis.axis;
                            stringstream ss2;
                            ss2 << ia;
                            m_EventId = ss2.str();
                            done = 1;
                        }
                    }
                    break;
                }
            }
        }
#ifdef WIN32
        usleep(10000);
#endif
    }

    clean();
}
