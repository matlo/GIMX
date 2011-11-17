#include "ConfigurationFile.h"
#include <XmlReader.h>
#include <XmlWritter.h>

ConfigurationFile::ConfigurationFile()
{
    //ctor
}

ConfigurationFile::~ConfigurationFile()
{
    //dtor
}

ConfigurationFile::ConfigurationFile(const ConfigurationFile& other)
{
    //copy ctor
}

ConfigurationFile& ConfigurationFile::operator=(const ConfigurationFile& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    for(unsigned int i=0; i<sizeof(m_Controllers)/sizeof(Controller); ++i)
    {
        m_Controllers[i] = rhs.m_Controllers[i];
    }
    return *this;
}

void ConfigurationFile::ReadConfigFile(wxString filePath)
{
    XmlReader reader(this);

    reader.ReadConfigFile(filePath);

    m_FilePath = filePath;
}

void ConfigurationFile::WriteConfigFile()
{
    XmlWritter writer(this);

    writer.WriteConfigFile();
}
