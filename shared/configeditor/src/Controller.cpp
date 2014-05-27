/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Controller.h"

Controller::Controller()
{
    m_MouseDPI = 0;
    m_ControllerType = C_TYPE_SIXAXIS;
    //ctor
}

Controller::~Controller()
{
    //dtor
}

Controller::Controller(const Controller& other)
{
    m_MouseDPI = other.m_MouseDPI;
    m_ControllerType = other.m_ControllerType;
    for(unsigned int i=0; i<sizeof(m_Configurations)/sizeof(Configuration); ++i)
    {
        m_Configurations[i] = other.m_Configurations[i];
    }
}

Controller& Controller::operator=(const Controller& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_MouseDPI = rhs.m_MouseDPI;
    m_ControllerType = rhs.m_ControllerType;
    for(unsigned int i=0; i<sizeof(m_Configurations)/sizeof(Configuration); ++i)
    {
        m_Configurations[i] = rhs.m_Configurations[i];
    }
    return *this;
}

bool Controller::IsEmpty()
{
  for(int i =0; i<MAX_CONFIGURATIONS; ++i)
  {
    if(!m_Configurations[i].IsEmpty())
    {
      return false;
    }
  }
  return true;
}
