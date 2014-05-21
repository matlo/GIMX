/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define CONFIGS_URL "http://diyps3controller.googlecode.com/svn/trunk/configurations/"
#define CONFIGS_FILE "configs"

#include <string>
#include <list>

using namespace std;

class configupdater
{
public:
  void SetParams(string cu, string cf, string cd)
  {
    configs_url = cu;
    configs_file = cf;
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
  string configs_url;
  string configs_file;
  string configs_dir;
  list<string> configlist;

  static configupdater* _singleton;
};
