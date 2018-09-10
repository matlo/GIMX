/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIGURATIONFILE_H

#define CONFIGURATIONFILE_H

#include "Controller.h"
#include <list>
#include "EventCatcher.h"

#define MAX_CONTROLLERS 7
#define MAX_DPI 20000

class ConfigurationFile
{
    public:
        ConfigurationFile();
        virtual ~ConfigurationFile();
        ConfigurationFile& operator=(const ConfigurationFile& other);
        int ReadConfigFile(const string& directory, const string& file);
        bool MultipleMK() { return m_multipleMK; }
        void SetCheckDevices(bool check) { m_checkDevices = check; }
        string GetError() { return m_Error; }
        string GetInfo() { return m_Info; }
        int WriteConfigFile(const string& directory, const string& file);
        string GetFile() const { return m_File; }
        string GetDirectory() const { return m_Directory; }
        void SetFile(const string& val) { m_File = val; }
        void SetDirectory(const string& val) { m_Directory = val; }
        Controller* GetController(unsigned int i) { return m_Controllers+i; }
        void SetController(Controller val, unsigned int i) { m_Controllers[i] = val; }
        int AutoBind(const string& directory, const string& file);
        int ConvertSensitivity(const string& directory, const string& file);
        void GetLabels(list<string>&, list<string>&);
        static void GetLabels(const string& directory, const string& file, list<string>& button_labels, list<string>& axis_labels);
        bool IsEmpty();
        bool operator==(const ConfigurationFile &other) const;
    protected:
    private:
        string m_File;
        string m_Directory;
        string m_Error;
        string m_Info;
        Controller m_Controllers[MAX_CONTROLLERS];
        bool m_multipleMK;
        bool m_checkDevices;
};

#endif // CONFIGURATIONFILE_H
