/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GHID_H_
#define GHID_H_

#include "async.h"

#define GHID_MAX_DEVICES ASYNC_MAX_DEVICES

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned short vendor_id;
  unsigned short product_id;
  unsigned short bcdDevice;
  int interface;
  char * path;
  int next;
} s_hid_dev;

int ghid_open_path(const char * device_path);
int ghid_open_ids(unsigned short vendor, unsigned short product);
s_hid_dev * ghid_enumerate(unsigned short vendor, unsigned short product);
void ghid_free_enumeration(s_hid_dev * devs);
const s_hid_info * ghid_get_hid_info(int device);
int ghid_close(int device);
int ghid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int ghid_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write,
    ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int ghid_poll(int device);
int ghid_write(int device, const void * buf, unsigned int count);
int ghid_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* GHID_H_ */
