/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Event.h"

Event::Event():
m_Type(""), m_Id(""), m_Threshold(""), m_DeadZone(""), m_Multiplier(""), m_Exponent(""), m_Shape("")
{
    //ctor
}

Event::Event(string type, string id, string threshold, string deadZone, string multiplier, string exponent, string shape):
m_Type(type), m_Id(id), m_Threshold(threshold), m_DeadZone(deadZone), m_Multiplier(multiplier), m_Exponent(exponent), m_Shape(shape)
{
    //ctor
}

Event::Event(string type, string id, string threshold):
m_Type(type), m_Id(id), m_Threshold(threshold), m_DeadZone(""), m_Multiplier(""), m_Exponent(""), m_Shape("")
{
    //ctor
}

Event::Event(string type, string id):
m_Type(type), m_Id(id), m_Threshold(""), m_DeadZone(""), m_Multiplier(""), m_Exponent(""), m_Shape("")
{
    //ctor
}

Event::Event(string type, string id, string deadZone, string multiplier, string exponent, string shape):
m_Type(type), m_Id(id), m_Threshold(""), m_DeadZone(deadZone), m_Multiplier(multiplier), m_Exponent(exponent), m_Shape(shape)
{
    //ctor
}

Event::Event(string id):
m_Type(""), m_Id(id), m_Threshold(""), m_DeadZone(""), m_Multiplier(""), m_Exponent(""), m_Shape("")
{
    //ctor
}

Event::~Event()
{
    //dtor
}

Event::Event(const Event& other):
m_Type(other.m_Type), m_Id(other.m_Id), m_Threshold(other.m_Threshold), m_DeadZone(other.m_DeadZone), m_Multiplier(other.m_Multiplier), m_Exponent(other.m_Exponent), m_Shape(other.m_Shape)
{
    //copy ctor
}

Event& Event::operator=(const Event& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Type = rhs.m_Type;
    m_Id = rhs.m_Id;
    m_Threshold = rhs.m_Threshold;
    m_DeadZone = rhs.m_DeadZone;
    m_Multiplier = rhs.m_Multiplier;
    m_Exponent = rhs.m_Exponent;
    m_Shape = rhs.m_Shape;
    return *this;
}

bool Event::operator==(const Event &other) const
{
    return m_Type == other.m_Type
    && m_Id == other.m_Id
    && m_Threshold == other.m_Threshold
    && m_DeadZone == other.m_DeadZone
    && m_Multiplier == other.m_Multiplier
    && m_Exponent == other.m_Exponent
    && m_Shape == other.m_Shape;
}
