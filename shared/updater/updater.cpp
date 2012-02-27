
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "updater.h"

#ifdef WIN32
#include <windows.h>
#endif

#define WGET_CMD "wget -q -w 0 -t 1 -T 10 "

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
  
  if(system(cmd.c_str()))
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
  
  if(system(cmd.c_str()))
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
