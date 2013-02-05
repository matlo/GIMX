/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "AxisMapper.h"

AxisMapper::AxisMapper()
{
    //ctor
}

AxisMapper::AxisMapper(string dtype, string did, string dname, string etype, string eid, string axis, string deadZone, string multiplier, string exponent, string shape, string label):
m_Device(dtype, did, dname), m_Event(etype, eid, deadZone, multiplier, exponent, shape), m_Axis(axis), m_Label(label)
{
    //ctor
}

AxisMapper::~AxisMapper()
{
    //dtor
}

AxisMapper::AxisMapper(const AxisMapper& other):
m_Device(other.m_Device), m_Event(other.m_Event), m_Axis(other.m_Axis), m_Label(other.m_Label)
{
    //copy ctor
}

AxisMapper& AxisMapper::operator=(const AxisMapper& other)
{
    if (this == &other) return *this; // handle self assignment
    m_Device = other.m_Device;
    m_Event = other.m_Event;
    m_Axis = other.m_Axis;
    m_Label = other.m_Label;
    return *this;
}

bool AxisMapper::operator==(const AxisMapper &other) const
{
    return m_Device == other.m_Device
    && m_Event == other.m_Event
    && m_Axis == other.m_Axis
    && m_Label == other.m_Label;
}
