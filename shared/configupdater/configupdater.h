
#define CONFIGS_URL "http://diyps3controller.googlecode.com/svn/trunk/GIMX/core/config/"

#ifdef WIN32
#define CONFIGS_FILE "configs"
#define CONFIGS_DIR "config/example/"
#else
#define CONFIGS_FILE "/tmp/configs"
#define CONFIGS_DIR "~/.emuclient/config/example/"
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
