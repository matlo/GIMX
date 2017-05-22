/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "XmlReader.h"
#include <stdexcept>
#include <sstream>

#include <defs.h>
#include <controller2.h>

using namespace std;

#define EMPTY_NAME_MSG "A device name is empty. Multiple mice and keyboards are not managed."

XmlReader::XmlReader()
{
    //ctor
    m_ConfigurationFile = NULL;
    m_evtcatch = event_catcher::getInstance();
    m_name_empty = false;
    m_name_nempty = false;
    m_checkDevices = true;
}

XmlReader::XmlReader(ConfigurationFile* configFile)
{
    //ctor
    m_ConfigurationFile = configFile;
    m_evtcatch = event_catcher::getInstance();
    m_name_empty = false;
    m_name_nempty = false;
    m_checkDevices = false;
}

XmlReader::~XmlReader()
{
    //dtor
}

void XmlReader::ProcessEventElement(xmlNode * a_node)
{
    string type;
    string id;
    string threshold;
    string deadZone;
    string multiplier;
    string exponent;
    string shape;
    string bufferSize;
    string filter;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    type = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_THRESHOLD);
    threshold = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DEADZONE);
    deadZone = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_MULTIPLIER);
    multiplier = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_EXPONENT);
    exponent = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
    if(type == "axis")
    {
        shape = string(prop?prop:"Circle");
    }
    else
    {
        shape = "";
    }
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_BUFFERSIZE);
    if (type == "axis")
    {
        bufferSize = string(prop?prop:"1");
    }
    else
    {
        bufferSize = "";
    }
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_FILTER);
    if (type == "axis")
    {
        filter = string(prop?prop:"0.00");
    }
    else
    {
        filter = "";
    }
    xmlFree(prop);

    m_TempEvent.SetType(type);
    m_TempEvent.SetId(id);
    m_TempEvent.SetThreshold(threshold);
    m_TempEvent.SetDeadZone(deadZone);
    m_TempEvent.SetMultiplier(multiplier);
    m_TempEvent.SetExponent(exponent);
    m_TempEvent.SetShape(shape);

    /*
     * Compatibility with GIMX < 1.0 configs.
     */
    if(!filter.empty() && m_TempDevice.GetType() == "mouse")
    {
      AddMouseOptions(&m_TempDevice, bufferSize, filter);
    }
}

void XmlReader::CheckDevice(string type, string name, string id)
{
  string info = "";

  if(type.empty())
  {
    return;
  }

  if(name.empty())
  {
    m_name_empty = true;
  }
  else
  {
    m_name_nempty = true;

    if(m_checkDevices && !m_evtcatch->check_device(type, name, id))
    {
      info.append(type + " not found: " + name + " (" + id + ")\n");

      if(m_info.find(info) == string::npos)
      {
          m_info.append(info);
      }
    }
  }
}

void XmlReader::ProcessDeviceElement(xmlNode * a_node)
{
    string type;
    string id;
    string name;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    type = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
    name = string(prop?prop:"");
    xmlFree(prop);

    CheckDevice(type, name, id);

    m_TempDevice.SetType(type);
    m_TempDevice.SetId(id);
    m_TempDevice.SetName(name);
}

void XmlReader::AddMouseOptions(Device* device, string buffersize, string filter)
{
    list<MouseOptions>* mo_list = m_TempConfiguration.GetMouseOptionsList();

    list<MouseOptions>::iterator it;

    for(it = mo_list->begin(); it!=mo_list->end(); ++it)
    {
      if(it->GetMouse()->GetName() == device->GetName() && it->GetMouse()->GetId() == device->GetId())
      {
        break;
      }
    }

    if(it == mo_list->end())
    {
      m_TempMouseOptions.SetMouse(*device);
      m_TempMouseOptions.SetMode("Aiming");
      m_TempMouseOptions.SetBufferSize(buffersize);
      m_TempMouseOptions.SetFilter(filter);
      mo_list->push_back(m_TempMouseOptions);
    }
}

