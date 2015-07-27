/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HIDASYNC_H_
#define HIDASYNC_H_

#ifdef WIN32
#include <windows.h>
#endif

#define HIDASYNC_MAX_DEVICES 256
#define HIDASYNC_MAX_TRANSFER_SIZE 64

typedef int (* HIDASYNC_READ_CALLBACK)(int user, const char * buf, unsigned int count);
#ifndef WIN32
typedef void (* HIDASYNC_REGISTER_SOURCE)(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
#else
typedef void (* HIDASYNC_REGISTER_SOURCE)(HANDLE handle, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
#endif

void hidsaync_init();
void hidasync_quit();
int hidasync_open_path(const char * device_path);
int hidasync_open_ids(unsigned short vendor, unsigned short product);
int hidasync_get_ids(int device, unsigned short * vendor, unsigned short * product);
int hidasync_close(int device);
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int hidasync_register(int device, int user, HIDASYNC_READ_CALLBACK fp_read, HIDASYNC_REGISTER_SOURCE fp_register);
int hidasync_write(int device, const void * buf, unsigned int count);

#endif /* HIDASYNC_H_ */
