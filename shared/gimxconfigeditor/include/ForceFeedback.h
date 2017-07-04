/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FORCEFEEDBACK_H
#define FORCEFEEDBACK_H

#include "Device.h"

class ForceFeedback
{
    public:
        ForceFeedback();
        ForceFeedback(string deviceName, string deviceId,
                string inversion, string constantGain, string damperGain, string springGain);
        virtual ~ForceFeedback();
        ForceFeedback(const ForceFeedback& other);
        ForceFeedback& operator=(const ForceFeedback& other);
        bool operator==(const ForceFeedback &other) const;
        Device* GetJoystick() { return &m_Joystick; }
        void SetJoystick(Device val) { m_Joystick = val; }
        string getInversion() const { return m_Inversion; }
        void setInversion(string inversion) { m_Inversion = inversion; }
        string getConstantGain() const { return m_ConstantGain; }
        void setConstantGain(string constantGain) { m_ConstantGain = constantGain; }
        string getDamperGain() const { return m_DamperGain; }
        void setDamperGain(string damperGain) { m_DamperGain = damperGain; }
        string getSpringGain() const { return m_SpringGain; }
        void setSpringGain(string springGain) { m_SpringGain = springGain; }
    protected:
    private:
        Device m_Joystick;
        string m_Inversion;
        string m_ConstantGain;
        string m_DamperGain;
        string m_SpringGain;
};

#endif // FORCEFEEDBACK_H
