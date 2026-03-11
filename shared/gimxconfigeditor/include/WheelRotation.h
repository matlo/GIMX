/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef WHEELROTATION_H
#define WHEELROTATION_H

#include "Device.h"

class WheelRotation {
public:
	WheelRotation();
	WheelRotation(string deviceName, string deviceId, string rotation);
    virtual ~WheelRotation();
    WheelRotation(const WheelRotation& other);
    WheelRotation& operator=(const WheelRotation& other);
    bool operator==(const WheelRotation& other) const;
    Device* GetJoystick() { return &m_Joystick; }
    void SetJoystick(Device val) { m_Joystick = val; }
    string GetRotation() { return m_Rotation; }
    void SetRotation(string rotation) { m_Rotation = rotation; }
protected:
private:
    Device m_Joystick;
    string m_Rotation;
};
#endif