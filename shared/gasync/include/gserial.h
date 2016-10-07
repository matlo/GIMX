/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GSERIAL_H_

#define GSERIAL_H_

#include "gpoll.h"

typedef int (* GSERIAL_READ_CALLBACK)(int user, const void * buf, int status);
typedef int (* GSERIAL_WRITE_CALLBACK)(int user, int status);
typedef int (* GSERIAL_CLOSE_CALLBACK)(int user);
#ifndef WIN32
typedef GPOLL_REGISTER_FD GSERIAL_REGISTER_SOURCE;
typedef GPOLL_REMOVE_FD GSERIAL_REMOVE_SOURCE;
#else
typedef GPOLL_REGISTER_HANDLE GSERIAL_REGISTER_SOURCE;
typedef GPOLL_REMOVE_HANDLE GSERIAL_REMOVE_SOURCE;
#endif

typedef struct {
    GSERIAL_READ_CALLBACK fp_read;       // called on data reception
    GSERIAL_WRITE_CALLBACK fp_write;     // called on write completion
    GSERIAL_CLOSE_CALLBACK fp_close;     // called on failure
    GSERIAL_REGISTER_SOURCE fp_register; // to register the device to event sources
    GSERIAL_REMOVE_SOURCE fp_remove;     // to remove the device from event sources
} GSERIAL_CALLBACKS;

#ifdef __cplusplus
extern "C" {
#endif

int gserial_open(const char * portname, unsigned int baudrate);
int gserial_close(int device);
int gserial_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int gserial_set_read_size(int device, unsigned int size);
int gserial_register(int device, int user, const GSERIAL_CALLBACKS * callbacks);
int gserial_write_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int gserial_write(int device, const void * buf, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif /* GSERIAL_H_ */
