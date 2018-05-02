/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GIMXCONFIGUPDATER_H_
#define GIMXCONFIGUPDATER_H_

#include <string>
#include <list>
#include <gimxdownloader/Downloader.h>

class configupdater
{
public:
  configupdater();
  virtual ~configupdater();
  enum ConfigUpdaterStatus {
      ConfigUpdaterStatusOk = 0,

      ConfigUpdaterStatusConnectionPending = 1,
      ConfigUpdaterStatusDownloadInProgress = 2,

      ConfigUpdaterStatusCancelled = -1,
      ConfigUpdaterStatusInitFailed = -2,
      ConfigUpdaterStatusDownloadFailed = -3,
  };
  typedef int (* ProgressCallback)(void *clientp, configupdater::ConfigUpdaterStatus status, double progress, double total);
  configupdater::ConfigUpdaterStatus getconfiglist(std::list<std::string>& cl, ProgressCallback callback, void * data);
  configupdater::ConfigUpdaterStatus getconfig(const std::string& cd, const std::string& c, ProgressCallback callback, void * data);
  int progress(configupdater::ConfigUpdaterStatus status, double dlnow, double dltotal);
private:

  static const char * configs_url;
  static const char * configs_download_url;

  ProgressCallback client_callback;
  void * client_data;
};

#endif /* GIMXCONFIGUPDATER_H_ */
