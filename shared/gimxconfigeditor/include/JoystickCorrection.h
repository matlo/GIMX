/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef JOYSTICKCORRECTION_H
#define JOYSTICKCORRECTION_H

#include "Device.h"
#include "Event.h"

class JoystickCorrection
{
    public:
        JoystickCorrection();
        JoystickCorrection(string dname, string did, string aid,
            string lvalue, string lcoef, string hvalue, string hcoef);
        virtual ~JoystickCorrection();
        JoystickCorrection(const JoystickCorrection& other);
        JoystickCorrection& operator=(const JoystickCorrection& other);
        bool operator==(const JoystickCorrection &other) const;
        Device* GetJoystick() { return &m_Joystick; }
        void SetJoystick(Device val) { m_Joystick = val; }
        Event* GetAxis() { return &m_Axis; }
        void SetAxis(Event val) { m_Axis = val; }
        string GetLowValue() { return m_LowValue; }
        void SetLowValue(string val) { m_LowValue = val; }
        string GetLowCoef() { return m_LowCoef; }
        void SetLowCoef(string val) { m_LowCoef = val; }
        string GetHighValue() { return m_HighValue; }
        void SetHighValue(string val) { m_HighValue = val; }
        string GetHighCoef() { return m_HighCoef; }
        void SetHighCoef(string val) { m_HighCoef = val; }
    protected:
    private:
        Device m_Joystick;
        Event m_Axis;
        string m_LowValue;
        string m_LowCoef;
        string m_HighValue;
        string m_HighCoef;
};

#endif // JOYSTICKCORRECTION_H
