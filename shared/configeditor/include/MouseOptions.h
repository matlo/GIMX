/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef MOUSEOPTIONS_H
#define MOUSEOPTIONS_H

#include <Device.h>
#include <Event.h>

class MouseOptions
{
    public:
        MouseOptions();
        MouseOptions(string dname, string did, string mode,
            string bsize, string filter);
        virtual ~MouseOptions();
        MouseOptions(const MouseOptions& other);
        MouseOptions& operator=(const MouseOptions& other);
        Device* GetMouse() { return &m_Mouse; }
        void SetMouse(Device val) { m_Mouse = val; }
        string GetMode() { return m_Mode; }
        void SetMode(string val) { m_Mode = val; }
        string GetBufferSize() { return m_BufferSize; }
        void SetBufferSize(string val) { m_BufferSize = val; }
        string GetFilter() { return m_Filter; }
        void SetFilter(string val) { m_Filter = val; }
    protected:
    private:
        Device m_Mouse;
        string m_Mode;
        string m_BufferSize;
        string m_Filter;
};

#endif // MOUSEOPTIONS_H
