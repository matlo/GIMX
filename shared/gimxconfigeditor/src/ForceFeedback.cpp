/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ForceFeedback.h>

ForceFeedback::ForceFeedback() :
m_Inversion("no"), m_RumbleGain("100"), m_ConstantGain("100"), m_DamperGain("100"), m_SpringGain("100")
{
    //ctor
}

ForceFeedback::ForceFeedback(string deviceName, string deviceId,
        string inversion, string rumbleGain, string constantGain, string damperGain, string springGain):
    m_Joystick("joystick", deviceName, deviceId), m_Inversion(inversion),
    m_RumbleGain(rumbleGain), m_ConstantGain(constantGain), m_DamperGain(damperGain), m_SpringGain(springGain)
{
    //ctor
}

ForceFeedback::~ForceFeedback()
{
    //dtor
}

ForceFeedback::ForceFeedback(const ForceFeedback& other):
    m_Joystick(other.m_Joystick),
    m_Inversion(other.m_Inversion),
    m_RumbleGain(other.m_RumbleGain),
    m_ConstantGain(other.m_ConstantGain),
    m_DamperGain(other.m_DamperGain),
    m_SpringGain(other.m_SpringGain)
{
    //copy ctor
}

ForceFeedback& ForceFeedback::operator=(const ForceFeedback& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Joystick = rhs.m_Joystick;
    m_Inversion = rhs.m_Inversion;
    m_RumbleGain = rhs.m_RumbleGain;
    m_ConstantGain = rhs.m_ConstantGain;
    m_DamperGain = rhs.m_DamperGain;
    m_SpringGain = rhs.m_SpringGain;
    return *this;
}

bool ForceFeedback::operator==(const ForceFeedback &other) const
{
    return m_Joystick == other.m_Joystick
    && m_Inversion == other.m_Inversion
    && m_RumbleGain == other.m_RumbleGain
    && m_ConstantGain == other.m_ConstantGain
    && m_DamperGain == other.m_DamperGain
    && m_SpringGain == other.m_SpringGain;
}
