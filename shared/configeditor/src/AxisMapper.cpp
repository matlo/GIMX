/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "AxisMapper.h"

AxisMapper::AxisMapper()
{
    //ctor
}

AxisMapper::AxisMapper(string dtype, string did, string dname, string etype, string eid, string axis, string deadZone, string multiplier, string exponent, string shape, string buffersize, string filter, string label):
m_Device(dtype, did, dname), m_Event(etype, eid, deadZone, multiplier, exponent, shape, buffersize, filter), m_Axis(axis), m_Label(label)
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

AxisMapper& AxisMapper::operator=(const AxisMapper& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_Axis = rhs.m_Axis;
    m_Label = rhs.m_Label;
    return *this;
}
