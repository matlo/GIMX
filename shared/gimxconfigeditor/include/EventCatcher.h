/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
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
    private:
        EventCatcher();
        virtual ~EventCatcher();
        vector<pair<Device, Event> > m_Events;
        string m_DeviceType;
        string m_EventType;
        unsigned int done;
        int stopTimer;
        bool wevents;

        static EventCatcher* _singleton;
};

#endif // EVENT_CATCHER_H
