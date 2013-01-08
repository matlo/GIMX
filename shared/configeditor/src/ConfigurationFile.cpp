/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

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
    if(item.empty())
    {
      continue;
    }
    while (item.find(" ") == 0)
    {
      item = item.substr(1);
    }
    while (item.rfind(" ") == item.size()-1)
    {
      item.replace(item.size()-1, 1, "");
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
  for(typename list<T>::iterator itModMappers = modMappers->begin(); itModMappers!=modMappers->end(); ++itModMappers)
  {
    bool bound = false;
    string modLabel = itModMappers->GetLabel();

    transform(modLabel.begin(), modLabel.end(), modLabel.begin(), (int(*)(int)) tolower);

    list<string> modTokens = split(modLabel, ',');

    for(list<string>::iterator itModTokens = modTokens.begin(); itModTokens != modTokens.end(); ++itModTokens)
    {
      if(!itModTokens->empty() && *itModTokens != "not found" && *itModTokens != "duplicate")
      {
        typename list<T>::iterator itRefMappers;
        for(itRefMappers = refMappers->begin(); itRefMappers!=refMappers->end(); ++itRefMappers)
        {
          string refLabel = itRefMappers->GetLabel();

          transform(refLabel.begin(), refLabel.end(), refLabel.begin(), (int(*)(int)) tolower);

          list<string> refTokens = split(refLabel, ',');

          if(find(refTokens.begin(), refTokens.end(), *itModTokens) != refTokens.end())
          {
            itModMappers->SetDevice(*itRefMappers->GetDevice());
            itModMappers->SetEvent(*itRefMappers->GetEvent());
            if(itRefMappers->GetLabel().find(", found") != string::npos)
            {
              if(itModMappers->GetLabel().find(", duplicate") == string::npos)
              {
                itModMappers->SetLabel(itModMappers->GetLabel() + ", duplicate");
              }
            }
            else
            {
              itRefMappers->SetLabel(itRefMappers->GetLabel() + ", found");
            }
            break;
          }
        }
        if(itRefMappers != refMappers->end())
        {
          bound = true;
          break;
        }
      }
    }
    if(!bound && !itModMappers->GetLabel().empty() && itModMappers->GetLabel().find(", not found") == string::npos)
    {
      itModMappers->SetLabel(itModMappers->GetLabel() + ", not found");
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

          for(list<AxisMapper>::iterator itModAxisMappers = modAxisMappers->begin(); itModAxisMappers!=modAxisMappers->end(); ++itModAxisMappers)
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

void ConfigurationFile::GetLabels(list<string>& button_labels, list<string>& axis_labels)
{
  for(int i=0; i<MAX_CONTROLLERS; ++i)
  {
    Controller* controller = GetController(i);

    for(int k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      list<ButtonMapper>* buttonMappers = config->GetButtonMapperList();

      for(list<ButtonMapper>::iterator itButtonMappers = buttonMappers->begin(); itButtonMappers!=buttonMappers->end(); ++itButtonMappers)
      {
        string label = itButtonMappers->GetLabel();
        if(!label.empty())
        {
          list<string> tokens = split(label, ',');
          for(list<string>::iterator tk_it = tokens.begin(); tk_it != tokens.end(); ++tk_it)
          {
            if(!tk_it->empty() && *tk_it != "not found" && *tk_it != "duplicate")
            {
              string tk = *tk_it;
              transform(tk.begin(), tk.end(), tk.begin(), (int(*)(int)) tolower);
              list<string>::iterator bl_it;
              for(bl_it = button_labels.begin(); bl_it != button_labels.end(); ++bl_it)
              {
                string bl = *bl_it;
                transform(bl.begin(), bl.end(), bl.begin(), (int(*)(int)) tolower);
                if(tk == bl)
                {
                  break;
                }
              }
              if(bl_it == button_labels.end())
              {
                button_labels.push_back(*tk_it);
              }
            }
          }
        }
      }

      list<AxisMapper>* axisMappers = config->GetAxisMapperList();

      for(list<AxisMapper>::iterator itAxisMappers = axisMappers->begin(); itAxisMappers!=axisMappers->end(); ++itAxisMappers)
      {
        string label = itAxisMappers->GetLabel();
        if(!label.empty())
        {
          list<string> tokens = split(label, ',');
          for(list<string>::iterator tk_it = tokens.begin(); tk_it != tokens.end(); ++tk_it)
          {
            if(!tk_it->empty() && *tk_it != "not found" && *tk_it != "duplicate")
            {
              string tk = *tk_it;
              transform(tk.begin(), tk.end(), tk.begin(), (int(*)(int)) tolower);
              list<string>::iterator al_it;
              for(al_it = axis_labels.begin(); al_it != axis_labels.end(); ++al_it)
              {
                string al = *al_it;
                transform(al.begin(), al.end(), al.begin(), (int(*)(int)) tolower);
                if(al == tk)
                {
                  break;
                }
              }
              if(al_it == axis_labels.end())
              {
                axis_labels.push_back(*tk_it);
              }
            }
          }
        }
      }
    }
  }
}

void ConfigurationFile::GetLabels(string file, list<string>& button_labels, list<string>& axis_labels)
{
  ConfigurationFile configFile;
  if(configFile.ReadConfigFile(file) >= 0)
  {
    configFile.GetLabels(button_labels, axis_labels);
  }
}

bool ConfigurationFile::IsEmpty()
{
  for(int i =0; i<MAX_CONTROLLERS; ++i)
  {
    if(!m_Controllers[i].IsEmpty())
    {
      return false;
    }
  }
  return true;
}
