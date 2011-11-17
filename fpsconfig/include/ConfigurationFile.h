#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H

#include <Controller.h>
#include <list>

#define MAX_CONTROLLERS 7

class ConfigurationFile
{
    public:
        ConfigurationFile();
        virtual ~ConfigurationFile();
        ConfigurationFile(const ConfigurationFile& other);
        ConfigurationFile& operator=(const ConfigurationFile& other);
        void ReadConfigFile(wxString filePath);
        void WriteConfigFile();
        wxString GetFilePath() { return m_FilePath; }
        void SetFilePath(wxString val) { m_FilePath = val; }
        Controller* GetController(unsigned int i) { return m_Controllers+i; }
        void SetController(Controller val, unsigned int i) { m_Controllers[i] = val; }
    protected:
    private:
        wxString m_FilePath;
        Controller m_Controllers[MAX_CONTROLLERS];
};

#endif // CONFIGURATIONFILE_H
