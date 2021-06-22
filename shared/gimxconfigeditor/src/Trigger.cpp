/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Trigger.h"

Trigger::Trigger()
{
    //ctor
}

Trigger::Trigger(string dtype, string did, string dname, string eid, string switchback, string delay):
    m_Device(dtype, did, dname),
    m_Event(eid),
    m_SwitchBack(switchback),
    m_Delay(delay)
{
    //ctor
}

Trigger::~Trigger()
{
    //dtor
}

Trigger::Trigger(const Trigger& other):
    m_Device(other.m_Device),
    m_Event(other.m_Event),
    m_SwitchBack(other.m_SwitchBack),
    m_Delay(other.m_Delay)
{
    //copy ctor
}

Trigger& Trigger::operator=(const Trigger& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_SwitchBack = rhs.m_SwitchBack;
    m_Delay = rhs.m_Delay;
    return *this;
}

bool Trigger::operator==(const Trigger &other) const
{
    return m_Device == other.m_Device
    && m_Event == other.m_Event
    && (m_SwitchBack == other.m_SwitchBack
            || (m_SwitchBack == "" && other.m_SwitchBack == "no")
            || (m_SwitchBack == "no" && other.m_SwitchBack == ""))
    && m_Delay == other.m_Delay;
}
