/*
 Copyright (c) 2018 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <sstream>
#include <cstring>
#include <cstdlib>
#include "Downloader.h"
#include <iomanip>

#include <curl/curl.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
#define CURL_INIT_FLAGS (CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL)
#else
#define CURL_INIT_FLAGS CURL_GLOBAL_NOTHING
#endif

static int numPlaces(int n) {
    if (n < 0)
        return numPlaces((n == INT_MIN) ? INT_MAX : -n);
    if (n < 10)
        return 1;
    return 1 + numPlaces(n / 10);
}

std::string Downloader::getProgress(double progress, double total) {
    double ddlnow = total * progress / (100 * 1024 * 1024);
    double ddltotal = total / (1024 * 1024);
    std::ostringstream ios;
    ios << std::fixed << std::setprecision(2) << ddlnow << " / " << ddltotal << " MB";
    return ios.str();
}

Downloader::Downloader() :
        m_callback(NULL), m_clientp(NULL), m_progress(0), m_curl_handle(NULL), m_lastUpdate(0) {
    curl_global_init (CURL_INIT_FLAGS);
}

Downloader::~Downloader() {
    curl_global_cleanup();
}

void Downloader::initCurlHandle(const std::string& url) {

    if (m_curl_handle != NULL) {
        curl_easy_cleanup(m_curl_handle);
        m_curl_handle = NULL;
    }

    m_curl_handle = curl_easy_init();

    if (m_curl_handle != NULL) {
        struct curl_slist * headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
        headers = curl_slist_append(headers, "Accept: */*");
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(m_curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef WIN32
        curl_easy_setopt(m_curl_handle, CURLOPT_CAINFO, "ssl/certs/ca-bundle.crt");
#endif
        curl_easy_setopt(m_curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
}

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;

    mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

Downloader::DownloaderStatus Downloader::download(const std::string& url, std::string& output) {

    if (url.empty()) {
        return Downloader::DownloaderStatusInitFailed;
    }

    struct MemoryStruct chunk;
    chunk.memory = (char*) malloc(sizeof(char));
    chunk.size = 0;

    initCurlHandle(url);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, (void *) &chunk);

    CURLcode res = curl_easy_perform(m_curl_handle);

    Downloader::DownloaderStatus status;

    if (res == CURLE_OK) {
        output = std::string(chunk.memory);
        status = Downloader::DownloaderStatusOk;
    } else {
        status = Downloader::DownloaderStatusDownloadFailed;
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(m_curl_handle);
    m_curl_handle = NULL;

    if (chunk.memory)
        free(chunk.memory);

    return status;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}

static int progress_callback(void * clientp, double dltotal, double dlnow, double ultotal __attribute__((unused)),
        double ulnow __attribute__((unused))) {
    return ((Downloader *) clientp)->progress(dlnow, dltotal);
}

int Downloader::progress(double dlnow, double dltotal) {

    DownloaderStatus status = Downloader::DownloaderStatusConnectionPending;

    if (dltotal == 0) {
        status = Downloader::DownloaderStatusConnectionPending;
    } else if (m_progress < 100) {
        status = Downloader::DownloaderStatusDownloadInProgress;
        m_progress = dlnow * 100 / dltotal;
    }

    bool forceProgress = false;

    if (m_progress >= 100) {
        forceProgress = true;
        status = Downloader::DownloaderStatusConnectionPending;
        long response_code;
        CURLcode res = curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &response_code);;
        if (res == CURLE_OK) {
            if ((response_code / 100) == 3) {
                // this is a redirect
                m_progress = 0;
                dltotal = 0;
                status = Downloader::DownloaderStatusConnectionPending;
            }
            else
            {
                status = Downloader::DownloaderStatusOk;
            }
        }
    }

    time_t now = time(NULL);

    if (!forceProgress && now - m_lastUpdate < 1)
    {
        return 0;
    }

    m_lastUpdate = now;

    return m_callback(m_clientp, status, m_progress, dltotal);
}

std::string Downloader::generateTempFile(const std::string& file) {

    std::string value;

#ifdef WIN32
    char temp[MAX_PATH];
    if (!GetTempPathA(sizeof(temp), temp)) {
        return "";
    }
    value.append(temp);
#endif

    value += file;

    return value;
}

#ifndef WIN32
static FILE *fopen2(const char *path, const char *mode) {
    return fopen(path, mode);
}
#else
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

static FILE *fopen2(const char *path, const char *mode) {
    wchar_t * wpath = utf8_to_utf16le(path);
    wchar_t * wmode = utf8_to_utf16le(mode);
    FILE* file = _wfopen(wpath, wmode);
    free(wmode);
    free(wpath);
    return file;
}
#endif

Downloader::DownloaderStatus Downloader::download(const std::string& url, const std::string& file,
        ProgressCallback callback, void * clientp) {

    if (callback == NULL || clientp == NULL) {
        return Downloader::DownloaderStatusInitFailed;
    }

    FILE* outfile = fopen2(file.c_str(), "wb");
    if (outfile == NULL) {
        return Downloader::DownloaderStatusInitFailed;
    }

    m_callback = callback;
    m_clientp = clientp;
    m_progress = 0;

    initCurlHandle(url);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_FILE, outfile);

    curl_easy_setopt(m_curl_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
    curl_easy_setopt(m_curl_handle, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 0L);

    //curl_easy_setopt(m_curl_handle, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(m_curl_handle);

    curl_easy_cleanup(m_curl_handle);
    m_curl_handle = NULL;

    fclose(outfile);

    Downloader::DownloaderStatus status;

    switch (res) {
    case CURLE_ABORTED_BY_CALLBACK:
        status = Downloader::DownloaderStatusCancelled;
        break;
    case CURLE_OK:
        status = Downloader::DownloaderStatusOk;
        break;
    default:
        status = Downloader::DownloaderStatusDownloadFailed;
        break;
    }

    if (status != Downloader::DownloaderStatusOk) {
        remove(file.c_str());
    }

    return status;
}
