/*
 Copyright (c) 2020 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "sockets.h"
#include <gimxfile/include/gfile.h>
#include <stdio.h>

//this is used to make sure WSAStartup/WSACleanup are only called once,
//and to make sure all the sockets are closed before calling WSACleanup
static unsigned int socket_count = 0;

int wsa_init() {

    WSADATA wsadata;
    if (!socket_count) {
        if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR) {
            fprintf(stderr, "WSAStartup failed.");
            return -1;
        }
    }
    return 0;
}

void wsa_count(int error) {

    if (!error) {
        ++socket_count;
    }

    if (!socket_count) {
        WSACleanup();
    }
}

void wsa_clean() {

    --socket_count;

    if (!socket_count) {
        WSACleanup();
    }
}

void psockerror(const char *msg) {
    DWORD error = GetLastError();
    LPWSTR pBuffer = NULL;

    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, 0, (LPWSTR) & pBuffer, 0, NULL)) {
      fprintf(stderr, "%s: code = %lu\n", msg, error);
    } else {
      char * message = gfile_utf16le_to_utf8(pBuffer);
      fprintf(stderr, "%s: %s\n", msg, message);
      free(message);
      LocalFree(pBuffer);
    }
}
