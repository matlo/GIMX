#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H

#include <Controller.h>
#include <list>
#include <event_catcher.h>

#define MAX_CONTROLLERS 7

class ConfigurationFile
{
    public:
        ConfigurationFile();
        virtual ~ConfigurationFile();
        ConfigurationFile(const ConfigurationFile& other);
        ConfigurationFile& operator=(const ConfigurationFile& other);
        void ReadConfigFile(wxString filePath);
        bool MultipleMK() { return m_multipleMK; }
        void WriteConfigFile();
        wxString GetFilePath() { return m_FilePath; }
        void SetFilePath(wxString val) { m_FilePath = val; }
        Controller* GetController(unsigned int i) { return m_Controllers+i; }
        void SetController(Controller val, unsigned int i) { m_Controllers[i] = val; }
        void SetEvCatch(event_catcher* e) { m_evcatch = e; }
    protected:
    private:
        wxString m_FilePath;
        Controller m_Controllers[MAX_CONTROLLERS];
        event_catcher* m_evcatch;
        bool m_multipleMK;
};

#endif // CONFIGURATIONFILE_H
