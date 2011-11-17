#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Device.h>
#include <Event.h>
#include <Trigger.h>
#include <Intensity.h>
#include <ButtonMapper.h>
#include <AxisMapper.h>
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
        std::list<ButtonMapper>* GetButtonMapperList() { return &m_ButtonMappers; }
        std::list<AxisMapper>* GetAxisMapperList() { return &m_AxisMappers; }
        void SetButtonMappers(std::list<ButtonMapper> bml) { m_ButtonMappers = bml; }
        void SetAxisMappers(std::list<AxisMapper> aml) { m_AxisMappers = aml; }
    protected:
    private:
        Trigger m_Trigger;
        std::list<Intensity> m_IntensityList;
        std::list<ButtonMapper> m_ButtonMappers;
        std::list<AxisMapper> m_AxisMappers;
};

#endif // CONFIGURATION_H
