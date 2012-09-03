#include "XmlReader.h"
#include <stdexcept>
#include <sstream>

using namespace std;

#define EMPTY_NAME_MSG "A device name is empty. Multiple mice and keyboards are not managed."

XmlReader::XmlReader()
{
    //ctor
    m_ConfigurationFile = NULL;
    m_evtcatch = NULL;
}

XmlReader::XmlReader(ConfigurationFile* configFile)
{
    //ctor
    m_ConfigurationFile = configFile;
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
    m_TempEvent.SetBufferSize(bufferSize);
    m_TempEvent.SetFilter(filter);
}

void XmlReader::ProcessDeviceElement(xmlNode * a_node)
{
    string type;
    string id;
    string name;
    string info = "";
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

    if(name.empty())
    {
      m_name_empty = true;
    }
    else
    {
      m_name_nempty = true;
            
      if(m_evtcatch && !m_evtcatch->check_device(type, name, id))
      {
        info.append(type + " not found: " + name + " (" + id + ")\n");

        if(m_info.find(info) == string::npos)
        {
            m_info.append(info);
        }
      }
    } 

    m_TempDevice.SetType(type);
    m_TempDevice.SetId(id);
    m_TempDevice.SetName(name);
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

    m_TempAxisMapper.SetAxis(id);
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

    m_TempButtonMapper.SetButton(id);
	  m_TempButtonMapper.SetLabel(label);
    m_TempButtonMapper.SetDevice(m_TempDevice);
    m_TempButtonMapper.SetEvent(m_TempEvent);
    m_TempConfiguration.GetButtonMapperList()->push_back(m_TempButtonMapper);
}

void XmlReader::ProcessAxisMapElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    m_TempConfiguration.GetAxisMapperList()->clear();

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
    m_TempConfiguration.GetButtonMapperList()->clear();

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
    m_TempIntensity.SetControl(control);
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
                m_TempIntensity.SetControl("lstick");
                m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
              }
              else if(control == "right_stick")
              {
                m_TempIntensity.SetControl("rstick");
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

    cur_node = a_node->children;

    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_TRIGGER))
            {
                ProcessTriggerElement(cur_node);
                break;
            }
            else
            {
                string message("bad element name: " + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message("missing trigger element");
        throw invalid_argument(message);
    }

    m_TempConfiguration.GetIntensityList()->clear();

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY_LIST))
            {
                ProcessIntensityListElement(cur_node);
            }
            else
            {
                break;
            }
        }
    }

    cur_node = cur_node->prev;

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_BUTTON_MAP))
            {
                ProcessButtonMapElement(cur_node);
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
        string message(string("missing button_map element"));
        throw invalid_argument(message);
    }

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_AXIS_MAP))
            {
                ProcessAxisMapElement(cur_node);
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
        string message(string("missing axis_map element"));
        throw invalid_argument(message);
    }

    m_TempController.SetConfiguration(m_TempConfiguration, config_index);
}

void XmlReader::ProcessControllerElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    unsigned int controller_index;
    stringstream ss1, ss2;
    string id, dpi;
    unsigned int idpi;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = string(prop?prop:"-1");
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DPI);
    dpi = string(prop?prop:"0");
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

    if(m_evtcatch)
    {
      m_evtcatch->init();
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

    if(m_evtcatch)
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