void XmlReader::ProcessAxisElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    string id;
    string label;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"");
    xmlFree(prop);
	
	  prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LABEL);
    label = string(prop?prop:"");
    xmlFree(prop);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
            {
                ProcessDeviceElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message(string("missing device element"));
        throw invalid_argument(message);
    }

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
            {
                ProcessEventElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message("missing event element");
        throw invalid_argument(message);
    }

    if(id.find("stick") == string::npos)
    {
        m_TempEvent.SetShape("");
    }

    m_TempAxisMapper.SetAxis(ControlMapper::GetAxisProps(id));
	  m_TempAxisMapper.SetLabel(label);
    m_TempAxisMapper.SetDevice(m_TempDevice);
    m_TempAxisMapper.SetEvent(m_TempEvent);
    m_TempConfiguration.GetAxisMapperList()->push_back(m_TempAxisMapper);
}

void XmlReader::ProcessButtonElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    string id;
    string label;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"");
    xmlFree(prop);
	
	  prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LABEL);
    label = string(prop?prop:"");
    xmlFree(prop);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
            {
                ProcessDeviceElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message(string("missing device element"));
        throw invalid_argument(message);
    }

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
            {
                ProcessEventElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message(string("missing event element"));
        throw invalid_argument(message);
    }

    m_TempButtonMapper.SetAxis(ControlMapper::GetAxisProps(id));
	  m_TempButtonMapper.SetLabel(label);
    m_TempButtonMapper.SetDevice(m_TempDevice);
    m_TempButtonMapper.SetEvent(m_TempEvent);
    m_TempConfiguration.GetButtonMapperList()->push_back(m_TempButtonMapper);
}

void XmlReader::ProcessAxisMapElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_AXIS))
            {
                ProcessAxisElement(cur_node);
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }
}

void XmlReader::ProcessButtonMapElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_BUTTON))
            {
                ProcessButtonElement(cur_node);
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }
}

void XmlReader::ProcessTriggerElement(xmlNode * a_node)
{
    string device_type;
    string device_id;
    string device_name;
    string button_id;
    string switch_back;
    unsigned short delay;
    char * prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    device_type = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    device_id = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
    device_name = string(prop?prop:"");
    xmlFree(prop);

    CheckDevice(device_type, device_name, device_id);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_BUTTON_ID);
    button_id = string(prop?prop:"");
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SWITCH_BACK);
    switch_back = string(prop?prop:"no");
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DELAY);
    if(prop)
    {
        delay = atoi(prop);
    }
    else
    {
        delay = 0;
    }
    xmlFree(prop);

    m_TempTrigger.SetDevice(Device(device_type, device_id, device_name));
    m_TempTrigger.SetEvent(Event(button_id));
    m_TempTrigger.SetSwitchBack(switch_back);
    m_TempTrigger.SetDelay(delay);

    m_TempConfiguration.SetTrigger(m_TempTrigger);
}

void XmlReader::ProcessMouseOptionsElement(xmlNode * node)
{
    string device_id1;
    string device_name1;
    string mode;
    string buffer;
    string filter;
    char * prop;

    prop = (char*)xmlGetProp(node, (xmlChar*) X_ATTR_ID);
    device_id1 = string(prop?prop:"");
    xmlFree(prop);
    prop = (char*)xmlGetProp(node, (xmlChar*) X_ATTR_NAME);
    device_name1 = string(prop?prop:"");
    xmlFree(prop);
    CheckDevice("mouse", device_name1, device_id1);
    m_TempMouseOptions.SetMouse(Device("mouse", device_id1, device_name1));

    prop = (char*)xmlGetProp(node, (xmlChar*) X_ATTR_MODE);
    mode = string(prop?prop:"Aiming");
    if(mode.empty()) mode = "Aiming";//Work-around empty mode bug.
    xmlFree(prop);
    m_TempMouseOptions.SetMode(mode);

    prop = (char*)xmlGetProp(node, (xmlChar*) X_ATTR_BUFFERSIZE);
    buffer = string(prop?prop:"");
    xmlFree(prop);
    m_TempMouseOptions.SetBufferSize(buffer);
    prop = (char*)xmlGetProp(node, (xmlChar*) X_ATTR_FILTER);
    filter = string(prop?prop:"");
    xmlFree(prop);
    m_TempMouseOptions.SetFilter(filter);

    m_TempConfiguration.GetMouseOptionsList()->push_back(m_TempMouseOptions);
}

void XmlReader::ProcessMouseOptionsListElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
      if (cur_node->type == XML_ELEMENT_NODE)
      {
          if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_MOUSE))
          {
              ProcessMouseOptionsElement(cur_node);
          }
          else
          {
              break;
          }
      }
  }
}

