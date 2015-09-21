/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIALASYNC_H_
#define SERIALASYNC_H_

#include <async.h>

#ifdef __cplusplus
extern "C" {
#endif

int serialasync_open(char * portname, unsigned int baudrate);
int serialasync_close(int device);
int serialasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int serialasync_set_read_size(int device, unsigned int size);
int serialasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int serialasync_write_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int serialasync_write(int device, const void * buf, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif /* SERIALASYNC_H_ */
