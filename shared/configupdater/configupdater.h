/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string>
#include <list>

using namespace std;

class configupdater
{
public:
  void setconfigdirectory(string cd)
  {
    configs_dir = cd;
  }
  list<string>* getconfiglist();
  int getconfigs(list<string>* cl);
  static configupdater* getInstance ()
  {
    if (NULL == _singleton)
    {
      _singleton =  new configupdater;
    }

    return _singleton;
  }
private:
  configupdater();
  virtual ~configupdater();
  int getconfiglist(const char * url);
  string configs_dir;
  list<string> configlist;

  static const char * configs_url;
  static const char * configs_download_url;
  static const char * configs_file;

  static configupdater* _singleton;
};
