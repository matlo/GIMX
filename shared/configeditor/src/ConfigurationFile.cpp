#include "ConfigurationFile.h"
#include <XmlReader.h>
#include <XmlWritter.h>

ConfigurationFile::ConfigurationFile()
{
    m_evcatch = NULL;
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

int ConfigurationFile::ReadConfigFile(string filePath)
{
    int ret;

    XmlReader reader(this);

    reader.SetEvtCatch(m_evcatch);

    ret = reader.ReadConfigFile(filePath);

    if(ret < 0)
    {
      m_Error = reader.GetError();
    }
    else if(ret > 0)
    {
      m_Info = reader.GetInfo();
    }

    m_multipleMK = reader.MultipleMK();

    m_FilePath = filePath;

    return ret;
}

int ConfigurationFile::WriteConfigFile()
{
    XmlWritter writer(this);

    return writer.WriteConfigFile();
}

int ConfigurationFile::AutoBind(string refFilePath)
{
  ConfigurationFile configFile;

  int ret = configFile.ReadConfigFile(refFilePath);

  if(ret >= 0)
  {
    for(int i=0; i<MAX_CONTROLLERS; ++i)
    {
      Controller* controller1 = configFile.GetController(i);
      Controller* controller2 = GetController(i);
      for(int j=0; j<MAX_CONFIGURATIONS; ++j)
      {
        Configuration* config1 = controller1->GetConfiguration(j);
        Configuration* config2 = controller2->GetConfiguration(j);

        config2->SetTrigger(*config1->GetTrigger());
        config2->SetIntensityList(*config1->GetIntensityList());

        list<ButtonMapper>* buttonMappers1 = config1->GetButtonMapperList();
        list<ButtonMapper>* buttonMappers2 = config2->GetButtonMapperList();

        for(list<ButtonMapper>::iterator it2 = buttonMappers2->begin(); it2!=buttonMappers2->end(); it2++)
        {
          string label2 = it2->GetLabel();
          if(!label2.empty())
          {
            for(std::list<ButtonMapper>::iterator it1 = buttonMappers1->begin(); it1!=buttonMappers1->end(); it1++)
            {
             if(it1->GetLabel() == label2)
             {
               it2->SetEvent(*it1->GetEvent());
               break;
             }
            }
          }
        }

        list<AxisMapper>* axisMappers1 = config1->GetAxisMapperList();
        list<AxisMapper>* axisMappers2 = config2->GetAxisMapperList();

        for(list<AxisMapper>::iterator it2 = axisMappers2->begin(); it2!=axisMappers2->end(); it2++)
        {
          string label2 = it2->GetLabel();
          if(!label2.empty())
          {
            for(std::list<AxisMapper>::iterator it1 = axisMappers1->begin(); it1!=axisMappers1->end(); it1++)
            {
             if(it1->GetLabel() == label2)
             {
               it2->SetEvent(*it1->GetEvent());
               break;
             }
            }
          }
        }
      }
    }

    ret = configFile2.WriteConfigFile();
  }

  return ret;
}
