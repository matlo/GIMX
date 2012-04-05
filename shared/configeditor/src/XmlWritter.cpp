#include "XmlWritter.h"

XmlWritter::XmlWritter()
{
    //ctor
    m_ConfigurationFile = NULL;
}

XmlWritter::XmlWritter(ConfigurationFile* configFile)
{
    //ctor
    m_ConfigurationFile = configFile;
    m_CurrentController = 0;
    m_CurrentConfiguration = 0;
}

XmlWritter::~XmlWritter()
{
    //dtor
}

void XmlWritter::CreateEventNode(xmlNodePtr parent_node, Event* event)
{
    xmlNodePtr e_node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_EVENT, NULL);
    xmlNewProp(e_node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) event->GetType().mb_str(wxConvUTF8));
    xmlNewProp(e_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) event->GetId().mb_str(wxConvUTF8));
    if(event->GetType() == _("axis"))
    {
        if(strcmp((const char*) parent_node->name, X_NODE_AXIS))
        {
            xmlNewProp(e_node, BAD_CAST X_ATTR_THRESHOLD, BAD_CAST (const char*) event->GetThreshold().mb_str(wxConvUTF8));
        }
        else
        {
            xmlNewProp(e_node, BAD_CAST X_ATTR_DEADZONE, BAD_CAST (const char*) event->GetDeadZone().mb_str(wxConvUTF8));
            xmlNewProp(e_node, BAD_CAST X_ATTR_MULTIPLIER, BAD_CAST (const char*) event->GetMultiplier().mb_str(wxConvUTF8));
            xmlNewProp(e_node, BAD_CAST X_ATTR_EXPONENT, BAD_CAST (const char*) event->GetExponent().mb_str(wxConvUTF8));
            xmlNewProp(e_node, BAD_CAST X_ATTR_SHAPE, BAD_CAST (const char*) event->GetShape().mb_str(wxConvUTF8));
            xmlNewProp(e_node, BAD_CAST X_ATTR_BUFFERSIZE, BAD_CAST (const char*) event->GetBufferSize().mb_str(wxConvUTF8));
            xmlNewProp(e_node, BAD_CAST X_ATTR_FILTER, BAD_CAST (const char*) event->GetFilter().mb_str(wxConvUTF8));
        }
    }
}

void XmlWritter::CreateDeviceNode(xmlNodePtr parent_node, Device* device)
{
    xmlNodePtr d_node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_DEVICE, NULL);
    xmlNewProp(d_node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) device->GetType().mb_str(wxConvUTF8));
    xmlNewProp(d_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) device->GetId().mb_str(wxConvUTF8));
    xmlNewProp(d_node, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) device->GetName().mb_str(wxConvUTF8));
}

void XmlWritter::CreateAxisMapNode(xmlNodePtr parent_node)
{
    xmlNodePtr am_node;

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_AXIS_MAP, NULL);
    std::list<AxisMapper>* am_list = m_ConfigurationFile->GetController(m_CurrentController)->GetConfiguration(m_CurrentConfiguration)->GetAxisMapperList();

    for(std::list<AxisMapper>::iterator it = am_list->begin(); it!=am_list->end(); it++)
    {
        am_node = xmlNewChild(node, NULL, BAD_CAST X_NODE_AXIS, NULL);

        xmlNewProp(am_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetAxis().mb_str(wxConvUTF8));

        CreateDeviceNode(am_node, it->GetDevice());

        CreateEventNode(am_node, it->GetEvent());
    }
}

void XmlWritter::CreateButtonMapNode(xmlNodePtr parent_node)
{
    xmlNodePtr bm_node;

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_BUTTON_MAP, NULL);
    std::list<ButtonMapper>* bm_list = m_ConfigurationFile->GetController(m_CurrentController)->GetConfiguration(m_CurrentConfiguration)->GetButtonMapperList();

    for(std::list<ButtonMapper>::iterator it = bm_list->begin(); it!=bm_list->end(); it++)
    {
        bm_node = xmlNewChild(node, NULL, BAD_CAST X_NODE_BUTTON, NULL);

        xmlNewProp(bm_node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetButton().mb_str(wxConvUTF8));

        CreateDeviceNode(bm_node, it->GetDevice());

        CreateEventNode(bm_node, it->GetEvent());
    }
}

void XmlWritter::CreateTriggerNode(xmlNodePtr parent_node)
{
    char delay[6];

    Trigger* trigger = m_ConfigurationFile->GetController(m_CurrentController)->GetConfiguration(m_CurrentConfiguration)->GetTrigger();

    xmlNodePtr node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_TRIGGER, NULL);

    xmlNewProp(node, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) trigger->GetDevice()->GetType().mb_str(wxConvUTF8));

    xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) trigger->GetDevice()->GetId().mb_str(wxConvUTF8));

    xmlNewProp(node, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) trigger->GetDevice()->GetName().mb_str(wxConvUTF8));

    xmlNewProp(node, BAD_CAST X_ATTR_BUTTON_ID, BAD_CAST (const char*) trigger->GetEvent()->GetId().mb_str(wxConvUTF8));

    xmlNewProp(node, BAD_CAST X_ATTR_SWITCH_BACK, BAD_CAST (const char*) trigger->GetSwitchBack().mb_str(wxConvUTF8));

    snprintf(delay, sizeof(delay), "%hu", trigger->GetDelay());
    xmlNewProp(node, BAD_CAST X_ATTR_DELAY, BAD_CAST (const char*) delay);
}

