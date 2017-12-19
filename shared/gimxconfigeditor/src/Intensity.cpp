/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Intensity.h"

Intensity::Intensity()
{
    //ctor
}

Intensity::Intensity(s_axis_props axis,
    string dtype1, string dname1, string did1, string eid1,
    string direction, string dead_zone, string shape, string steps):
    m_Axis(axis),
    m_Device(dtype1, did1, dname1), m_Event(eid1),
    m_direction(direction),
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
    m_Axis(other.m_Axis),
    m_Device(other.m_Device), m_Event(other.m_Event),
    m_direction(other.m_direction),
    m_dead_zone(other.m_dead_zone),
    m_shape(other.m_shape),
    m_steps(other.m_steps)
{
    //copy ctor
}

Intensity& Intensity::operator=(const Intensity& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Device = rhs.m_Device;
    m_Event = rhs.m_Event;
    m_direction = rhs.m_direction;
    m_Axis = rhs.m_Axis;
    m_dead_zone = rhs.m_dead_zone;
    m_shape = rhs.m_shape;
    m_steps = rhs.m_steps;
    return *this;
}

bool Intensity::operator==(const Intensity &other) const
{
    return m_Device == other.m_Device
    && m_Event == other.m_Event
    && m_direction == other.m_direction
    && CompareAxisProps(other.m_Axis)
    && m_dead_zone == other.m_dead_zone
    && m_shape == other.m_shape
    && m_steps == other.m_steps;
}

bool Intensity::CompareAxisProps(s_axis_props axis) const
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

string Intensity::GetGenericAxisName()
{
  return controller_get_generic_axis_name_from_index(m_Axis);
}

string Intensity::GetSpecificAxisName(e_controller_type type)
{
  string name = controller_get_specific_axis_name_from_index(type, m_Axis);
  if(name.find("stick", 0) == 1)
  {
    // "rstick x" -> "rstick"
    // "rstick y" -> "rstick"
    // "lstick x" -> "lstick"
    // "lstick y" -> "lstick"
    name = name.substr(0, 6);
  }
  return name;
}

string Intensity::GetGenericAxisName(s_axis_props axis_props)
{
  return controller_get_generic_axis_name_from_index(axis_props);
}

s_axis_props Intensity::GetAxisProps(string axis)
{
  if(axis == "lstick")
  {
    axis = "lstick x";
  }
  else if(axis == "rstick")
  {
    axis = "rstick x";
  }
  return controller_get_axis_index_from_name(axis.c_str());
}

s_axis_props Intensity::GetGenericAxisProps(e_controller_type type, string axis)
{
  if(axis == "lstick")
  {
    axis = "lstick x";
  }
  else if(axis == "rstick")
  {
    axis = "rstick x";
  }
  return controller_get_axis_index_from_specific_name(type, axis.c_str());
}
