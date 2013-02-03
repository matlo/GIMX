/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef INTENSITY_H
#define INTENSITY_H

#include <Device.h>
#include <Event.h>

class Intensity
{
    public:
        Intensity();
        Intensity(string control,
            string dtype1, string dname1, string did1, string eid1,
            string direction, unsigned char dead_zone, string Shape, unsigned char steps);
        virtual ~Intensity();
        Intensity(const Intensity& other);
        Intensity& operator=(const Intensity& other);
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        unsigned char GetDeadZone() { return m_dead_zone; }
        void SetDeadZone(unsigned char val) { m_dead_zone = val; }
        unsigned char GetSteps() { return m_steps; }
        void SetSteps(unsigned char val) { m_steps = val; }
        string GetShape() { return m_shape; }
        void SetShape(string val) { m_shape = val; }
        string GetControl() { return m_Control; }
        void SetControl(string val) { m_Control = val; }
        string GetDirection() { return m_direction; }
        void SetDirection(string val) { m_direction = val; }
    protected:
    private:
        string m_Control;
        Device m_Device;
        Event m_Event;
        string m_direction;
        unsigned char m_dead_zone;
        string m_shape;
        unsigned char m_steps;
};

#endif // INTENSITY_H
