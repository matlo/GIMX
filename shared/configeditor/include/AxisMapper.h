#ifndef AXISMAPPER_H
#define AXISMAPPER_H

#include <Device.h>
#include <Event.h>

class AxisMapper
{
    public:
        AxisMapper();
        AxisMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString axis, wxString deadZone, wxString multiplier, wxString exponent, wxString shape, wxString buffersize, wxString filter, wxString label);
        virtual ~AxisMapper();
        AxisMapper(const AxisMapper& other);
        AxisMapper& operator=(const AxisMapper& other);
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        wxString GetAxis() { return m_Axis; }
        void SetAxis(wxString val) { m_Axis = val; }
        wxString GetLabel() { return m_Label; }
        void SetLabel(wxString val) { m_Label = val; }
    protected:
    private:
        Device m_Device;
        Event m_Event;
        wxString m_Axis;
		    wxString m_Label;
};

#endif // AXISMAPPER_H
