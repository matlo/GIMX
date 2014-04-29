/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLMAPPER_H
#define CONTROLMAPPER_H

#include <Device.h>
#include <Event.h>

#include <controller2.h>

class ControlMapper
{
    public:
        ControlMapper();
        ControlMapper(string dtype, string did, string dname, string etype, string eid, string threshold, s_axis_props button, string label);
        ControlMapper(string dtype, string did, string dname, string etype, string eid, s_axis_props axis, string deadZone, string multiplier, string exponent, string shape, string label);
        virtual ~ControlMapper();
        ControlMapper(const ControlMapper& other);
        ControlMapper& operator=(const ControlMapper& other);
        bool operator==(const ControlMapper &other) const;
        bool CompareAxisProps(s_axis_props axis) const;
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        s_axis_props GetAxis() { return m_Axis; }
        void SetAxis(s_axis_props val) { m_Axis = val; }
        string GetLabel() { return m_Label; }
        void SetLabel(string val) { m_Label = val; }
        static s_axis_props GetAxisProps(string button);
        static s_axis_props GetGenericAxisProps(e_controller_type type, string axis);
        string GetGenericAxisName();
        static string GetGenericAxisName(s_axis_props axis_props);
        string GetSpecificAxisName(e_controller_type type);
        static string GetSpecificAxisName(e_controller_type type, s_axis_props axis_props);
    protected:
    private:
        Device m_Device;
        Event m_Event;
        s_axis_props m_Axis;
		    string m_Label;
};

#endif // CONTROLMAPPER_H
