#ifndef BUTTONMAPPER_H
#define BUTTONMAPPER_H

#include <Device.h>
#include <Event.h>

class ButtonMapper
{
    public:
        ButtonMapper();
        ButtonMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString threshold, wxString button, wxString label);
        virtual ~ButtonMapper();
        ButtonMapper(const ButtonMapper& other);
        ButtonMapper& operator=(const ButtonMapper& other);
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        wxString GetButton() { return m_Button; }
        void SetButton(wxString val) { m_Button = val; }
        wxString GetLabel() { return m_Label; }
        void SetLabel(wxString val) { m_Label = val; }
    protected:
    private:
        Device m_Device;
        Event m_Event;
        wxString m_Button;
		    wxString m_Label;
};

#endif // BUTTONMAPPER_H
