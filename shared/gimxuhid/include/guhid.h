/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GUHID_H_
#define GUHID_H_

#include <gimxhid/include/ghid.h>

int guhid_create(const s_hid_info * hid_info, int hid);
int guhid_close(int device);
int guhid_write(int device, const void * buf, unsigned int count);
int guhid_is_opened(int device);

#endif /* GUHID_H_ */
