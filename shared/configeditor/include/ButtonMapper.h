/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef BUTTONMAPPER_H
#define BUTTONMAPPER_H

#include <Device.h>
#include <Event.h>

class ButtonMapper
{
    public:
        ButtonMapper();
        ButtonMapper(string dtype, string did, string dname, string etype, string eid, string threshold, string button, string label);
        virtual ~ButtonMapper();
        ButtonMapper(const ButtonMapper& other);
        ButtonMapper& operator=(const ButtonMapper& other);
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        string GetButton() { return m_Button; }
        void SetButton(string val) { m_Button = val; }
        string GetLabel() { return m_Label; }
        void SetLabel(string val) { m_Label = val; }
    protected:
    private:
        Device m_Device;
        Event m_Event;
        string m_Button;
		    string m_Label;
};

#endif // BUTTONMAPPER_H
