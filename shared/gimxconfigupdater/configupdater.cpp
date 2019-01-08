/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "configupdater.h"
#include <sstream>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pwd.h> //to get the user & group id
#include <unistd.h>
#endif

#include <ext/stdio_filebuf.h>
#include <fcntl.h>

#ifdef WIN32

static wchar_t * utf8_to_utf16le(const char * inbuf)
{
  wchar_t * outbuf = NULL;
  int outsize = MultiByteToWideChar(CP_UTF8, 0, inbuf, -1, NULL, 0);
  if (outsize != 0) {
      outbuf = (wchar_t*) malloc(outsize * sizeof(*outbuf));
      if (outbuf != NULL) {
         int res = MultiByteToWideChar(CP_UTF8, 0, inbuf, -1, outbuf, outsize);
         if (res == 0) {
             free(outbuf);
             outbuf = NULL;
         }
      }
  }
  return outbuf;
}

#define ORDONLY _O_RDONLY
#define OWRONLY _O_WRONLY
#define OTRUNC  _O_TRUNC

#define OFBUF(path, flags) \
    wchar_t * wpath = utf8_to_utf16le(path.c_str()); \
    __gnu_cxx::stdio_filebuf<char> fb(_wopen(wpath, _O_BINARY | flags), std::ios::out | std::ios::binary); \
    free(wpath);

#define IFBUF(path, flags) \
    wchar_t * wpath = utf8_to_utf16le(path.c_str()); \
    __gnu_cxx::stdio_filebuf<char> fb(_wopen(wpath, _O_BINARY | flags), std::ios::in | std::ios::binary); \
    free(wpath);

#else

#define ORDONLY O_RDONLY
#define OWRONLY O_WRONLY
#define OTRUNC  O_TRUNC

#define OFBUF(path, flags) \
    __gnu_cxx::stdio_filebuf<char> fb(open(path.c_str(), flags), std::ios::out);

#define IFBUF(path, flags) \
    __gnu_cxx::stdio_filebuf<char> fb(open(path.c_str(), flags), std::ios::in);

#endif

#define IFSTREAM(path, name, flags) \
    IFBUF(path, flags) \
    std::istream name (&fb);

#define OFSTREAM(path, name, flags) \
    OFBUF(path, flags) \
    std::ostream name (&fb);

#ifdef WIN32
const char * configupdater::configs_url = "https://api.github.com/repos/matlo/GIMX-configurations/contents/Windows";
const char * configupdater::configs_download_url = "https://raw.githubusercontent.com/matlo/GIMX-configurations/master/Windows/";
#else
const char * configupdater::configs_url = "https://api.github.com/repos/matlo/GIMX-configurations/contents/Linux";
const char * configupdater::configs_download_url =
        "https://raw.githubusercontent.com/matlo/GIMX-configurations/master/Linux/";
#endif

configupdater::configupdater() :
        client_callback(NULL), client_data(NULL) {
}

configupdater::~configupdater() {
}

int configupdater::progress(configupdater::ConfigUpdaterStatus status, double dlnow, double dltotal) {

    return client_callback(client_data, status, dlnow, dltotal);
}

configupdater::ConfigUpdaterStatus convertDowloadStatus(Downloader::DownloaderStatus status) {
    switch (status) {
    case Downloader::DownloaderStatusOk:
        return configupdater::ConfigUpdaterStatusOk;
    case Downloader::DownloaderStatusConnectionPending:
        return configupdater::ConfigUpdaterStatusConnectionPending;
    case Downloader::DownloaderStatusDownloadInProgress:
        return configupdater::ConfigUpdaterStatusDownloadInProgress;
    case Downloader::DownloaderStatusCancelled:
        return configupdater::ConfigUpdaterStatusCancelled;
    case Downloader::DownloaderStatusInitFailed:
        return configupdater::ConfigUpdaterStatusInitFailed;
    case Downloader::DownloaderStatusDownloadFailed:
        return configupdater::ConfigUpdaterStatusDownloadFailed;
    }
    return configupdater::ConfigUpdaterStatusOk;
}

int progressCallback(void *clientp, Downloader::DownloaderStatus status, double progress, double total) {

    configupdater * updater = static_cast<configupdater *>(clientp);

    return updater->progress(convertDowloadStatus(status), progress, total);
}

configupdater::ConfigUpdaterStatus configupdater::getconfiglist(std::list<std::string>& cl, ProgressCallback callback, void * data) {

    client_callback = callback;
    client_data = data;

    std::string tempFile = Downloader::generateTempFile("configs.json");

    Downloader::DownloaderStatus downloadStatus = Downloader().download(configs_url, tempFile, progressCallback, this);

    if (downloadStatus != Downloader::DownloaderStatusOk) {
        remove(tempFile.c_str());
        return convertDowloadStatus(downloadStatus);
    }

    IFSTREAM(tempFile, infile, ORDONLY)

    while (infile.good()) {
        std::string line;
        std::getline(infile, line);
        size_t pos1 = line.find("\"name\": ");
        if (pos1 != std::string::npos) {
            size_t pos2 = line.find("\"", pos1 + strlen("\"name\": "));
            if (pos2 != std::string::npos) {
                size_t pos3 = line.find(".xml\",", pos2 + 1);
                if (pos3 != std::string::npos) {
                    cl.push_back(line.substr(pos2 + 1, pos3 + 4 - (pos2 + 1)));
                }
            }
        }
    }

    remove(tempFile.c_str());

    return configupdater::ConfigUpdaterStatusOk;
}

configupdater::ConfigUpdaterStatus configupdater::getconfig(const std::string& directory, const std::string& config, ProgressCallback callback, void * data) {

    if (directory.empty()) {
        return configupdater::ConfigUpdaterStatusInitFailed;
    }

    client_callback = callback;
    client_data = data;

    std::string url = configs_download_url + config;
    std::string file = directory + config;

    Downloader::DownloaderStatus downloadStatus = Downloader().download(url, file, progressCallback, this);

    if (downloadStatus != Downloader::DownloaderStatusOk) {
        return convertDowloadStatus(downloadStatus);
    }

#ifndef WIN32
    if (chown(file.c_str(), getpwuid(getuid())->pw_uid, getpwuid(getuid())->pw_gid) < 0) {
        return configupdater::ConfigUpdaterStatusDownloadFailed;
    }
#endif

    return configupdater::ConfigUpdaterStatusOk;
}
