/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HIDASYNC_H_
#define HIDASYNC_H_

#include <async.h>

#define HIDASYNC_MAX_TRANSFER_SIZE 64

typedef struct {
    unsigned short vendorId;
    unsigned short productId;
    unsigned short version;
    unsigned char countryCode;
    unsigned char * reportDescriptor;
    unsigned short reportDescriptorLength;
    char * manufacturerString;
    char * productString;
} s_hid_info;

int hidasync_open_path(const char * device_path);
int hidasync_open_ids(unsigned short vendor, unsigned short product);
const s_hid_info * hidasync_get_hid_info(int device);
int hidasync_close(int device);
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int hidasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int hidasync_write(int device, const void * buf, unsigned int count);
int hidasync_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);

#endif /* HIDASYNC_H_ */
