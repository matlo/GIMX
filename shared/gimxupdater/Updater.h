/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GIMXUPDATER_H_
#define GIMXUPDATER_H_

#include <string>

class Updater {
public:
    Updater();
    virtual ~Updater();
    int checkVersion(std::string versionUrl, std::string version);
    enum UpdaterStatus {
        UpdaterStatusOk = 0,

        UpdaterStatusConnectionPending = 1,
        UpdaterStatusDownloadInProgress = 2,
        UpdaterStatusInstallPending = 3,

        UpdaterStatusCancelled = -1,
        UpdaterStatusInitFailed = -2,
        UpdaterStatusDownloadFailed = -3,
        UpdaterStatusInstallFailed = -4,
    };
    typedef int (*ProgressCallback)(void *clientp, Updater::UpdaterStatus status, double progress, double total);
    UpdaterStatus update(std::string url, ProgressCallback callback, void *clientp, bool wait);
    int progress(Updater::UpdaterStatus, double dlnow, double dltotal);
    static std::string getProgress(double progress, double total);
private:
    ProgressCallback clientCallback;
    void * clientData;
};

#endif /* GIMXUPDATER_H_ */
