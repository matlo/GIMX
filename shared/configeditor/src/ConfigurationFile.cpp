#include "ConfigurationFile.h"
#include <XmlReader.h>
#include <XmlWritter.h>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <iterator>

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

list<string> &split(const string &s, char delim, list<string> &elems)
{
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim))
  {
    while (item.find(" ") == 0)
    {
      item = item.substr(1);
    }
    elems.push_back(item);
  }
  return elems;
}

list<string> split(const string &s, char delim)
{
  list < string > elems;
  return split(s, delim, elems);
}

template <class T>
static void AutoBindMappers(list<T>* refMappers, list<T>* modMappers)
{
  for(typename list<T>::iterator itModMappers = modMappers->begin(); itModMappers!=modMappers->end(); itModMappers++)
  {
    string modLabel = itModMappers->GetLabel();

    transform(modLabel.begin(), modLabel.end(), modLabel.begin(), (int(*)(int)) tolower);

    list<string> modTokens = split(modLabel, ',');

    for(list<string>::iterator itModTokens = modTokens.begin(); itModTokens != modTokens.end(); itModTokens++)
    {
      if(!itModTokens->empty())
      {
        typename list<T>::iterator itRefMappers;
        for(itRefMappers = refMappers->begin(); itRefMappers!=refMappers->end(); itRefMappers++)
        {
          string refLabel = itRefMappers->GetLabel();

          transform(refLabel.begin(), refLabel.end(), refLabel.begin(), (int(*)(int)) tolower);

          list<string> refTokens = split(refLabel, ',');

          if(find(refTokens.begin(), refTokens.end(), *itModTokens) != refTokens.end())
          {
            itModMappers->SetDevice(*itRefMappers->GetDevice());
            itModMappers->SetEvent(*itRefMappers->GetEvent());
            break;
          }
        }
        if(itRefMappers != refMappers->end())
        {
          break;
        }
      }
    }
  }
}

int ConfigurationFile::AutoBind(string refFilePath)
{
  ConfigurationFile refConfigFile;

  int ret = refConfigFile.ReadConfigFile(refFilePath);

  if(ret >= 0)
  {
    for(int i=0; i<MAX_CONTROLLERS; ++i)
    {
      Controller* refController = refConfigFile.GetController(i);
      Controller* modController = GetController(i);
      for(int j=0; j<MAX_CONFIGURATIONS; ++j)
      {
        Configuration* refConfig = refController->GetConfiguration(j);
        Configuration* modConfig = modController->GetConfiguration(j);

        modConfig->SetTrigger(*refConfig->GetTrigger());
        modConfig->SetIntensityList(*refConfig->GetIntensityList());

        AutoBindMappers<ButtonMapper>(refConfig->GetButtonMapperList(), modConfig->GetButtonMapperList());

        AutoBindMappers<AxisMapper>(refConfig->GetAxisMapperList(), modConfig->GetAxisMapperList());
      }
    }
  }

  return ret;
}

int ConfigurationFile::ConvertSensitivity(string refFilePath)
{
  ConfigurationFile refConfigFile;

  int ret = refConfigFile.ReadConfigFile(refFilePath);

  if(ret >= 0)
  {
    for(int i=0; i<MAX_CONTROLLERS; ++i)
    {
      Controller* refController = refConfigFile.GetController(i);
      Controller* modController = GetController(i);

      int refdpi = refController->GetMouseDPI();
      int dpi = modController->GetMouseDPI();

      if(refdpi && dpi && refdpi != dpi)
      {
        for(int k=0; k<MAX_CONFIGURATIONS; ++k)
        {
          Configuration* modConfig = modController->GetConfiguration(k);

          list<AxisMapper>* modAxisMappers = modConfig->GetAxisMapperList();

          for(list<AxisMapper>::iterator itModAxisMappers = modAxisMappers->begin(); itModAxisMappers!=modAxisMappers->end(); itModAxisMappers++)
          {
            if(itModAxisMappers->GetDevice()->GetType() == "mouse" && itModAxisMappers->GetEvent()->GetType() == "axis")
            {
                double val = atof(itModAxisMappers->GetEvent()->GetMultiplier().c_str());
                double exp = atof(itModAxisMappers->GetEvent()->GetExponent().c_str());
                val = val * pow((double)dpi / refdpi, exp);
                ostringstream ios;
                ios << setprecision(2) << val;
                itModAxisMappers->GetEvent()->SetMultiplier(ios.str());
            }
          }
        }

        modController->SetMouseDPI(refdpi);
      }
    }
  }

  return ret;

}
