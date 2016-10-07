/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef ASYNC_H_

#define ASYNC_H_

#include "gpoll.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define ASYNC_MAX_DEVICES 256

typedef enum {
    E_ASYNC_DEVICE_TYPE_SERIAL,
    E_ASYNC_DEVICE_TYPE_HID,
} e_async_device_type;

typedef int (* ASYNC_READ_CALLBACK)(int user, const void * buf, int status);
typedef int (* ASYNC_WRITE_CALLBACK)(int user, int status);
typedef int (* ASYNC_CLOSE_CALLBACK)(int user);
#ifndef WIN32
typedef GPOLL_REGISTER_FD ASYNC_REGISTER_SOURCE;
typedef GPOLL_REMOVE_FD ASYNC_REMOVE_SOURCE;
#else
typedef GPOLL_REGISTER_HANDLE ASYNC_REGISTER_SOURCE;
typedef GPOLL_REMOVE_HANDLE ASYNC_REMOVE_SOURCE;
#endif

typedef struct {
    ASYNC_READ_CALLBACK fp_read;       // called on data reception
    ASYNC_WRITE_CALLBACK fp_write;     // called on write completion
    ASYNC_CLOSE_CALLBACK fp_close;     // called on failure
    ASYNC_REGISTER_SOURCE fp_register; // to register device to event sources
    ASYNC_REMOVE_SOURCE fp_remove;     // to remove device from event sources
} ASYNC_CALLBACKS;

int async_open_path(const char * path, int print);
int async_close(int device);
int async_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int async_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);
int async_set_read_size(int device, unsigned int size);
int async_register(int device, int user, const ASYNC_CALLBACKS * callbacks);
int async_write(int device, const void * buf, unsigned int count);
int async_set_overlapped(int device);

#ifdef WIN32
HANDLE * async_get_handle(int device);
const char * async_get_path(int device);
void async_set_device_type(int device, e_async_device_type device_type);
int async_set_write_size(int device, unsigned int size);
void async_set_private(int device, void * priv);
void * async_get_private(int device);
#else
int async_get_fd(int device);
#endif

#endif /* ASYNC_H_ */
