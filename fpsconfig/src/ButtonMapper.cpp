#include "ButtonMapper.h"

ButtonMapper::ButtonMapper()
{
    //ctor
}

ButtonMapper::ButtonMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString threshold, wxString button):
m_Device(dtype, did, dname), m_Event(etype, eid, threshold), m_Button(button)
{
    //ctor
}

ButtonMapper::~ButtonMapper()
{
    //dtor
}

ButtonMapper::ButtonMapper(const ButtonMapper& other):
m_Device(other.m_Device), m_Event(other.m_Event), m_Button(other.m_Button)
{
    //copy ctor
}

ButtonMapper& ButtonMapper::operator=(const ButtonMapper& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_Button = rhs.m_Button;
    return *this;
}
