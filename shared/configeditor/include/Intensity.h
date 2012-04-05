#ifndef INTENSITY_H
#define INTENSITY_H

#include <Device.h>
#include <Event.h>

class Intensity
{
    public:
        Intensity();
        Intensity(wxString control,
            wxString dtype1, wxString did1, wxString dname1, wxString eid1,
            wxString dtype2, wxString did2, wxString dname2, wxString eid2,
            unsigned char dead_zone, unsigned char steps, wxString Shape);
        virtual ~Intensity();
        Intensity(const Intensity& other);
        Intensity& operator=(const Intensity& other);
        Device* GetDeviceUp() { return &m_Device_up; }
        void SetDeviceUp(Device val) { m_Device_up = val; }
        Event* GetEventUp() { return &m_Event_up; }
        void SetEventUp(Event val) { m_Event_up = val; }
        Device* GetDeviceDown() { return &m_Device_down; }
        void SetDeviceDown(Device val) { m_Device_down = val; }
        Event* GetEventDown() { return &m_Event_down; }
        void SetEventDown(Event val) { m_Event_down = val; }
        unsigned char GetDeadZone() { return m_dead_zone; }
        void SetDeadZone(unsigned char val) { m_dead_zone = val; }
        unsigned char GetSteps() { return m_steps; }
        void SetSteps(unsigned char val) { m_steps = val; }
        wxString GetShape() { return m_shape; }
        void SetShape(wxString val) { m_shape = val; }
        wxString GetControl() { return m_Control; }
        void SetControl(wxString val) { m_Control = val; }
    protected:
    private:
        wxString m_Control;
        Device m_Device_up;
        Event m_Event_up;
        Device m_Device_down;
        Event m_Event_down;
        unsigned char m_dead_zone;
        wxString m_shape;
        unsigned char m_steps;
};

#endif // INTENSITY_H
