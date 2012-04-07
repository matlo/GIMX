#include "XmlReader.h"
#include <stdexcept>
using namespace std;

#define EMPTY_NAME_MSG _("A device name is empty. Multiple mice and keyboards are not managed.")

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
    wxString type;
    wxString id;
    wxString threshold;
    wxString deadZone;
    wxString multiplier;
    wxString exponent;
    wxString shape;
    wxString bufferSize;
    wxString filter;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    type = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_THRESHOLD);
    threshold = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DEADZONE);
    deadZone = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_MULTIPLIER);
    multiplier = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_EXPONENT);
    exponent = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
    if(type == _("axis"))
    {
        if(prop)
        {
            shape = wxString(prop, wxConvUTF8);
        }
        else
        {
            shape = _("Circle");
        }
    }
    else
    {
        shape = _("");
    }
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_BUFFERSIZE);
    if (type == _("axis"))
    {
        if (prop)
        {
            bufferSize = wxString(prop, wxConvUTF8);
        }
        else
        {
            bufferSize = _("1");
        }
    }
    else
    {
        bufferSize = _("");
    }
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_FILTER);
    if (type == _("axis"))
    {
        if (prop)
        {
            filter = wxString(prop, wxConvUTF8);
        }
        else
        {
            filter = _("0.00");
        }
    }
    else
    {
        filter = _("");
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
    wxString type;
    wxString id;
    wxString name;
    wxString info = _("");
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    type = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
    name = wxString(prop, wxConvUTF8);
    xmlFree(prop);

    if(name.IsEmpty())
    {
        info.Append(EMPTY_NAME_MSG);

        if(!m_info.Contains(info))
        {
            m_info.Append(info);
        }
    }
    else if(m_evtcatch && !m_evtcatch->check_device(type, name, id))
    {
        info.Append(type);
        info.Append(_(" not found: "));
        info.Append(name);
        info.Append(_(" "));
        info.Append(id);
        info.Append(_("\n"));

        if(!m_info.Contains(info))
        {
            m_info.Append(info);
        }
    }    

    m_TempDevice.SetType(type);
    m_TempDevice.SetId(id);
    m_TempDevice.SetName(name);
}

void XmlReader::ProcessAxisElement(xmlNode * a_node)
{
    xmlNode* cur_node = NULL;
    wxString id;
    wxString label;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = wxString(prop, wxConvUTF8);
    xmlFree(prop);
	
	  prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LABEL);
    label = wxString(prop, wxConvUTF8);
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

    if(!id.Contains(_("stick")))
    {
        m_TempEvent.SetShape(wxEmptyString);
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
    wxString id;
    wxString label;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = wxString(prop, wxConvUTF8);
    xmlFree(prop);
	
	  prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_LABEL);
    label = wxString(prop, wxConvUTF8);
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
    wxString device_type;
    wxString device_id;
    wxString device_name;
    wxString button_id;
    wxString switch_back;
    unsigned short delay;
    char * prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
    device_type = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    device_id = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
    device_name = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_BUTTON_ID);
    button_id = wxString(prop, wxConvUTF8);
    xmlFree(prop);

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SWITCH_BACK);
    if(prop)
    {
        switch_back = wxString(prop, wxConvUTF8);
    }
    else
    {
        switch_back = _("no");
    }
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
    wxString control;
    unsigned char dead_zone;
    wxString shape;
    unsigned char steps;
    wxString device_type;
    wxString device_id;
    wxString device_name;
    wxString button_id;
    char * prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_CONTROL);
    control = wxString(prop, wxConvUTF8);
    xmlFree(prop);
    m_TempIntensity.SetControl(control);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DEADZONE);
    dead_zone = atoi(prop);
    xmlFree(prop);
    m_TempIntensity.SetDeadZone(dead_zone);
    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
    shape = wxString(prop, wxConvUTF8);
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
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_TYPE);
              device_type = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_ID);
              device_id = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_NAME);
              device_name = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_BUTTON_ID);
              button_id = wxString(prop, wxConvUTF8);
              xmlFree(prop);

              m_TempIntensity.SetDeviceUp(Device(device_type, device_id, device_name));
              m_TempIntensity.SetEventUp(Event(button_id));
              break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DOWN))
            {
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_TYPE);
              device_type = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_ID);
              device_id = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_NAME);
              device_name = wxString(prop, wxConvUTF8);
              xmlFree(prop);
              prop = (char*)xmlGetProp(cur_node, (xmlChar*) X_ATTR_BUTTON_ID);
              button_id = wxString(prop, wxConvUTF8);
              xmlFree(prop);

              m_TempIntensity.SetDeviceDown(Device(device_type, device_id, device_name));
              m_TempIntensity.SetEventDown(Event(button_id));
              break;
            }
            else
            {
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
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
    unsigned char config_index;
    wxString id;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    id = wxString(prop, wxConvUTF8);
    xmlFree(prop);

    config_index = wxAtoi(id) - 1;

    if(config_index >= MAX_CONFIGURATIONS || config_index < 0)
    {
        string message(string("bad configuration id: ") + string(id.mb_str()));
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
                string message(string("bad element name: ") + string((char*)cur_node->name));
                throw invalid_argument(message);
            }
        }
    }

    if(!cur_node)
    {
        string message(string("missing trigger element"));
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

    if(m_TempConfiguration.GetIntensityList()->empty())
    {
      m_TempIntensity.SetControl(_("left_stick"));
      m_TempIntensity.SetDeviceUp(Device(wxEmptyString, wxEmptyString, wxEmptyString));
      m_TempIntensity.SetEventUp(Event(wxEmptyString));
      m_TempIntensity.SetDeviceDown(Device(wxEmptyString, wxEmptyString, wxEmptyString));
      m_TempIntensity.SetEventDown(Event(wxEmptyString));
      m_TempIntensity.SetDeadZone(0);
      m_TempIntensity.SetShape(_("Circle"));
      m_TempIntensity.SetSteps(1);
      m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
      m_TempIntensity.SetControl(_("right_stick"));
      m_TempConfiguration.GetIntensityList()->push_back(m_TempIntensity);
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
    unsigned char controller_index;
    wxString id, dpi;
    char* prop;

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);
    if(prop)
    {
        id = wxString(prop, wxConvUTF8);
        xmlFree(prop);
    }
    else
    {
        id = _("-1");
    }

    prop = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_DPI);
    if(prop)
    {
        dpi = wxString(prop, wxConvUTF8);
        xmlFree(prop);
    }
    else
    {
        dpi = _("0");
    }

    controller_index = wxAtoi(id) - 1;

    if(controller_index >= MAX_CONTROLLERS || controller_index < 0)
    {
        string message(string("bad controller id: ") + string(id.mb_str()));
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

    m_TempController.SetMouseDPI(wxAtoi(dpi));

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
    if(m_info.Contains(EMPTY_NAME_MSG))
    {
      return false;
    }
    else
    {
      return true;
    }
}

void XmlReader::ReadConfigFile(wxString filePath)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    m_info.Empty();

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
        doc = xmlReadFile(filePath.mb_str(), NULL, 0);

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
        wxMessageBox(wxString(e.what(), wxConvUTF8));
    }

    if(m_evtcatch)
    {
      m_evtcatch->clean();
    }
    if(!m_info.IsEmpty())
    {
      wxMessageBox( m_info, wxT("Info"), wxICON_INFORMATION);
    }

    /*free the document */
    xmlFreeDoc(doc);
}
