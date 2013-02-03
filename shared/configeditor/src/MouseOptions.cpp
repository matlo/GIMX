/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "MouseOptions.h"

MouseOptions::MouseOptions()
{
    //ctor
}

MouseOptions::MouseOptions(string dname, string did, string mode,
    string bsize, string filter):
    m_Mouse("mouse", did, dname),
    m_Mode(mode),
    m_BufferSize(bsize),
    m_Filter(filter)
{
    //ctor
}

MouseOptions::~MouseOptions()
{
    //dtor
}

MouseOptions::MouseOptions(const MouseOptions& other):
    m_Mouse(other.m_Mouse),
    m_Mode(other.m_Mode),
    m_BufferSize(other.m_BufferSize),
    m_Filter(other.m_Filter)
{
    //copy ctor
}

MouseOptions& MouseOptions::operator=(const MouseOptions& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    m_Mouse = rhs.m_Mouse;
    m_Mode = rhs.m_Mode;
    m_BufferSize = rhs.m_BufferSize;
    m_Filter = rhs.m_Filter;
    return *this;
}
