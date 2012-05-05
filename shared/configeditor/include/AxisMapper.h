#ifndef AXISMAPPER_H
#define AXISMAPPER_H

#include <Device.h>
#include <Event.h>

class AxisMapper
{
    public:
        AxisMapper();
        AxisMapper(string dtype, string did, string dname, string etype, string eid, string axis, string deadZone, string multiplier, string exponent, string shape, string buffersize, string filter, string label);
        virtual ~AxisMapper();
        AxisMapper(const AxisMapper& other);
        AxisMapper& operator=(const AxisMapper& other);
        Device* GetDevice() { return &m_Device; }
        void SetDevice(Device val) { m_Device = val; }
        Event* GetEvent() { return &m_Event; }
        void SetEvent(Event val) { m_Event = val; }
        string GetAxis() { return m_Axis; }
        void SetAxis(string val) { m_Axis = val; }
        string GetLabel() { return m_Label; }
        void SetLabel(string val) { m_Label = val; }
    protected:
    private:
        Device m_Device;
        Event m_Event;
        string m_Axis;
		    string m_Label;
};

#endif // AXISMAPPER_H