void XmlWritter::CreateIntensityNodes(xmlNodePtr parent_node)
{
    char steps[4];
    char dead_zone[4];

    xmlNodePtr pnode = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_INTENSITY_LIST, NULL);

    std::list<Intensity>* i_list = m_ConfigurationFile->GetController(m_CurrentController)->GetConfiguration(m_CurrentConfiguration)->GetIntensityList();

    for(std::list<Intensity>::iterator it = i_list->begin(); it!=i_list->end(); it++)
    {
#ifndef WIN32
      snprintf(steps, sizeof(steps), "%hhu", it->GetSteps());
      snprintf(dead_zone, sizeof(dead_zone), "%hhu", it->GetDeadZone());
#else
      snprintf(steps, sizeof(steps), "%hu", it->GetSteps());
      snprintf(dead_zone, sizeof(dead_zone), "%hu", it->GetDeadZone());
#endif

      xmlNodePtr node = xmlNewChild(pnode, NULL, BAD_CAST X_NODE_INTENSITY, NULL);

      xmlNewProp(node, BAD_CAST X_ATTR_CONTROL, BAD_CAST (const char*) it->GetControl().mb_str(wxConvUTF8));

      xmlNewProp(node, BAD_CAST X_ATTR_DEADZONE, BAD_CAST (const char*) dead_zone);

      xmlNewProp(node, BAD_CAST X_ATTR_SHAPE, BAD_CAST (const char*) it->GetShape().mb_str(wxConvUTF8));

      xmlNewProp(node, BAD_CAST X_ATTR_STEPS, BAD_CAST (const char*) steps);

      xmlNodePtr nodeup = xmlNewChild(node, NULL, BAD_CAST X_NODE_UP, NULL);

      xmlNewProp(nodeup, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) it->GetDeviceUp()->GetType().mb_str(wxConvUTF8));

      xmlNewProp(nodeup, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) it->GetDeviceUp()->GetName().mb_str(wxConvUTF8));

      xmlNewProp(nodeup, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetDeviceUp()->GetId().mb_str(wxConvUTF8));

      xmlNewProp(nodeup, BAD_CAST X_ATTR_BUTTON_ID, BAD_CAST (const char*) it->GetEventUp()->GetId().mb_str(wxConvUTF8));

      xmlNodePtr nodedown = xmlNewChild(node, NULL, BAD_CAST X_NODE_DOWN, NULL);

      xmlNewProp(nodedown, BAD_CAST X_ATTR_TYPE, BAD_CAST (const char*) it->GetDeviceDown()->GetType().mb_str(wxConvUTF8));

      xmlNewProp(nodedown, BAD_CAST X_ATTR_NAME, BAD_CAST (const char*) it->GetDeviceDown()->GetName().mb_str(wxConvUTF8));

      xmlNewProp(nodedown, BAD_CAST X_ATTR_ID, BAD_CAST (const char*) it->GetDeviceDown()->GetId().mb_str(wxConvUTF8));

      xmlNewProp(nodedown, BAD_CAST X_ATTR_BUTTON_ID, BAD_CAST (const char*) it->GetEventDown()->GetId().mb_str(wxConvUTF8));
    }
}

void XmlWritter::CreateConfigurationNodes(xmlNodePtr parent_node)
{
    xmlNodePtr node;
    char id[2];
    int i;

    for(i=0; i<MAX_CONFIGURATIONS; ++i)
    {
        m_CurrentConfiguration = i;

        node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_CONFIGURATION, NULL);

#ifndef WIN32
        snprintf(id, sizeof(id), "%hhu", i+1);
#else
        snprintf(id, sizeof(id), "%hu", i+1);
#endif

        xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST id);

        CreateTriggerNode(node);

        CreateIntensityNodes(node);

        CreateButtonMapNode(node);

        CreateAxisMapNode(node);
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

        node = xmlNewChild(parent_node, NULL, BAD_CAST X_NODE_CONTROLLER, NULL);

#ifndef WIN32
        snprintf(id, sizeof(id), "%hhu", i+1);
#else
        snprintf(id, sizeof(id), "%hu", i+1);
#endif

        xmlNewProp(node, BAD_CAST X_ATTR_ID, BAD_CAST id);

        snprintf(dpi, sizeof(dpi), "%u", m_ConfigurationFile->GetController(m_CurrentController)->GetMouseDPI());

        xmlNewProp(node, BAD_CAST X_ATTR_DPI, BAD_CAST dpi);

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
    ret = xmlSaveFormatFileEnc(m_ConfigurationFile->GetFilePath().mb_str(), doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    return ret;
}
