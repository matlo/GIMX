/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define CONFIGS_URL "http://diyps3controller.googlecode.com/svn/trunk/configurations/"

#ifdef WIN32
#define CONFIGS_FILE "configs"
#else
#define CONFIGS_FILE "/tmp/configs"
#endif

#include <string>
#include <list>

using namespace std;

class configupdater
{
public:
  configupdater(string cu, string cf, string cd) : configs_url(cu), configs_file(cf), configs_dir(cd) {};
  list<string>* getconfiglist();
  int getconfigs(list<string>* cl);
private:
  string configs_url;
  string configs_file;
  string configs_dir;
  list<string> configlist;
};
