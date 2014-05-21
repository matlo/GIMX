/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "configupdater.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pwd.h> //to get the user & group id
#include <unistd.h>
#endif

configupdater* configupdater::_singleton = NULL;

#ifdef WIN32
#define CURL_INIT_FLAGS CURL_GLOBAL_WIN32
#else
#define CURL_INIT_FLAGS CURL_GLOBAL_NOTHING
#endif

configupdater::configupdater()
{
  curl_global_init(CURL_INIT_FLAGS);
}

configupdater::~configupdater()
{
  curl_global_cleanup();
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

list<string>* configupdater::getconfiglist()
{
  int ret = -1;
  
  configlist.clear();
  
  if(configs_url.empty() || configs_file.empty())
  {
    return NULL;
  }

  string output = "";

#ifdef WIN32
  char temp[MAX_PATH];
  if(!GetTempPathA(sizeof(temp), temp))
  {
    return NULL;
  }
  output.append(temp);
#endif
  output.append(configs_file);

  FILE* outfile = fopen(output.c_str(), "wb");
  if(outfile)
  {
    CURL *curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, configs_url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_FILE, outfile);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    int res = curl_easy_perform(curl_handle);
    if(res == CURLE_OK)
    {
      ret = 0;
    }

    curl_easy_cleanup(curl_handle);

    fclose(outfile);
  }
  
  if(ret != -1)
  {
    ifstream infile;  
    infile.open(output.c_str());
    
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
  }

  remove(output.c_str());

  return (ret != -1) ? &configlist : NULL;
}

int configupdater::getconfigs(list<string>* cl)
{
  int ret = -1;
  
  if(configs_dir.empty())
  {
    return -1;
  }
  
  for(list<string>::iterator it = cl->begin(); it != cl->end(); ++it)
  {
    string config = configs_url + *it;
    string output = configs_dir + *it;
    
    FILE* outfile = fopen(output.c_str(), "wb");
    if(outfile)
    {
      CURL *curl_handle = curl_easy_init();

      curl_easy_setopt(curl_handle, CURLOPT_URL, config.c_str());
      curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
      curl_easy_setopt(curl_handle, CURLOPT_FILE, outfile);
      curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      int res = curl_easy_perform(curl_handle);
      if(res == CURLE_OK)
      {
        ret = 0;
      }

      curl_easy_cleanup(curl_handle);

      fclose(outfile);
    }

#ifndef WIN32
    if(chown(file.c_str(), getpwuid(getuid())->pw_uid, getpwuid(getuid())->pw_gid) < 0)
    {
      ret = -1;
    }
#endif
  }
  
  return ret;
}
