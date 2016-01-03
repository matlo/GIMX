/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GSERIAL_H_

#define GSERIAL_H_

#include "async.h"
#include "gpoll.h"

#ifdef __cplusplus
extern "C" {
#endif

int gserial_open(const char * portname, unsigned int baudrate);
int gserial_close(int device);
int gserial_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int gserial_set_read_size(int device, unsigned int size);
int gserial_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write,
    ASYNC_CLOSE_CALLBACK fp_close, GPOLL_REGISTER_FD fp_register);
int gserial_write_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int gserial_write(int device, const void * buf, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif /* GSERIAL_H_ */
