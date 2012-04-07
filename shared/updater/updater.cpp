
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "updater.h"

#ifdef WIN32
#include <windows.h>
#define WGET_CMD "wget.exe -q -w 0 -t 1 -T 10 "
#else
#define WGET_CMD "wget -q -w 0 -t 1 -T 10 "
#endif

static int exec(string command)
{
#ifdef WIN32
  unsigned int ret;
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

  if(!result)
  {
    ret = -1;
  }
  else
  {
    if(!GetExitCodeProcess(processInformation.hProcess, &ret))
    {
      ret = -1;
    }
  }

  CloseHandle(processInformation.hProcess);

  return ret;
#else
  return system(command.c_str());
#endif
}

int updater::checkversion()
{
  string v;
  
  if(version_url.empty() || version_file.empty() || version.empty())
  {
    return -1;
  }
  
  string cmd = WGET_CMD;
  cmd.append(version_url);
  cmd.append(" -O ");
  cmd.append(version_file);
  
  if(exec(cmd))
  {
    return -1;
  }
  
  ifstream infile;  
  infile.open(version_file.c_str());
  
  if (infile.good())
  {
    infile >> v;
  }
  else
  {
    return -1;
  }

  infile.close();
  
  remove(version_file.c_str());

  return v > version;
}

int updater::update()
{
  if(download_url.empty())
  {
    return -1;
  }
  
  string cmd = WGET_CMD;
  cmd.append(download_url);
  cmd.append(" -O ");
  cmd.append(download_file);
  
  if(exec(cmd))
  {
    return -1;
  }
  
  cmd.clear();
#ifdef WIN32
  STARTUPINFOA startupInfo =
  { 0};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInformation;
  
  cmd.append(download_file);
  char* ccmd = strdup(cmd.c_str());

  BOOL result = CreateProcessA(
      download_file.c_str(),
      ccmd,
      NULL,
      NULL,
      FALSE,
      NORMAL_PRIORITY_CLASS,
      NULL,
      NULL,
      &startupInfo,
      &processInformation
  );

  free(ccmd);
  
  if(!result)
  {
    return -1;
  }
#else
  cmd.append("gnome-open ");
  cmd.append(download_file);
  cmd.append("&");
  
  if(system(cmd.c_str()))
  {
    return -1;
  }
#endif

  return 0;
}
