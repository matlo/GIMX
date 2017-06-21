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
#define CURL_INIT_FLAGS (CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL)
#else
#define CURL_INIT_FLAGS CURL_GLOBAL_NOTHING
#endif

#ifdef WIN32
const char * configupdater::configs_url = "https://api.github.com/repos/matlo/GIMX-configurations/contents/Windows";
const char * configupdater::configs_download_url = "https://raw.githubusercontent.com/matlo/GIMX-configurations/master/Windows/";
#else
const char * configupdater::configs_url = "https://api.github.com/repos/matlo/GIMX-configurations/contents/Linux";
const char * configupdater::configs_download_url = "https://raw.githubusercontent.com/matlo/GIMX-configurations/master/Linux/";
#endif
const char * configupdater::configs_file = "configs";

configupdater::configupdater() : client_callback(NULL), client_data(NULL)
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

static int progress_callback(void * clientp, double dltotal, double dlnow, double ultotal __attribute__((unused)), double ulnow __attribute__((unused)))
{
    return ((configupdater *) clientp)->onProgress(dlnow, dltotal);
}

int configupdater::getconfiglist(const char * url)
{
  int ret = -1;

  string output = "";

#ifdef WIN32
  char temp[MAX_PATH];
  if(!GetTempPathA(sizeof(temp), temp))
  {
    return -1;
  }
  output.append(temp);
#endif
  output.append(configs_file);

  current.empty();

  FILE* outfile = fopen(output.c_str(), "wb");
  if(outfile)
  {
    CURL * curl_handle = curl_easy_init();

    if(curl_handle)
    {
      struct curl_slist * headers = NULL;
      headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");

      if(headers)
      {
#ifdef WIN32
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "ssl/certs/ca-bundle.crt");
#endif
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_FILE, outfile);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        if (client_callback != NULL && client_data != NULL)
        {
          curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
          curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
          curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        }

        CURLcode res = curl_easy_perform(curl_handle);
        if(res == CURLE_OK)
        {
          ret = 0;
        }
        else
        {
          fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
      }
      curl_easy_cleanup(curl_handle);
    }

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
      size_t pos1 = line.find("\"name\": ");
      if(pos1 != string::npos)
      {
        size_t pos2 = line.find("\"", pos1 + strlen("\"name\": "));
        if(pos2 != string::npos)
        {
          size_t pos3 = line.find(".xml\",", pos2 + 1);
          if(pos3 != string::npos)
          {
            configlist.push_back(line.substr(pos2 + 1, pos3 + 4 - (pos2 + 1)));
          }
        }
      }
    }

    infile.close();
  }

  remove(output.c_str());

  return ret;
}

list<string>* configupdater::getconfiglist(CONFIGUPDATER_PROGRESS_CALLBACK callback, void * data)
{
  configlist.clear();
  
  client_callback = callback;
  client_data = data;

  int ret = getconfiglist(configs_url);
  
  return (ret != -1) ? &configlist : NULL;
}

int configupdater::getconfigs(list<string>* cl, CONFIGUPDATER_PROGRESS_CALLBACK callback, void * data)
{
  int ret = -1;
  
  if(configs_dir.empty())
  {
    return -1;
  }
  
  client_callback = callback;
  client_data = data;

  for(list<string>::iterator it = cl->begin(); it != cl->end(); ++it)
  {
    string config = configs_download_url + *it;
    string output = configs_dir + *it;
    
    current = *it;

    FILE* outfile = fopen(output.c_str(), "wb");
    if(outfile)
    {
      CURL *curl_handle = curl_easy_init();

      if(curl_handle)
      {
#ifdef WIN32
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "ssl/certs/ca-bundle.crt");
#endif
        curl_easy_setopt(curl_handle, CURLOPT_URL, config.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_FILE, outfile);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        if (callback != NULL && data != NULL)
        {
          curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
          curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
          curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        }

        CURLcode res = curl_easy_perform(curl_handle);
        if(res == CURLE_OK)
        {
          ret = 0;
        }
        else
        {
          fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl_handle);
      }

      fclose(outfile);
    }

#ifndef WIN32
    if(chown(output.c_str(), getpwuid(getuid())->pw_uid, getpwuid(getuid())->pw_gid) < 0)
    {
      ret = -1;
    }
#endif
  }
  
  return ret;
}
