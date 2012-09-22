/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "ButtonMapper.h"

ButtonMapper::ButtonMapper()
{
    //ctor
}

ButtonMapper::ButtonMapper(string dtype, string did, string dname, string etype, string eid, string threshold, string button, string label):
m_Device(dtype, did, dname), m_Event(etype, eid, threshold), m_Button(button), m_Label(label)
{
    //ctor
}

ButtonMapper::~ButtonMapper()
{
    //dtor
}

ButtonMapper::ButtonMapper(const ButtonMapper& other):
m_Device(other.m_Device), m_Event(other.m_Event), m_Button(other.m_Button), m_Label(other.m_Label)
{
    //copy ctor
}

ButtonMapper& ButtonMapper::operator=(const ButtonMapper& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_Button = rhs.m_Button;
    m_Label = rhs.m_Label;
    return *this;
}
