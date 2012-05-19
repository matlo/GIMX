#include "ConfigurationFile.h"
#include <XmlReader.h>
#include <XmlWritter.h>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <iomanip>

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
      Controller* refController = configFile.GetController(i);
      Controller* controller = GetController(i);
      for(int j=0; j<MAX_CONFIGURATIONS; ++j)
      {
        Configuration* config1 = refController->GetConfiguration(j);
        Configuration* config2 = controller->GetConfiguration(j);

        config2->SetTrigger(*config1->GetTrigger());
        config2->SetIntensityList(*config1->GetIntensityList());

        list<ButtonMapper>* buttonMappers1 = config1->GetButtonMapperList();
        list<ButtonMapper>* buttonMappers2 = config2->GetButtonMapperList();

        for(list<ButtonMapper>::iterator it2 = buttonMappers2->begin(); it2!=buttonMappers2->end(); it2++)
        {
          string label2 = it2->GetLabel();

          transform(label2.begin(), label2.end(), label2.begin(), (int(*)(int)) tolower);

          if(!label2.empty())
          {
            for(std::list<ButtonMapper>::iterator it1 = buttonMappers1->begin(); it1!=buttonMappers1->end(); it1++)
            {
              string label1 = it1->GetLabel();

              transform(label1.begin(), label1.end(), label1.begin(), (int(*)(int)) tolower);

              if(label1 == label2)
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

          transform(label2.begin(), label2.end(), label2.begin(), (int(*)(int)) tolower);

          if(!label2.empty())
          {
            for(std::list<AxisMapper>::iterator it1 = axisMappers1->begin(); it1!=axisMappers1->end(); it1++)
            {
              string label1 = it1->GetLabel();

              transform(label1.begin(), label1.end(), label1.begin(), (int(*)(int)) tolower);

              if(label1 == label2)
              {
                it2->SetEvent(*it1->GetEvent());
                break;
              }
            }
          }
        }
      }
    }
  }

  return ret;
}

int ConfigurationFile::ConvertSensitivity(string refFilePath)
{
  ConfigurationFile configFile;

  int ret = configFile.ReadConfigFile(refFilePath);

  if(ret >= 0)
  {
    for(int i=0; i<MAX_CONTROLLERS; ++i)
    {
      Controller* refController = configFile.GetController(i);
      Controller* controller = GetController(i);

      int dpi1 = refController->GetMouseDPI();
      int dpi2 = controller->GetMouseDPI();

      if(dpi1 && dpi2 && dpi1 != dpi2)
      {
        for(int k=0; k<MAX_CONFIGURATIONS; ++k)
        {
          Configuration* config = controller->GetConfiguration(k);

          list<AxisMapper>* axisMappers = config->GetAxisMapperList();

          for(list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
          {
            if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis")
            {
                double val = atof(it->GetEvent()->GetMultiplier().c_str());
                double exp = atof(it->GetEvent()->GetExponent().c_str());
                val = val * pow((double)dpi2 / dpi1, exp);
                ostringstream ios;
                ios << setprecision(2) << val;
                it->GetEvent()->SetMultiplier(ios.str());
            }
          }
        }

        controller->SetMouseDPI(dpi1);
      }
    }
  }

  return ret;

}
