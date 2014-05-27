/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Device.h>
#include <Event.h>
#include <Trigger.h>
#include <Intensity.h>
#include <ControlMapper.h>
#include <MouseOptions.h>
#include <list>

class Configuration
{
    public:
        Configuration();
        virtual ~Configuration();
        Configuration(const Configuration& other);
        Configuration& operator=(const Configuration& other);
        Trigger* GetTrigger() { return &m_Trigger; }
        void SetTrigger(Trigger val) { m_Trigger = val; }
        std::list<Intensity>* GetIntensityList() { return &m_IntensityList; }
        void SetIntensityList(std::list<Intensity> val) { m_IntensityList = val; }
        std::list<MouseOptions>* GetMouseOptionsList() { return &m_MouseOptionsList; }
        void SetMouseOptionsList(std::list<MouseOptions> val) { m_MouseOptionsList = val; }
        std::list<ControlMapper>* GetButtonMapperList() { return &m_ButtonMappers; }
        std::list<ControlMapper>* GetAxisMapperList() { return &m_AxisMappers; }
        void SetButtonMappers(std::list<ControlMapper> bml) { m_ButtonMappers = bml; }
        void SetAxisMappers(std::list<ControlMapper> aml) { m_AxisMappers = aml; }
        bool IsEmpty();
    protected:
    private:
        Trigger m_Trigger;
        std::list<Intensity> m_IntensityList;
        std::list<MouseOptions> m_MouseOptionsList;
        std::list<ControlMapper> m_ButtonMappers;
        std::list<ControlMapper> m_AxisMappers;
};

#endif // CONFIGURATION_H
