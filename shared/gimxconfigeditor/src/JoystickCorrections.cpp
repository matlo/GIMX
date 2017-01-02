/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "JoystickCorrection.h"

JoystickCorrection::JoystickCorrection()
{
    //ctor
}

JoystickCorrection::JoystickCorrection(string dname, string did, string aid,
    string lvalue, string lcoef, string hvalue, string hcoef):
    m_Joystick("joystick", did, dname),
    m_Axis("axis", aid),
    m_LowValue(lvalue),
    m_LowCoef(lcoef),
    m_HighValue(hvalue),
    m_HighCoef(hcoef)
{
    //ctor
}

JoystickCorrection::~JoystickCorrection()
{
    //dtor
}

JoystickCorrection::JoystickCorrection(const JoystickCorrection& other):
    m_Joystick(other.m_Joystick),
    m_Axis(other.m_Axis),
    m_LowValue(other.m_LowValue),
    m_LowCoef(other.m_LowCoef),
    m_HighValue(other.m_HighValue),
    m_HighCoef(other.m_HighCoef)
{
    //copy ctor
}

JoystickCorrection& JoystickCorrection::operator=(const JoystickCorrection& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Joystick = rhs.m_Joystick;
    m_Axis = rhs.m_Axis;
    m_LowValue = rhs.m_LowValue;
    m_LowCoef = rhs.m_LowCoef;
    m_HighValue = rhs.m_HighValue;
    m_HighCoef = rhs.m_HighCoef;
    return *this;
}

bool JoystickCorrection::operator==(const JoystickCorrection &other) const
{
    return m_Joystick == other.m_Joystick
    && m_Axis == other.m_Axis
    && m_LowValue == other.m_LowValue
    && m_LowCoef == other.m_LowCoef
    && m_HighValue == other.m_HighValue
    && m_HighCoef == other.m_HighCoef;
}
