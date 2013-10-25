/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <sstream>
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
  DWORD ret;
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

static vector<string> &split(const string &s, char delim, vector<string> &elems)
{
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim))
  {
    elems.push_back(item);
  }
  return elems;
}

static vector<string> split(const string &s, char delim)
{
  vector < string > elems;
  return split(s, delim, elems);
}

int updater::checkversion()
{
  string v;
  int major, minor;
  int old_major, old_minor;
  
  if(version_url.empty() || version_file.empty() || version.empty())
  {
    return -1;
  }

#ifdef WIN32
  char temp[MAX_PATH];
  if(!GetTempPathA(sizeof(temp), temp))
  {
    return -1;
  }
#endif

  string cmd = WGET_CMD;
  cmd.append(version_url);
  cmd.append(" -O ");
#ifdef WIN32
  cmd.append(temp);
#endif
  cmd.append(version_file);
  
  if(exec(cmd))
  {
    return -1;
  }
  
  string file;
#ifdef WIN32
  file.append(temp);
#endif
  file.append(version_file);

  ifstream infile;
  infile.open(file.c_str());
  
  if (infile.good())
  {
    infile >> v;
  }
  else
  {
    return -1;
  }

  infile.close();
  
  remove(file.c_str());

  vector<string> elems = split(v, '.');
  if (elems.size() != 2)
  {
    return -1;
  }

  major = atoi(elems[0].c_str());
  minor = atoi(elems[1].c_str());

  vector<string> old_elems = split(version, '.');
  if (old_elems.size() != 2)
  {
    return -1;
  }

  old_major = atoi(old_elems[0].c_str());
  old_minor = atoi(old_elems[1].c_str());

  return major >= old_major && minor > old_minor;
}

int updater::update()
{
  if(download_url.empty())
  {
    return -1;
  }

#ifdef WIN32
  char temp[MAX_PATH];
  if(!GetTempPathA(sizeof(temp), temp))
  {
    return -1;
  }
#endif

  string cmd = WGET_CMD;
  cmd.append(download_url);
  cmd.append(" -O ");
#ifdef WIN32
  cmd.append(temp);
#endif
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

#ifdef WIN32
  cmd.append(temp);
#endif
  cmd.append(download_file);
  char* ccmd = strdup(cmd.c_str());

  BOOL result = CreateProcessA(
      cmd.c_str(),
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
  cmd.append("xdg-open ");
  cmd.append(download_file);
  cmd.append("&");
  
  if(system(cmd.c_str()))
  {
    return -1;
  }
#endif

  return 0;
}
