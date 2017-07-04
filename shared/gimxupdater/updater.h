/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define VERSION_URL "http://gimx.fr/download/version"

#ifdef WIN32

#define VERSION_FILE "version"

#ifdef __x86_64__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-windows-64bits"
#endif

#ifdef __i386__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-windows-32bits"
#endif

#define DOWNLOAD_FILE "gimx-update.exe"

#else

#define VERSION_FILE "/tmp/version"
#define DOWNLOAD_FILE "/tmp/gimx-update.deb"

#include <limits.h>

#ifdef __x86_64__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-64bits.html"
#endif

#ifdef __i386__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-32bits.html"
#endif

#ifdef __ARM_ARCH_6__
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-raspbian.html"
#endif

#endif

#include <string>

using namespace std;

typedef int (* UPDATER_PROGRESS_CALLBACK)(void *clientp, string & file, unsigned int dlnow, unsigned int dltotal);

class updater
{
public:
  int CheckVersion();
  int Update(UPDATER_PROGRESS_CALLBACK callback, void * data);
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
  int onProgress(unsigned int dlnow, unsigned int dltotal)
  {
      string file;
      return client_callback(client_data, file, dlnow, dltotal);
  }
private:
  updater();
  virtual ~updater();
  string version_url;
  string version_file;
  string version;
  string download_url;
  string download_file;

  UPDATER_PROGRESS_CALLBACK client_callback;
  void * client_data;

  static updater* _singleton;
};
