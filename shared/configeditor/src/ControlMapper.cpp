/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "ControlMapper.h"

ControlMapper::ControlMapper()
{
    //ctor
}

ControlMapper::ControlMapper(string dtype, string did, string dname, string etype, string eid, string threshold, s_axis_props button, string label):
m_Device(dtype, did, dname), m_Event(etype, eid, threshold), m_Axis(button), m_Label(label)
{
  //ctor
}

ControlMapper::ControlMapper(string dtype, string did, string dname, string etype, string eid, s_axis_props axis, string deadZone, string multiplier, string exponent, string shape, string label):
m_Device(dtype, did, dname), m_Event(etype, eid, deadZone, multiplier, exponent, shape), m_Axis(axis), m_Label(label)
{
  //ctor
}

ControlMapper::~ControlMapper()
{
    //dtor
}

ControlMapper::ControlMapper(const ControlMapper& other):
m_Device(other.m_Device), m_Event(other.m_Event), m_Axis(other.m_Axis), m_Label(other.m_Label)
{
    //copy ctor
}

ControlMapper& ControlMapper::operator=(const ControlMapper& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_Axis = rhs.m_Axis;
    m_Label = rhs.m_Label;
    return *this;
}

bool ControlMapper::operator==(const ControlMapper &other) const
{
    return m_Device == other.m_Device
    && m_Event == other.m_Event
    && CompareAxisProps(other.m_Axis)
    && m_Label == other.m_Label;
}

bool ControlMapper::CompareAxisProps(s_axis_props axis) const
{
  if(m_Axis.axis != axis.axis)
  {
    return false;
  }
  if(m_Axis.axis <= rel_axis_max && m_Axis.props != axis.props)
  {
    return false;
  }
  return true;
}

s_axis_props ControlMapper::GetAxisProps(string button)
{
  return controller_get_axis_index_from_name(button.c_str());
}

s_axis_props ControlMapper::GetGenericAxisProps(e_controller_type type, string axis)
{
  return controller_get_axis_index_from_specific_name(type, axis.c_str());
}

string ControlMapper::GetGenericAxisName()
{
  return controller_get_generic_axis_name_from_index(m_Axis);
}

string ControlMapper::GetGenericAxisName(s_axis_props axis_props)
{
  return controller_get_generic_axis_name_from_index(axis_props);
}

string ControlMapper::GetSpecificAxisName(e_controller_type type)
{
  return controller_get_specific_axis_name_from_index(type, m_Axis);
}

string ControlMapper::GetSpecificAxisName(e_controller_type type, s_axis_props axis_props)
{
  return controller_get_specific_axis_name_from_index(type, axis_props);
}
