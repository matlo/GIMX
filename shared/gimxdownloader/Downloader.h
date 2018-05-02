/*
 Copyright (c) 2018 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GIMXDOWNLOADER_H_
#define GIMXDOWNLOADER_H_

#include <string>
#include <ctime>

typedef void CURL;

class Downloader {
public:
    Downloader();
    virtual ~Downloader();
    enum DownloaderStatus {
        DownloaderStatusOk = 0,
        DownloaderStatusConnectionPending = 1,
        DownloaderStatusDownloadInProgress = 2,
        DownloaderStatusCancelled = -1,
        DownloaderStatusInitFailed = -2,
        DownloaderStatusDownloadFailed = -3,
    };
    typedef int (*ProgressCallback)(void *clientp, Downloader::DownloaderStatus status, double progress, double total);
    DownloaderStatus download(const std::string& url, std::string& output);
    DownloaderStatus download(const std::string& url, const std::string& file, ProgressCallback callback, void * clientp);
    int progress(double dlnow, double dltotal);
    static std::string generateTempFile(const std::string& extension);
    static std::string getProgress(double progress, double total);
private:
    void initCurlHandle(const std::string& url);
    ProgressCallback m_callback;
    void * m_clientp;
    double m_progress;
    CURL * m_curl_handle;
    time_t m_lastUpdate;
};

#endif /* GIMXDOWNLOADER_H_ */
