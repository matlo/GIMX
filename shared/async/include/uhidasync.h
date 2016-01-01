/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef UHIDASYNC_H_
#define UHIDASYNC_H_

#include "ghid.h"

int uhidasync_create(const s_hid_info * hidDesc);
int uhidasync_close(int device);
int uhidasync_write(int device, const void * buf, unsigned int count);
int uhidasync_is_opened(int device);

#endif /* UHIDASYNC_H_ */
