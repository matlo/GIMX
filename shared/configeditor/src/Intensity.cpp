/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Intensity.h"

Intensity::Intensity()
{
    //ctor
    m_dead_zone = 0;
    m_steps = 0;
}

Intensity::Intensity(string control,
    string dtype1, string dname1, string did1, string eid1,
    string direction, unsigned char dead_zone, string shape, unsigned char steps):
    m_Control(control),
    m_Device(dtype1, did1, dname1), m_Event(eid1),
    m_direction(direction),
    m_dead_zone(dead_zone),
    m_shape(shape),
    m_steps(steps)
{
    //ctor
}

Intensity::~Intensity()
{
    //dtor
}

Intensity::Intensity(const Intensity& other):
    m_Control(other.m_Control),
    m_Device(other.m_Device), m_Event(other.m_Event),
    m_direction(other.m_direction),
    m_dead_zone(other.m_dead_zone),
    m_shape(other.m_shape),
    m_steps(other.m_steps)
{
    //copy ctor
}

Intensity& Intensity::operator=(const Intensity& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_direction = rhs.m_direction;
    m_Control = rhs.m_Control;
    m_dead_zone = rhs.m_dead_zone;
    m_shape = rhs.m_shape;
    m_steps = rhs.m_steps;
    return *this;
}

bool Intensity::operator==(const Intensity &other) const
{
    return m_Device == other.m_Device
    && m_Event == other.m_Event
    && m_direction == other.m_direction
    && m_Control == other.m_Control
    && m_dead_zone == other.m_dead_zone
    && m_shape == other.m_shape
    && m_steps == other.m_steps;
}
