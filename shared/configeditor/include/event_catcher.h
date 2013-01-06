/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef EVENT_CATCHER_H
#define EVENT_CATCHER_H

#include <string>

using namespace std;

#define MAX_DEVICES 256

class event_catcher
{
    public:
        event_catcher();
        virtual ~event_catcher();
        string GetDeviceType() { return m_DeviceType; }
        string GetDeviceName() { return m_DeviceName; }
        string GetDeviceId() { return m_DeviceId; }
        string GetEventType() { return m_EventType; }
        string GetEventId() { return m_EventId; }
        int init();
        void run(string device_type, string event_type);
        void clean();
        bool check_device(string device_type, string device_name, string device_id);
    protected:
    private:
        string m_DeviceType;
        string m_DeviceName;
        string m_DeviceId;
        string m_EventType;
        string m_EventId;
        /*string joystickName[MAX_DEVICES];
        int joystickVirtualIndex[MAX_DEVICES];
        int joystickNbButton[MAX_DEVICES];
        int joystickSixaxis[MAX_DEVICES];
        string mouseName[MAX_DEVICES];
        int mouseVirtualIndex[MAX_DEVICES];
        string keyboardName[MAX_DEVICES];
        int keyboardVirtualIndex[MAX_DEVICES];*/
        unsigned int done;
};

#endif // EVENT_CATCHER_H
