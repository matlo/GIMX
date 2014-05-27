/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Configuration.h>
#include <defs.h>

#define MAX_CONFIGURATIONS 8

class Controller
{
    public:
        Controller();
        virtual ~Controller();
        Controller(const Controller& other);
        Controller& operator=(const Controller& other);
        Configuration* GetConfiguration(unsigned int i) { return m_Configurations+i; }
        void SetConfiguration(Configuration val, unsigned int i) { m_Configurations[i] = val; }
        e_controller_type GetControllerType() { return m_ControllerType; };
        void SetControllerType(e_controller_type type) { m_ControllerType = type; }
        unsigned int GetMouseDPI() { return m_MouseDPI; }
        void SetMouseDPI(unsigned int val) { m_MouseDPI = val; }
        bool IsEmpty();
    protected:
    private:
        e_controller_type m_ControllerType;
        unsigned int m_MouseDPI;
        Configuration m_Configurations[MAX_CONFIGURATIONS];
};

#endif // CONTROLLER_H
