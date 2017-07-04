/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef EVENT_CATCHER_H
#define EVENT_CATCHER_H

#include <string>
#include <vector>
#include "Device.h"
#include "Event.h"

using namespace std;

class EventCatcher
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
        static EventCatcher* getInstance ()
        {
          if (NULL == _singleton)
          {
            _singleton =  new EventCatcher;
          }

          return _singleton;
        }
        bool hasJoystick();
        bool hasMouse();
        bool hasKeyboard();
        pair<int, int> getAxisRange(string name, string id, string axis);

        int calibrate(int which, int axis, int value);

    private:
        EventCatcher();
        virtual ~EventCatcher();
        int init(bool calibrate);
        vector<pair<Device, Event> > m_Events;
        string m_DeviceType;
        string m_EventType;
        unsigned int done;
        int stopTimer;
        bool wevents;

        string device_name;
        string device_id;
        string event_id;
        int min_value;
        int max_value;
        int last_value;

        static EventCatcher* _singleton;
};

#endif // EVENT_CATCHER_H
