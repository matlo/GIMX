#include "Intensity.h"

Intensity::Intensity()
{
    //ctor
    m_dead_zone = 0;
    m_steps = 0;
}

Intensity::Intensity(wxString control,
    wxString dtype1, wxString did1, wxString dname1, wxString eid1,
    wxString dtype2, wxString did2, wxString dname2, wxString eid2,
    unsigned char dead_zone, unsigned char steps, wxString shape):
    m_Control(control),
    m_Device_up(dtype1, did1, dname1), m_Event_up(eid1),
    m_Device_down(dtype2, did2, dname2), m_Event_down(eid2),
    m_dead_zone(dead_zone),
    m_shape(shape),
    m_steps(steps)
{
    //ctor
}

Intensity::~Intensity()
{
    //dtor
}

Intensity::Intensity(const Intensity& other):
    m_Control(other.m_Control),
    m_Device_up(other.m_Device_up), m_Event_up(other.m_Event_up),
    m_Device_down(other.m_Device_down), m_Event_down(other.m_Event_down),
    m_dead_zone(other.m_dead_zone),
    m_shape(other.m_shape),
    m_steps(other.m_steps)
{
    //copy ctor
}

Intensity& Intensity::operator=(const Intensity& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device_up = rhs.m_Device_up;
    m_Event_up = rhs.m_Event_up;
    m_Device_down = rhs.m_Device_down;
    m_Event_down = rhs.m_Event_down;
    m_Control = rhs.m_Control;
    m_dead_zone = rhs.m_dead_zone;
    m_shape = rhs.m_shape;
    m_steps = rhs.m_steps;
    return *this;
}
