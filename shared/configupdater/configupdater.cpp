
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "configupdater.h"

#ifdef WIN32
#include <windows.h>
#endif

#define WGET_CMD "wget -q -w 0 -t 1 -T 10 "

list<string>* configupdater::getconfiglist()
{
  configlist.clear();
  
  if(configs_url.empty() || configs_file.empty() || configs_dir.empty())
  {
    return NULL;
  }
  
  string cmd = WGET_CMD;
  cmd.append(configs_url);
  cmd.append(" -O ");
  cmd.append(configs_file);
  
  if(system(cmd.c_str()))
  {
    return NULL;
  }
  
  ifstream infile;  
  infile.open(configs_file.c_str());
  
  while (infile.good())
  {
    string line;
    getline(infile, line);
    size_t pos1 = line.find(".xml\">");
    size_t pos2 = line.find("</a>");
    if(pos1 != string::npos && pos2 != string::npos)
    {
      configlist.push_back(line.substr(pos1+6, pos2-pos1-6));
    }
  }

  infile.close();
  
  return &configlist;
}

int configupdater::getconfigs(list<string>* cl)
{
  for(list<string>::iterator it = cl->begin(); it != cl->end(); ++it)
  {
    string cmd = WGET_CMD;
    cmd.append(configs_url);
    cmd.append(*it);
    cmd.append(" -O ");
    cmd.append(configs_dir);
    cmd.append(*it);
    
    if(system(cmd.c_str()))
    {
      return -1;
    }
  }
  
  return 0;
}
