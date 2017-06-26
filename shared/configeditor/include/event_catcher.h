/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef EVENT_CATCHER_H
#define EVENT_CATCHER_H

#include <string>
#include <vector>
#include "Device.h"
#include "Event.h"

using namespace std;

class event_catcher
{
    public:
        string GetDeviceType() { return m_DeviceType; }
        string GetEventType() { return m_EventType; }
        void SetDone() { done = 1; }
        int GetDone() { return done; }
        void SetWindowEvents(bool value) { wevents = value; }
        void StartTimer();
        int init();
        void run(string device_type, string event_type);
        void clean();
        bool check_device(string device_type, string device_name, string device_id);
        void AddEvent(Device device, Event event);
        vector<pair<Device, Event> > * GetEvents() { return &m_Events; }
        static event_catcher* getInstance ()
        {
          if (NULL == _singleton)
          {
            _singleton =  new event_catcher;
          }

          return _singleton;
        }
        bool hasJoystick();
        bool hasMouse();
        bool hasKeyboard();
        pair<int, int> getAxisRange(string name, string id, string axis);

        int calibrate(int which, int axis, int value);

    private:
        event_catcher();
        virtual ~event_catcher();
        vector<pair<Device, Event> > m_Events;
        string m_DeviceType;
        string m_EventType;
        unsigned int done;
        int stopTimer;
        bool wevents;

        bool calibrating;
        string device_name;
        string device_id;
        string event_id;
        int min_value;
        int max_value;
        int last_value;

        static event_catcher* _singleton;
};

#endif // EVENT_CATCHER_H
