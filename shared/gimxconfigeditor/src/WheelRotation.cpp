/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <WheelRotation.h>

WheelRotation::WheelRotation() : m_Rotation("900") {

}

WheelRotation::WheelRotation(string deviceName, string deviceId, string rotation) : m_Joystick("joystick", deviceName, deviceId), m_Rotation(rotation) {

}

WheelRotation::~WheelRotation() {

}

WheelRotation::WheelRotation(const WheelRotation& other) : m_Joystick(other.m_Joystick), m_Rotation(other.m_Rotation) {

}

WheelRotation& WheelRotation::operator=(const WheelRotation& rhs) {
	if (this == &rhs) return *this; // handle self assignment
	m_Joystick = rhs.m_Joystick;
	m_Rotation = rhs.m_Rotation;
	return *this;
}

bool WheelRotation::operator==(const WheelRotation& other) const {
	return m_Joystick == other.m_Joystick
		&& m_Rotation == other.m_Rotation;
}