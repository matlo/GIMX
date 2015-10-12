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
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

updater* updater::_singleton = NULL;

#ifdef WIN32
#define CURL_INIT_FLAGS (CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL)
#else
#define CURL_INIT_FLAGS CURL_GLOBAL_NOTHING
#endif

updater::updater()
{
  //ctor
  curl_global_init(CURL_INIT_FLAGS);
}

updater::~updater()
{
  //dtor
  curl_global_cleanup();
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

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int updater::CheckVersion()
{
  string v;
  int major, minor;
  int old_major, old_minor;
  int ret = -1;

  if(version_url.empty() || version_file.empty() || version.empty())
  {
    return -1;
  }

  CURLcode res;
  struct MemoryStruct chunk;
  chunk.memory = (char*) malloc(sizeof(char));
  chunk.size = 0;

  CURL *curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, VERSION_URL);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  res = curl_easy_perform(curl_handle);
  if(res == CURLE_OK)
  {
    string v = string(chunk.memory);

    vector<string> elems = split(v, '.');
    if (elems.size() == 2)
    {
      major = atoi(elems[0].c_str());
      minor = atoi(elems[1].c_str());

      vector<string> old_elems = split(version, '.');
      if (old_elems.size() == 2)
      {
        old_major = atoi(old_elems[0].c_str());
        old_minor = atoi(old_elems[1].c_str());

        ret = (major > old_major || (major == old_major && minor > old_minor));
      }
    }
  }

 /* cleanup curl stuff */
 curl_easy_cleanup(curl_handle);

 if(chunk.memory)
   free(chunk.memory);

 return ret;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

int updater::Update()
{
  int ret = -1;

  if(download_url.empty())
  {
    return -1;
  }

  string output = "";

#ifdef WIN32
  char temp[MAX_PATH];
  if(!GetTempPathA(sizeof(temp), temp))
  {
    return -1;
  }
  output.append(temp);
#endif
  output.append(download_file);

  FILE* outfile = fopen(output.c_str(), "wb");
  if(outfile)
  {
    CURL *curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, download_url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef WIN32
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "ssl/certs/ca-bundle.crt");
#endif
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_FILE, outfile);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

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

    fclose(outfile);

#ifdef WIN32
    STARTUPINFOA startupInfo = STARTUPINFO();
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInformation;

    char* ccmd = strdup(output.c_str());
  
    BOOL result = CreateProcessA(
        output.c_str(),
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
    string cmd = "";
    cmd.append("xdg-open ");
    cmd.append(download_file);
    cmd.append("&");

    if(system(cmd.c_str()))
    {
      return -1;
    }
#endif
  }

  return ret;
}
