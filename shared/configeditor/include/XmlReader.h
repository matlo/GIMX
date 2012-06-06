#ifndef XMLREADER_H
#define XMLREADER_H

#include <ConfigurationFile.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

#define X_NODE_ROOT "root"
#define X_NODE_CONTROLLER "controller"
#define X_NODE_CONFIGURATION "configuration"

#define X_NODE_TRIGGER "trigger"
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
        void SetEvtCatch(event_catcher* e) { m_evtcatch = e; }
    protected:
    private:
        void ProcessRootElement(xmlNode * a_node);
        void ProcessControllerElement(xmlNode * a_node);
        void ProcessConfigurationElement(xmlNode * a_node);
        void ProcessTriggerElement(xmlNode * a_node);
        void ProcessIntensityElement(xmlNode * a_node);
        void ProcessIntensityListElement(xmlNode * a_node);
        void ProcessButtonMapElement(xmlNode * a_node);
        void ProcessAxisMapElement(xmlNode * a_node);
        void ProcessButtonElement(xmlNode * a_node);
        void ProcessAxisElement(xmlNode * a_node);
        void ProcessDeviceElement(xmlNode * a_node);
        void ProcessEventElement(xmlNode * a_node);
        Event m_TempEvent;
        Device m_TempDevice;
        ButtonMapper m_TempButtonMapper;
        AxisMapper m_TempAxisMapper;
        Trigger m_TempTrigger;
        Intensity m_TempIntensity;
        Configuration m_TempConfiguration;
        Controller m_TempController;
        ConfigurationFile m_TempConfigurationFile;
        ConfigurationFile* m_ConfigurationFile;
        event_catcher* m_evtcatch;
        string m_info;
        string m_error;
        bool m_name_empty;
        bool m_name_nempty;
};

#endif // XMLREADER_H
