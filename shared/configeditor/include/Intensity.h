#ifndef INTENSITY_H
#define INTENSITY_H

#include <Device.h>
#include <Event.h>

class Intensity
{
    public:
        Intensity();
        Intensity(string control,
            string dtype1, string did1, string dname1, string eid1,
            string dtype2, string did2, string dname2, string eid2,
            unsigned char dead_zone, unsigned char steps, string Shape);
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
        string GetShape() { return m_shape; }
        void SetShape(string val) { m_shape = val; }
        string GetControl() { return m_Control; }
        void SetControl(string val) { m_Control = val; }
    protected:
    private:
        string m_Control;
        Device m_Device_up;
        Event m_Event_up;
        Device m_Device_down;
        Event m_Event_down;
        unsigned char m_dead_zone;
        string m_shape;
        unsigned char m_steps;
};

#endif // INTENSITY_H
