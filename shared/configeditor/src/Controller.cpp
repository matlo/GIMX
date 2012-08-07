#include "Controller.h"

Controller::Controller()
{
    m_MouseDPI = 0;
    //ctor
}

Controller::~Controller()
{
    //dtor
}

Controller::Controller(const Controller& other)
{
    m_MouseDPI = other.m_MouseDPI;
    for(unsigned int i=0; i<sizeof(m_Configurations)/sizeof(Configuration); ++i)
    {
        m_Configurations[i] = other.m_Configurations[i];
    }
}

Controller& Controller::operator=(const Controller& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_MouseDPI = rhs.m_MouseDPI;
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
