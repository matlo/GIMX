/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define VERSION_URL "http://gimx.fr/download/version"

#ifdef WIN32

#define VERSION_FILE "version"
#ifdef __x86_64__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-windows-64bits"
#else
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-windows-32bits"
#endif
#define DOWNLOAD_FILE "gimx-update.exe"

#else

#define VERSION_FILE "/tmp/version"
#define DOWNLOAD_FILE "/tmp/gimx-update.deb"

#include <limits.h>

#ifndef __ARM_ARCH_6__
#if ( __WORDSIZE == 64 )
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-64bits.html"
#else
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-32bits.html"
#endif
#else
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-raspbian.html"
#endif

#endif

#include <string>

using namespace std;

class updater
{
public:
  int CheckVersion();
  int Update();
  void SetParams(string vu, string vf, string v, string du, string df)
  {
    version_url = vu;
    version_file = vf;
    version = v;
    download_url = du;
    download_file = df;
  };
  static updater* getInstance ()
  {
    if (NULL == _singleton)
    {
      _singleton =  new updater;
    }

    return _singleton;
  }
private:
  updater();
  virtual ~updater();
  string version_url;
  string version_file;
  string version;
  string download_url;
  string download_file;

  static updater* _singleton;
};
