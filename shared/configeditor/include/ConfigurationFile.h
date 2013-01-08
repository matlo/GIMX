/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H

#include <Controller.h>
#include <list>
#include <event_catcher.h>

#define MAX_CONTROLLERS 7
#define MAX_DPI 20000

class ConfigurationFile
{
    public:
        ConfigurationFile();
        virtual ~ConfigurationFile();
        ConfigurationFile(const ConfigurationFile& other);
        ConfigurationFile& operator=(const ConfigurationFile& other);
        int ReadConfigFile(string filePath);
        bool MultipleMK() { return m_multipleMK; }
        string GetError() { return m_Error; }
        string GetInfo() { return m_Info; }
        int WriteConfigFile();
        string GetFilePath() { return m_FilePath; }
        void SetFilePath(string val) { m_FilePath = val; }
        Controller* GetController(unsigned int i) { return m_Controllers+i; }
        void SetController(Controller val, unsigned int i) { m_Controllers[i] = val; }
        void SetEvCatch(event_catcher* e) { m_evcatch = e; }
        int AutoBind(string refFilePath);
        int ConvertSensitivity(string refFilePath);
        void GetLabels(list<string>&, list<string>&);
        static void GetLabels(string, list<string>&, list<string>&);
        bool IsEmpty();
    protected:
    private:
        string m_FilePath;
        string m_Error;
        string m_Info;
        Controller m_Controllers[MAX_CONTROLLERS];
        event_catcher* m_evcatch;
        bool m_multipleMK;
};

#endif // CONFIGURATIONFILE_H
