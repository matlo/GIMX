/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "XmlWritter.h"
#include <string>
#include <string.h>

#ifndef WIN32
#include <pwd.h> //to get the user & group id
#include <unistd.h>
#endif

#include <gimxcontroller/include/controller.h>

using namespace std;

XmlWritter::XmlWritter()
{
    //ctor
    m_ConfigurationFile = NULL;
    m_CurrentController = 0;
    m_CurrentProfile = 0;
}

XmlWritter::XmlWritter(ConfigurationFile* configFile)
{
    //ctor
    m_ConfigurationFile = configFile;
    m_CurrentController = 0;
    m_CurrentProfile = 0;
}

XmlWritter::~XmlWritter()
{
    //dtor
}

void XmlWritter::CreateEventNode(xmlNodePtr parent_node, Event* event)
{
    xmlNodePtr e_node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_EVENT, NULL);
    xmlNewProp(e_node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) event->GetType().c_str());
    xmlNewProp(e_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) event->GetId().c_str());
    if(!strcmp((const char*) parent_node->name, X_NODE_CORRECTION))
    {
      return;
    }
    if(event->GetType().find("axis") != string::npos)
    {
        if(strcmp((const char*) parent_node->name, X_NODE_AXIS))
        {
            xmlNewProp(e_node, BAD_CAST X_ATTR_THRESHOLD, BAD_CAST (const char*) event->GetThreshold().c_str());
        }
        else
        {
            xmlNewProp(e_node, BAD_CAST X_ATTR_DEADZONE, BAD_CAST (const char*) event->GetDeadZone().c_str());
            xmlNewProp(e_node, BAD_CAST X_ATTR_MULTIPLIER, BAD_CAST (const char*) event->GetMultiplier().c_str());
            xmlNewProp(e_node, BAD_CAST X_ATTR_EXPONENT, BAD_CAST (const char*) event->GetExponent().c_str());
            xmlNewProp(e_node, BAD_CAST X_ATTR_SHAPE, BAD_CAST (const char*) event->GetShape().c_str());
        }
    }
}

void XmlWritter::CreateDeviceNode(xmlNodePtr parent_node, Device* device)
{
    xmlNodePtr d_node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_DEVICE, NULL);
    xmlNewProp(d_node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) device->GetType().c_str());
    xmlNewProp(d_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) device->GetId().c_str());
    xmlNewProp(d_node, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) device->GetName().c_str());
}

void XmlWritter::CreateAxisMapNode(xmlNodePtr parent_node)
{
    xmlNodePtr am_node;

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_AXIS_MAP, NULL);
    list<ControlMapper>* am_list = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetAxisMapperList();

    for(list<ControlMapper>::iterator it = am_list->begin(); it!=am_list->end(); ++it)
    {
        am_node = xmlNewChild(node, NULL, BAD_CAST X_NODE_AXIS, NULL);

        xmlNewProp(am_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetGenericAxisName().c_str());
		    xmlNewProp(am_node, BAD_CAST X_ATTR_LABEL, BAD_CAST (const char*) it->GetLabel().c_str());

        CreateDeviceNode(am_node, it->GetDevice());

        CreateEventNode(am_node, it->GetEvent());
    }
}

void XmlWritter::CreateButtonMapNode(xmlNodePtr parent_node)
{
    xmlNodePtr bm_node;

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_BUTTON_MAP, NULL);
    list<ControlMapper>* bm_list = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetButtonMapperList();

    for(list<ControlMapper>::iterator it = bm_list->begin(); it!=bm_list->end(); ++it)
    {
        bm_node = xmlNewChild(node, NULL, BAD_CAST X_NODE_BUTTON, NULL);

        xmlNewProp(bm_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetGenericAxisName().c_str());
		    xmlNewProp(bm_node, BAD_CAST X_ATTR_LABEL, BAD_CAST (const char*) it->GetLabel().c_str());

        CreateDeviceNode(bm_node, it->GetDevice());

        CreateEventNode(bm_node, it->GetEvent());
    }
}

