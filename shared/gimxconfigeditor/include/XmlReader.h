/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XMLREADER_H
#define XMLREADER_H

#include <ConfigurationFile.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

#define X_NODE_ROOT "root"
#define X_NODE_CONTROLLER "controller"
#define X_NODE_CONFIGURATION "configuration"

#define X_NODE_TRIGGER "trigger"
#define X_NODE_MOUSEOPTIONS "mouse_options"
#define X_NODE_MOUSEOPTIONS_LIST "mouse_options_list"
#define X_NODE_INTENSITY "intensity"
#define X_NODE_INTENSITY_LIST "intensity_list"
#define X_NODE_BUTTON_MAP "button_map"
#define X_NODE_AXIS_MAP "axis_map"

#define X_NODE_DEVICE "device"
#define X_NODE_EVENT "event"
#define X_NODE_AXIS "axis"
#define X_NODE_BUTTON "button"

#define X_NODE_UP "up"
#define X_NODE_DOWN "down"

#define X_ATTR_ID "id"
#define X_ATTR_DPI "dpi"
#define X_ATTR_TYPE "type"
#define X_ATTR_NAME "name"
#define X_ATTR_BUTTON_ID "button_id"
#define X_ATTR_THRESHOLD "threshold"
#define X_ATTR_DEADZONE "dead_zone"
#define X_ATTR_MULTIPLIER "multiplier"
#define X_ATTR_EXPONENT "exponent"
#define X_ATTR_SHAPE "shape"
#define X_ATTR_BUFFERSIZE "buffer_size"
#define X_ATTR_FILTER "filter"
#define X_ATTR_SWITCH_BACK "switch_back"
#define X_ATTR_DELAY "delay"
#define X_ATTR_STEPS "steps"
#define X_ATTR_CONTROL "control"

#define X_ATTR_LABEL "label"

#define X_NODE_MOUSE "mouse"
#define X_NODE_MODE "mode"
#define X_NODE_TOGGLE "toggle"
#define X_NODE_SMOOTHING "smoothing"

#define X_ATTR_MODE "mode"

#define X_NODE_JOYSTICK_CORRECTIONS_LIST "joystick_corrections_list"
#define X_NODE_CORRECTION "correction"
#define X_ATTR_LOW_VALUE "low_value"
#define X_ATTR_LOW_COEF "low_coef"
#define X_ATTR_HIGH_VALUE "high_value"
#define X_ATTR_HIGH_COEF "high_coef"

#define X_NODE_FORCE_FEEDBACK "force_feedback"
#define X_NODE_INVERSION "inversion"
#define X_ATTR_ENABLE "enable"
#define X_NODE_GAIN "gain"
#define X_ATTR_RUMBLE "rumble"
#define X_ATTR_CONSTANT "constant"
#define X_ATTR_SPRING "spring"
#define X_ATTR_DAMPER "damper"

#define X_ATTR_VALUE_YES "yes"
#define X_ATTR_VALUE_NO "no"

#define X_NODE_MACROS "macros"

class XmlReader
{
    public:
        XmlReader();
        XmlReader(ConfigurationFile* configFile);
        virtual ~XmlReader();
        int ReadConfigFile(string filePath);
        string GetInfo() {return m_info;};
        string GetError() {return m_error;};
        bool MultipleMK();
        void SetCheckDevices(bool check) { m_checkDevices = check; }
    protected:
    private:
        void ProcessRootElement(xmlNode * a_node);
        void ProcessControllerElement(xmlNode * a_node);
        void ProcessConfigurationElement(xmlNode * a_node);
        void ProcessTriggerElement(xmlNode * a_node);
        void ProcessMouseOptionsElement(xmlNode * a_node);
        void ProcessMouseOptionsListElement(xmlNode * a_node);
        void AddMouseOptions(Device* device, string buffersize, string filter);
        void ProcessIntensityElement(xmlNode * a_node);
        void ProcessIntensityListElement(xmlNode * a_node);
        void ProcessJoystickCorrectionsListElement(xmlNode * a_node);
        void ProcessForceFeedbackElement(xmlNode * a_node);
        void ProcessMacrosElement(xmlNode * a_node);
        void ProcessCorrectionElement(xmlNode * a_node);
        void ProcessButtonMapElement(xmlNode * a_node);
        void ProcessAxisMapElement(xmlNode * a_node);
        void ProcessButtonElement(xmlNode * a_node);
        void ProcessAxisElement(xmlNode * a_node);
        void ProcessDeviceElement(xmlNode * a_node);
        void CheckDevice(string type, string name, string id);
        void ProcessEventElement(xmlNode * a_node);
        void ProcessInversionElement(xmlNode * a_node);
        void ProcessGainElement(xmlNode * a_node);
        Event m_TempEvent;
        Device m_TempDevice;
        ControlMapper m_TempButtonMapper;
        ControlMapper m_TempAxisMapper;
        Trigger m_TempTrigger;
        MouseOptions m_TempMouseOptions;
        JoystickCorrection m_TempJoystickCorrection;
        ForceFeedback m_TempForceFeedback;
        Intensity m_TempIntensity;
        Profile m_TempConfiguration;
        Controller m_TempController;
        ConfigurationFile m_TempConfigurationFile;
        ConfigurationFile* m_ConfigurationFile;
        EventCatcher* m_evtcatch;
        string m_info;
        string m_error;
        bool m_name_empty;
        bool m_name_nempty;
        bool m_checkDevices;
};

#endif // XMLREADER_H
