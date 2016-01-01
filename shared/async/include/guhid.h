/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef UHIDASYNC_H_
#define GUHID_H_

#include "ghid.h"

int guhid_create(const s_hid_info * hidDesc);
int guhid_close(int device);
int guhid_write(int device, const void * buf, unsigned int count);
int guhid_is_opened(int device);

#endif /* UHIDASYNC_H_ */