void XmlWritter::CreateTriggerNode(xmlNodePtr parent_node)
{
    char delay[6];

    Trigger* trigger = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetTrigger();

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_TRIGGER, NULL);

    xmlNewProp(node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) trigger->GetDevice()->GetType().c_str());

    xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) trigger->GetDevice()->GetId().c_str());

    xmlNewProp(node, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) trigger->GetDevice()->GetName().c_str());

    xmlNewProp(node, BAD_CAST X_ATTR_BUTTON_ID, BAD_CAST (const char*) trigger->GetEvent()->GetId().c_str());

    xmlNewProp(node, BAD_CAST X_ATTR_SWITCH_BACK, BAD_CAST (const char*) trigger->GetSwitchBack().c_str());

    snprintf(delay, sizeof(delay), "%hu", trigger->GetDelay());
    xmlNewProp(node, BAD_CAST X_ATTR_DELAY, BAD_CAST (const char*) delay);
}

void XmlWritter::CreateMouseOptionsNodes(xmlNodePtr parent_node)
{
    xmlNodePtr pnode = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_MOUSEOPTIONS_LIST, NULL);

    list<MouseOptions>* i_list = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetMouseOptionsList();

    for(list<MouseOptions>::iterator it = i_list->begin(); it!=i_list->end(); ++it)
    {
      xmlNodePtr nodemo = xmlNewChild(pnode, NULL, BAD_CAST X_NODE_MOUSE, NULL);

      xmlNewProp(nodemo, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) it->GetMouse()->GetName().c_str());
      xmlNewProp(nodemo, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetMouse()->GetId().c_str());

      xmlNewProp(nodemo, BAD_CAST X_ATTR_MODE, BAD_CAST (const char*) it->GetMode().c_str());

      xmlNewProp(nodemo, BAD_CAST X_ATTR_BUFFERSIZE, BAD_CAST (const char*) it->GetBufferSize().c_str());
      xmlNewProp(nodemo, BAD_CAST X_ATTR_FILTER, BAD_CAST (const char*) it->GetFilter().c_str());
    }
}

void XmlWritter::CreateJoystickCorrectionsNodes(xmlNodePtr parent_node)
{
    xmlNodePtr corrections = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_JOYSTICK_CORRECTIONS_LIST, NULL);

    list<JoystickCorrection>* i_list = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetJoystickCorrectionsList();

    for(list<JoystickCorrection>::iterator it = i_list->begin(); it!=i_list->end(); ++it)
    {
      xmlNodePtr correction = xmlNewChild(corrections, NULL, BAD_CAST X_NODE_CORRECTION, NULL);

      xmlNewProp(correction, BAD_CAST X_ATTR_LOW_VALUE, BAD_CAST (const char*) it->GetLowValue().c_str());
      xmlNewProp(correction, BAD_CAST X_ATTR_LOW_COEF, BAD_CAST (const char*) it->GetLowCoef().c_str());

      xmlNewProp(correction, BAD_CAST X_ATTR_HIGH_VALUE, BAD_CAST (const char*) it->GetHighValue().c_str());
      xmlNewProp(correction, BAD_CAST X_ATTR_HIGH_COEF, BAD_CAST (const char*) it->GetHighCoef().c_str());

      CreateDeviceNode(correction, it->GetJoystick());
      CreateEventNode(correction, it->GetAxis());
    }
}

