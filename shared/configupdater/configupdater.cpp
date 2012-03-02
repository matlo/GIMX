
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "configupdater.h"

#ifdef WIN32
#include <windows.h>
#define WGET_CMD "wget.exe -q -w 0 -t 1 -T 10 "
#else
#define WGET_CMD "wget -q -w 0 -t 1 -T 10 "
#endif

static int exec(string command)
{
#ifdef WIN32
  STARTUPINFOA startupInfo =
  { 0};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInformation;

  char* cmd = strdup(command.c_str());

  BOOL result = CreateProcessA(
      "wget.exe",
      cmd,
      NULL,
      NULL,
      FALSE,
      CREATE_NO_WINDOW,
      NULL,
      NULL,
      &startupInfo,
      &processInformation
  );

  free(cmd);

  WaitForSingleObject(processInformation.hProcess, INFINITE);
  CloseHandle(processInformation.hProcess);

  if(!result)
  {
    return -1;
  }
  return 0;
#else
  return system(command.c_str());
#endif
}

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
  
  if(exec(cmd))
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
  
  remove(configs_file.c_str());

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
    
    if(exec(cmd))
    {
      return -1;
    }
  }
  
  return 0;
}
