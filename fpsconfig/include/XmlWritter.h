#ifndef XMLWRITTER_H
#define XMLWRITTER_H

#include <XmlReader.h>

class XmlWritter
{
    public:
        XmlWritter();
        XmlWritter(ConfigurationFile* configFile);
        virtual ~XmlWritter();
        void WriteConfigFile();
        void CreateEventNode(xmlNodePtr parent_node, Event* event);
        void CreateDeviceNode(xmlNodePtr parent_node, Device* device);
        void CreateAxisMapNode(xmlNodePtr parent_node);
        void CreateButtonMapNode(xmlNodePtr parent_node);
        void CreateTriggerNode(xmlNodePtr parent_node);
        void CreateIntensityNodes(xmlNodePtr parent_node);
        void CreateConfigurationNodes(xmlNodePtr parent_node);
        void CreateControllerNodes(xmlNodePtr parent_node);
    protected:
    private:
        unsigned int m_CurrentConfiguration;
        unsigned int m_CurrentController;
        ConfigurationFile* m_ConfigurationFile;
};

#endif // XMLWRITTER_H
