#include "AxisMapper.h"

AxisMapper::AxisMapper()
{
    //ctor
}

AxisMapper::AxisMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString axis, wxString deadZone, wxString multiplier, wxString exponent, wxString shape, wxString buffersize, wxString filter, wxString label):
m_Device(dtype, did, dname), m_Event(etype, eid, deadZone, multiplier, exponent, shape, buffersize, filter), m_Axis(axis), m_Label(label)
{
    //ctor
}

AxisMapper::~AxisMapper()
{
    //dtor
}

AxisMapper::AxisMapper(const AxisMapper& other):
m_Device(other.m_Device), m_Event(other.m_Event), m_Axis(other.m_Axis), m_Label(other.m_Label)
{
    //copy ctor
}

AxisMapper& AxisMapper::operator=(const AxisMapper& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_Axis = rhs.m_Axis;
    m_Label = rhs.m_Label;
    return *this;
}
