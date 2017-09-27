/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XMLWRITTER_H
#define XMLWRITTER_H

#include <XmlReader.h>

class XmlWritter
{
    public:
        XmlWritter();
        XmlWritter(ConfigurationFile* configFile);
        virtual ~XmlWritter();
        int WriteConfigFile();
    protected:
    private:
        void CreateEventNode(xmlNodePtr parent_node, Event* event);
        void CreateDeviceNode(xmlNodePtr parent_node, Device* device);
        void CreateAxisMapNode(xmlNodePtr parent_node);
        void CreateButtonMapNode(xmlNodePtr parent_node);
        void CreateTriggerNode(xmlNodePtr parent_node);
        void CreateMouseOptionsNodes(xmlNodePtr parent_node);
        void CreateIntensityNodes(xmlNodePtr parent_node);
        void CreateConfigurationNodes(xmlNodePtr parent_node);
        void CreateControllerNodes(xmlNodePtr parent_node);
        void CreateJoystickCorrectionsNodes(xmlNodePtr parent_node);
        void CreateForceFeedbackNode(xmlNodePtr parent_node);
        void CreateInversionNode(xmlNodePtr parent_node, ForceFeedback* ffb);
        void CreateGainNode(xmlNodePtr parent_node, ForceFeedback* ffb);
        unsigned int m_CurrentProfile;
        unsigned int m_CurrentController;
        ConfigurationFile* m_ConfigurationFile;
};

#endif // XMLWRITTER_H
