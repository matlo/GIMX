/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TRIGGER_H
#define TRIGGER_H

#include "Device.h"
#include "Event.h"

class Trigger
{
    public:
        Trigger();
        Trigger(string dtype, string did, string dname, string eid, string switchback, string delay);
        virtual ~Trigger();
        Trigger(const Trigger& other);
        Trigger& operator=(const Trigger& other);
        bool operator==(const Trigger &other) const;
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        string GetSwitchBack() { return m_SwitchBack; }
        void SetSwitchBack(string val) { m_SwitchBack = val; }
        string GetDelay() { return m_Delay; }
        void SetDelay(string val) { m_Delay = val; }
    protected:
    private:
        Device m_Device;
        Event m_Event;
        string m_SwitchBack;
        string m_Delay;
};

#endif // TRIGGER_H
