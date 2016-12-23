/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER_H

#define CONTROLLER_H

#include "Profile.h"
#include <defs.h>

#define MAX_PROFILES 8

class Controller
{
    public:
        Controller();
        virtual ~Controller();
        Controller(const Controller& other);
        Controller& operator=(const Controller& other);
        Profile* GetProfile(unsigned int i) { return m_Profile+i; }
        void SetProfile(Profile val, unsigned int i) { m_Profile[i] = val; }
        e_controller_type GetControllerType() { return m_ControllerType; };
        void SetControllerType(e_controller_type type) { m_ControllerType = type; }
        unsigned int GetMouseDPI() { return m_MouseDPI; }
        void SetMouseDPI(unsigned int val) { m_MouseDPI = val; }
        bool IsEmpty();
    protected:
    private:
        e_controller_type m_ControllerType;
        unsigned int m_MouseDPI;
        Profile m_Profile[MAX_PROFILES];
};

#endif // CONTROLLER_H
