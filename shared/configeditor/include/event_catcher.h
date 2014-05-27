/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef EVENT_CATCHER_H
#define EVENT_CATCHER_H

#include <string>

using namespace std;

class event_catcher
{
    public:
        string GetDeviceType() { return m_DeviceType; }
        string GetDeviceName() { return m_DeviceName; }
        string GetDeviceId() { return m_DeviceId; }
        string GetEventType() { return m_EventType; }
        string GetEventId() { return m_EventId; }
        void SetDeviceType(string device_type) { m_DeviceType = device_type; }
        void SetDeviceName(string device_name) { m_DeviceName = device_name; }
        void SetDeviceId(string device_id) { m_DeviceId = device_id; }
        void SetEventType(string event_type) { m_EventType = event_type; }
        void SetEventId(string event_id) { m_EventId = event_id; }
        void SetDone() { done = 1; }
        int GetDone() { return done; }
        int init();
        void run(string device_type, string event_type);
        void clean();
        bool check_device(string device_type, string device_name, string device_id);
        static event_catcher* getInstance ()
        {
          if (NULL == _singleton)
          {
            _singleton =  new event_catcher;
          }

          return _singleton;
        }
    private:
        event_catcher();
        virtual ~event_catcher();
        string m_DeviceType;
        string m_DeviceName;
        string m_DeviceId;
        string m_EventType;
        string m_EventId;
        unsigned int done;

        static event_catcher* _singleton;
};

#endif // EVENT_CATCHER_H