void XmlReader::ProcessIntensityElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    string control;
    unsigned char dead_zone;
    string shape;
    unsigned char steps;
    string device_type;
    string device_id;
    string device_name;
    string button_id;
    char * prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_CONTROL);
    control = string(prop?prop:"");
    xmlFree(prop);
    m_TempIntensity.SetAxis(Intensity::GetAxisProps(control));
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DEADZONE);
    dead_zone = atoi(prop);
    xmlFree(prop);
    m_TempIntensity.SetDeadZone(dead_zone);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
    shape = string(prop?prop:"");
    xmlFree(prop);
    m_TempIntensity.SetShape(shape);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_STEPS);
    steps = atoi(prop);
    xmlFree(prop);
    m_TempIntensity.SetSteps(steps);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_UP))
            {
              m_TempIntensity.SetDirection("Increase");
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DOWN))
            {
              m_TempIntensity.SetDirection("Decrease");
            }
            else
            {
              string message(string("bad element name: ") + string((char*)cur_node->name));
              throw invalid_argument(message);
            }
            
            prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_TYPE);
            device_type = string(prop?prop:"");
            xmlFree(prop);
            prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_ID);
            device_id = string(prop?prop:"");
            xmlFree(prop);
            prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_NAME);
            device_name = string(prop?prop:"");
            xmlFree(prop);
            prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_BUTTON_ID);
            button_id = string(prop?prop:"");
            xmlFree(prop);

            if(!button_id.empty())
            {
              m_TempIntensity.SetDevice(Device(device_type, device_id, device_name));
              m_TempIntensity.SetEvent(Event(button_id));
              
              if(control == "left_stick")
              {
                m_TempIntensity.SetAxis(Intensity::GetAxisProps("lstick"));
                m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
              }
              else if(control == "right_stick")
              {
                m_TempIntensity.SetAxis(Intensity::GetAxisProps("rstick"));
                m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
              }
              else
              {
                m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
              }
            }
        }
    }
}

void XmlReader::ProcessIntensityListElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
      if (cur_node->type == XML_ELEMENT_NODE)
      {
          if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY))
          {
              ProcessIntensityElement(cur_node);
          }
          else
          {
              break;
          }
      }
  }
}

void XmlReader::ProcessCorrectionElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LOW_VALUE);
    m_TempJoystickCorrection.SetLowValue(string(prop?prop:"0"));
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LOW_COEF);
    m_TempJoystickCorrection.SetLowCoef(string(prop?prop:"1"));
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_HIGH_VALUE);
    m_TempJoystickCorrection.SetHighValue(string(prop?prop:"0"));
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_HIGH_COEF);
    m_TempJoystickCorrection.SetHighCoef(string(prop?prop:"1"));
    xmlFree(prop);

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
            {
                ProcessDeviceElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message(string("missing device element"));
        throw invalid_argument(message);
    }

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
            {
                ProcessEventElement(cur_node);
                break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message("missing event element");
        throw invalid_argument(message);
    }

    m_TempJoystickCorrection.SetJoystick(m_TempDevice);
    m_TempJoystickCorrection.SetAxis(m_TempEvent);

    m_TempConfiguration.GetJoystickCorrectionsList()->push_back(m_TempJoystickCorrection);
}

void XmlReader::ProcessJoystickCorrectionsListElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
      if (cur_node->type == XML_ELEMENT_NODE)
      {
          if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CORRECTION))
          {
              ProcessCorrectionElement(cur_node);
          }
          else
          {
              break;
          }
      }
  }
}

void XmlReader::ProcessInversionElement(xmlNode * a_node)
{
  char * prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ENABLE);
  m_TempForceFeedback.setInversion(string(prop ? prop : "no"));
}

void XmlReader::ProcessForceFeedbackElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;

    int hasDevice = 0, hasInversion = 0;

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
            {
                hasDevice = 1;
                ProcessDeviceElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INVERSION))
            {
                hasInversion = 1;
                ProcessInversionElement(cur_node);
            }
        }
    }
    
    if (hasDevice == 0)
    {
        string message("missing device element");
        throw invalid_argument(message);
    }

    if (hasInversion == 0)
    {
        string message("missing inversion element");
        throw invalid_argument(message);
    }

    m_TempForceFeedback.SetJoystick(m_TempDevice);

    m_TempConfiguration.SetForceFeedback(m_TempForceFeedback);
}

