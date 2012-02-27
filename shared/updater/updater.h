
#define VERSION_URL "http://gimx.fr/download/version"

#ifdef WIN32

#define VERSION_FILE "version"
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-windows.html"
#define DOWNLOAD_FILE "setup.exe"

#else

#define VERSION_FILE "/tmp/version"
#define DOWNLOAD_FILE "/tmp/gimx.deb"

#include <limits.h>

#if ( __WORDSIZE == 64 )
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-64bits.html"
#else
#define DOWNLOAD_URL "http://gimx.fr/download/gimx-ubuntu-32bits.html"
#endif

#endif

#include <string>

using namespace std;

class updater
{
public:
  updater(string vu, string vf, string v, string du, string df) : 
    version_url(vu),
    version_file(vf),
    version(v),
    download_url(du),
    download_file(df)
  {};
  int checkversion();
  int update();
private:
  string version_url;
  string version_file;
  string version;
  string download_url;
  string download_file;
};
