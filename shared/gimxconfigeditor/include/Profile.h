/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef PROFILE_H
#define PROFILE_H

#include "Device.h"
#include "Event.h"
#include "Trigger.h"
#include "Intensity.h"
#include "ControlMapper.h"
#include "MouseOptions.h"
#include "JoystickCorrection.h"
#include "ForceFeedback.h"
#include <list>

class Profile
{
    public:
        Profile();
        virtual ~Profile();
        Profile(const Profile& other);
        Profile& operator=(const Profile& other);
        Trigger* GetTrigger() { return &m_Trigger; }
        void SetTrigger(Trigger val) { m_Trigger = val; }
        std::list<Intensity>* GetIntensityList() { return &m_IntensityList; }
        void SetIntensityList(std::list<Intensity> val) { m_IntensityList = val; }
        std::list<MouseOptions>* GetMouseOptionsList() { return &m_MouseOptionsList; }
        void SetMouseOptionsList(std::list<MouseOptions> val) { m_MouseOptionsList = val; }
        std::list<JoystickCorrection>* GetJoystickCorrectionsList() { return &m_JoystickCorrectionsList; }
        void SetJoystickCorrectionsList(std::list<JoystickCorrection> val) { m_JoystickCorrectionsList = val; }
        ForceFeedback* GetForceFeedback() { return &m_ForceFeedback; }
        void SetForceFeedback(ForceFeedback val) { m_ForceFeedback = val; }
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
        std::list<JoystickCorrection> m_JoystickCorrectionsList;
        ForceFeedback m_ForceFeedback;
};

#endif // PROFILE_H
