/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef INTENSITY_H
#define INTENSITY_H

#include <Device.h>
#include <Event.h>

#include <controller.h>

class Intensity
{
    public:
        Intensity();
        Intensity(s_axis_props axis,
            string dtype1, string dname1, string did1, string eid1,
            string direction, unsigned char dead_zone, string Shape, unsigned char steps);
        virtual ~Intensity();
        Intensity(const Intensity& other);
        Intensity& operator=(const Intensity& other);
        bool operator==(const Intensity &other) const;
        bool CompareAxisProps(s_axis_props axis) const;
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
        s_axis_props GetAxis() { return m_Axis; }
        void SetAxis(s_axis_props val) { m_Axis = val; }
        string GetDirection() { return m_direction; }
        void SetDirection(string val) { m_direction = val; }
        string GetGenericAxisName();
        string GetSpecificAxisName(e_controller_type type);
        static string GetGenericAxisName(s_axis_props axis_props);
        static s_axis_props GetAxisProps(string axis);
        static s_axis_props GetGenericAxisProps(e_controller_type type, string axis);
    protected:
    private:
        s_axis_props m_Axis;
        Device m_Device;
        Event m_Event;
        string m_direction;
        unsigned char m_dead_zone;
        string m_shape;
        unsigned char m_steps;
};

#endif // INTENSITY_H
