/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Configuration.h"

Configuration::Configuration()
{
    //ctor
}

Configuration::~Configuration()
{
    //dtor
}

Configuration::Configuration(const Configuration& other):m_Trigger(other.m_Trigger), m_IntensityList(other.m_IntensityList),
    m_MouseOptionsList(other.m_MouseOptionsList), m_ButtonMappers(other.m_ButtonMappers), m_AxisMappers(other.m_AxisMappers)
{
    //copy ctor
}

Configuration& Configuration::operator=(const Configuration& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Trigger = rhs.m_Trigger;
    m_IntensityList = rhs.m_IntensityList;
    m_MouseOptionsList = rhs.m_MouseOptionsList;
    m_ButtonMappers = rhs.m_ButtonMappers;
    m_AxisMappers = rhs.m_AxisMappers;
    return *this;
}

bool Configuration::IsEmpty()
{
  if(m_Trigger.GetDevice()->GetType().size())
  {
    return false;
  }
  if(m_IntensityList.size())
  {
    return false;
  }
  if(m_MouseOptionsList.size())
  {
    return false;
  }
  if(m_ButtonMappers.size())
  {
    return false;
  }
  if(m_AxisMappers.size())
  {
    return false;
  }
  return true;
}