void XmlWritter::CreateIntensityNodes(xmlNodePtr parent_node)
{
    char steps[4];
    char dead_zone[4];

    xmlNodePtr pnode = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_INTENSITY_LIST, NULL);

    list<Intensity>* i_list = m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->GetIntensityList();

    for(list<Intensity>::iterator it = i_list->begin(); it!=i_list->end(); ++it)
    {
      if(it->GetDirection() != "Increase" && it->GetDirection() != "Decrease")
      {
        continue;
      }
      
#ifndef WIN32
      snprintf(steps, sizeof(steps), "%hhu", it->GetSteps());
      snprintf(dead_zone, sizeof(dead_zone), "%hhu", it->GetDeadZone());
#else
      snprintf(steps, sizeof(steps), "%hu", it->GetSteps());
      snprintf(dead_zone, sizeof(dead_zone), "%hu", it->GetDeadZone());
#endif

      xmlNodePtr node = xmlNewChild(pnode, NULL, BAD_CAST X_NODE_INTENSITY, NULL);

      xmlNewProp(node, BAD_CAST X_ATTR_CONTROL, BAD_CAST (const char*) it->GetGenericAxisName().c_str());

      xmlNewProp(node, BAD_CAST X_ATTR_DEADZONE, BAD_CAST (const char*) dead_zone);

      xmlNewProp(node, BAD_CAST X_ATTR_SHAPE, BAD_CAST (const char*) it->GetShape().c_str());

      xmlNewProp(node, BAD_CAST X_ATTR_STEPS, BAD_CAST (const char*) steps);

      xmlNodePtr nodedir = NULL;
      
      if(it->GetDirection() == "Increase")
      {
        nodedir = xmlNewChild(node, NULL, BAD_CAST X_NODE_UP, NULL);
      }
      else if(it->GetDirection() == "Decrease")
      {
        nodedir = xmlNewChild(node, NULL, BAD_CAST X_NODE_DOWN, NULL);
      }

      xmlNewProp(nodedir, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) it->GetDevice()->GetType().c_str());

      xmlNewProp(nodedir, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) it->GetDevice()->GetName().c_str());

      xmlNewProp(nodedir, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetDevice()->GetId().c_str());

      xmlNewProp(nodedir, BAD_CAST X_ATTR_BUTTON_ID, BAD_CAST (const char*) it->GetEvent()->GetId().c_str());
    }
}

void XmlWritter::CreateConfigurationNodes(xmlNodePtr parent_node)
{
    xmlNodePtr node;
    char id[2];
    int i;
        
    for(i=0; i<MAX_PROFILES; ++i)
    {
        m_CurrentProfile = i;
        
        if(m_ConfigurationFile->GetController(m_CurrentController)->GetProfile(m_CurrentProfile)->IsEmpty())
        {
          continue;
        }

        node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_CONFIGURATION, NULL);

#ifndef WIN32
        snprintf(id, sizeof(id), "%hhu", i+1);
#else
        snprintf(id, sizeof(id), "%hu", i+1);
#endif

        xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST id);

        CreateTriggerNode(node);

        CreateMouseOptionsNodes(node);

        CreateIntensityNodes(node);

        CreateButtonMapNode(node);

        CreateAxisMapNode(node);

        CreateJoystickCorrectionsNodes(node);
    }
}

void XmlWritter::CreateControllerNodes(xmlNodePtr parent_node)
{
    xmlNodePtr node;
    char id[2];
    char dpi[5];
    int i;

    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
        m_CurrentController = i;
        
        if(m_ConfigurationFile->GetController(m_CurrentController)->IsEmpty())
        {
          continue;
        }

        node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_CONTROLLER, NULL);

#ifndef WIN32
        snprintf(id, sizeof(id), "%hhu", i+1);
#else
        snprintf(id, sizeof(id), "%hu", i+1);
#endif

        xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST id);

        snprintf(dpi, sizeof(dpi), "%u", m_ConfigurationFile->GetController(m_CurrentController)->GetMouseDPI());

        xmlNewProp(node, BAD_CAST X_ATTR_DPI, BAD_CAST dpi);

        const char* cname = controller_get_name(m_ConfigurationFile->GetController(m_CurrentController)->GetControllerType());

        xmlNewProp(node, BAD_CAST X_ATTR_TYPE, BAD_CAST cname);

        CreateConfigurationNodes(node);
    }
}

int XmlWritter::WriteConfigFile()
{
    int ret;
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL;/* node pointers */

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST X_NODE_ROOT);
    xmlDocSetRootElement(doc, root_node);

    CreateControllerNodes(root_node);

    /*
     * Dumping document to stdio or file
     */
    ret = xmlSaveFormatFileEnc(m_ConfigurationFile->GetFilePath().c_str(), doc, "UTF-8", 1);

#ifndef WIN32
    if(ret != -1 
    && chown(m_ConfigurationFile->GetFilePath().c_str(), getpwuid(getuid())->pw_uid, getpwuid(getuid())->pw_gid) < 0)
    {
      ret = -1;
    }
#endif

    /*free the document */
    xmlFreeDoc(doc);

    return ret;
}
