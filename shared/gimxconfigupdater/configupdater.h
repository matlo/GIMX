/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string>
#include <list>

using namespace std;

typedef int (* CONFIGUPDATER_PROGRESS_CALLBACK)(void *clientp, string & file, unsigned int dlnow, unsigned int dltotal);

class configupdater
{
public:
  void setconfigdirectory(string cd)
  {
    configs_dir = cd;
  }
  list<string>* getconfiglist(CONFIGUPDATER_PROGRESS_CALLBACK callback, void * data);
  int getconfigs(list<string>* cl, CONFIGUPDATER_PROGRESS_CALLBACK callback, void * data);
  static configupdater* getInstance ()
  {
    if (NULL == _singleton)
    {
      _singleton =  new configupdater;
    }

    return _singleton;
  }
  int onProgress(unsigned int dlnow, unsigned int dltotal)
  {
      return client_callback(client_data, current, dlnow, dltotal);
  }
private:
  configupdater();
  virtual ~configupdater();
  int getconfiglist(const char * url);
  string configs_dir;
  list<string> configlist;

  string current;

  static const char * configs_url;
  static const char * configs_download_url;
  static const char * configs_file;

  CONFIGUPDATER_PROGRESS_CALLBACK client_callback;
  void * client_data;

  static configupdater* _singleton;
};
