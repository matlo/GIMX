/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef USBHIDASYNC_H_
#define USBHIDASYNC_H_

#include "ghid.h"

int usbhidasync_open_ids(unsigned short vendor, unsigned short product);
s_hid_dev * usbhidasync_enumerate(unsigned short vendor, unsigned short product);
void usbhidasync_free_enumeration(s_hid_dev * hid_devs);
int usbhidasync_open_path(const char * path);
const s_hid_info * usbhidasync_get_hid_info(int device);
int usbhidasync_close(int device);
int usbhidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int usbhidasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int usbhidasync_write(int device, const void * buf, unsigned int count);
int usbhidasync_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);

#endif /* USBHIDASYNC_H_ */