void XmlReader::ProcessConfigurationElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    unsigned int config_index;
    stringstream ss;
    string id;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"");
    xmlFree(prop);

    ss << id;
    ss >> config_index;
    config_index--;

    if(config_index >= MAX_CONFIGURATIONS)
    {
        string message("bad configuration id: " + id);
        throw invalid_argument(message);
    }

    m_TempConfiguration = Configuration(); // clear everything

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_TRIGGER))
            {
                ProcessTriggerElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_MOUSEOPTIONS_LIST))
            {
                ProcessMouseOptionsListElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY_LIST))
            {
                ProcessIntensityListElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_BUTTON_MAP))
            {
                ProcessButtonMapElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_AXIS_MAP))
            {
                ProcessAxisMapElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_JOYSTICK_CORRECTIONS_LIST))
            {
                ProcessJoystickCorrectionsListElement(cur_node);
            }
            else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_FORCE_FEEDBACK))
            {
                ProcessForceFeedbackElement(cur_node);
            }
        }
    }

    m_TempController.SetConfiguration(m_TempConfiguration, config_index);
}

void XmlReader::ProcessControllerElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    unsigned int controller_index;
    stringstream ss1, ss2;
    string id, dpi, ctype;
    unsigned int idpi;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"-1");
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DPI);
    dpi = string(prop?prop:"0");
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    ctype = string(prop?prop:"");
    xmlFree(prop);

    ss1 << id;
    ss1 >> controller_index;
    controller_index--;

    if(controller_index >= MAX_CONTROLLERS)
    {
        string message("bad controller id: " + id);
        throw invalid_argument(message);
    }

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CONFIGURATION))
            {
                ProcessConfigurationElement(cur_node);
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    ss2 << dpi;
    ss2 >> idpi;
    m_TempController.SetMouseDPI(idpi);

    e_controller_type type = controller_get_type(ctype.c_str());
    if (type == C_TYPE_NONE)
    {
      type = C_TYPE_SIXAXIS;
    }

    m_TempController.SetControllerType(type);

    m_TempConfigurationFile.SetController(m_TempController, controller_index);
}

void XmlReader::ProcessRootElement(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;

  if (a_node->type == XML_ELEMENT_NODE)
  {
    if (xmlStrEqual(a_node->name, (xmlChar*) X_NODE_ROOT))
    {
      for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
      {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
          if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CONTROLLER))
          {
            ProcessControllerElement(cur_node);
          }
          else
          {
            string message(string("bad element name: ") + string((char*)cur_node->name));
            throw invalid_argument(message);
          }
        }
      }
    }
    else
    {
      string message(string("bad element name: ") + string((char*)a_node->name));
      throw invalid_argument(message);
    }
  }
}

bool XmlReader::MultipleMK()
{
    return !m_name_empty;
}

int XmlReader::ReadConfigFile(string filePath)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    int ret = 0;

    m_info.clear();
    m_error.clear();
    m_name_empty = false;
    m_name_nempty = false;

    if(m_checkDevices)
    {
      if(m_evtcatch->init() < 0)
      {
        m_error = "can't read inputs!";
        return -1;
      }
    }

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    try
    {
        /*parse the file and get the DOM */
        doc = xmlReadFile(filePath.c_str(), NULL, 0);

        if (doc != NULL)
        {
            /*Get the root element node */
            root_element = xmlDocGetRootElement(doc);

            if(root_element != NULL)
            {
                ProcessRootElement(root_element);
                *m_ConfigurationFile = m_TempConfigurationFile;
            }
            else
            {
                throw invalid_argument("error: no root element");
            }
        }
        else
        {
            throw invalid_argument("error: could not parse file ");
        }
    }
    catch(exception& e)
    {
        m_error = e.what();
        ret = -1;
    }

    if(m_checkDevices)
    {
      m_evtcatch->clean();
    }

    if(m_name_empty && m_name_nempty)
    {
      m_info.append(EMPTY_NAME_MSG);
    }

    if(!ret && !m_info.empty())
    {
      ret = 1;
    }

    /*free the document */
    xmlFreeDoc(doc);
    return ret;
}
