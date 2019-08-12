/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "Updater.h"
#include <gimxdownloader/Downloader.h>
#include <gimxfile/include/gfile.h>

#include <vector>
#include <sstream>
#include <cstdlib>

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

std::string Updater::getProgress(double progress, double total) {
    return Downloader::getProgress(progress, total);
}

Updater::Updater() :
        clientCallback(NULL), clientData(NULL) {
}

Updater::~Updater() {
}

static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

int Updater::checkVersion(std::string versionUrl, std::string version) {

    if (versionUrl.empty() || version.empty()) {
        return -1;
    }

    std::string v;
    Downloader::DownloaderStatus downloadStatus = Downloader().download(versionUrl, v);

    if (downloadStatus != Downloader::DownloaderStatusOk) {
        return -1;
    }

    std::vector<std::string> elems = split(v, '.');
    if (elems.size() == 2) {
        int major = atoi(elems[0].c_str());
        int minor = atoi(elems[1].c_str());

        std::vector<std::string> old_elems = split(version, '.');
        if (old_elems.size() == 2) {
            int old_major = atoi(old_elems[0].c_str());
            int old_minor = atoi(old_elems[1].c_str());

            return major > old_major || (major == old_major && minor > old_minor);
        }
    }

    return -1;
}

int Updater::progress(Updater::UpdaterStatus status, double dlnow, double dltotal) {

    return clientCallback(clientData, status, dlnow, dltotal);
}

Updater::UpdaterStatus convertDowloadStatus(Downloader::DownloaderStatus status) {
    switch (status) {
    case Downloader::DownloaderStatusOk:
        return Updater::UpdaterStatusOk;
    case Downloader::DownloaderStatusConnectionPending:
        return Updater::UpdaterStatusConnectionPending;
    case Downloader::DownloaderStatusDownloadInProgress:
        return Updater::UpdaterStatusDownloadInProgress;
    case Downloader::DownloaderStatusCancelled:
        return Updater::UpdaterStatusCancelled;
    case Downloader::DownloaderStatusInitFailed:
        return Updater::UpdaterStatusInitFailed;
    case Downloader::DownloaderStatusDownloadFailed:
        return Updater::UpdaterStatusDownloadFailed;
    }
    return Updater::UpdaterStatusOk;
}

int progressCallback(void *clientp, Downloader::DownloaderStatus status, double progress, double total) {

    Updater * updater = static_cast<Updater *>(clientp);

    return updater->progress(convertDowloadStatus(status), progress, total);
}

Updater::UpdaterStatus Updater::update(std::string url, ProgressCallback callback, void *clientp, bool wait) {

    clientCallback = callback;
    clientData = clientp;

    std::string file;
#ifdef WIN32
    file = "update.exe";
#else
    file = "update.deb";
#endif

    std::string tempFile = Downloader::generateTempFile(file);

    Downloader::DownloaderStatus downloadStatus = Downloader().download(url, tempFile, progressCallback, this);

    if (downloadStatus != Downloader::DownloaderStatusOk) {
        return convertDowloadStatus(downloadStatus);
    }

    UpdaterStatus status = UpdaterStatusOk;

#ifdef WIN32
    wchar_t * utf16 = gfile_utf8_to_utf16le(tempFile.c_str());

    SHELLEXECUTEINFOW shExInfo = SHELLEXECUTEINFOW();
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = wait ? (SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC) : SEE_MASK_DEFAULT;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = L"runas";
    shExInfo.lpFile = utf16;
    shExInfo.lpParameters = L"";
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = 0;

    if (!ShellExecuteExW(&shExInfo)) {
        free(utf16);
        if (GetLastError() == ERROR_CANCELLED) {
            return UpdaterStatusCancelled;
        }
        return UpdaterStatusInstallFailed;
    }

    while (WaitForSingleObject(shExInfo.hProcess, 1000) == WAIT_TIMEOUT) {
        if (clientCallback(clientData, UpdaterStatusInstallPending, 0, 0) != 0) {
            status = UpdaterStatusCancelled;
            break;
        }
    }

    CloseHandle(shExInfo.hProcess);
    free(utf16);
#else
    (void)wait;
    std::string cmd = "";
    cmd.append("xdg-open ");
    cmd.append(tempFile);
    cmd.append("&");

    if(system(cmd.c_str()))
    {
        return UpdaterStatusInstallFailed;
    }
#endif

    return status;
}
