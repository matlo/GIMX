/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GHID_H_

#define GHID_H_

#include "gpoll.h"

typedef int (* GHID_READ_CALLBACK)(int user, const void * buf, int status);
typedef int (* GHID_WRITE_CALLBACK)(int user, int status);
typedef int (* GHID_CLOSE_CALLBACK)(int user);
#ifndef WIN32
typedef GPOLL_REGISTER_FD GHID_REGISTER_SOURCE;
#else
typedef GPOLL_REGISTER_HANDLE GHID_REGISTER_SOURCE;
#endif

#define GHID_MAX_DEVICES 256

#ifdef __cplusplus
extern "C" {
#endif

struct ghid_device {
  unsigned short vendor_id;
  unsigned short product_id;
  unsigned short bcdDevice;
  int interface_number;
  char * path;
  struct ghid_device * next;
};

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short bcdDevice;
#ifndef WIN32
    unsigned short version;
    unsigned char countryCode;
    unsigned char * reportDescriptor;
    unsigned short reportDescriptorLength;
    char * manufacturerString;
    char * productString;
#endif
} s_hid_info;

int ghid_open_path(const char * device_path);
int ghid_open_ids(unsigned short vendor, unsigned short product);
struct ghid_device * ghid_enumerate(unsigned short vendor, unsigned short product);
void ghid_free_enumeration(struct ghid_device * devs);
const s_hid_info * ghid_get_hid_info(int device);
int ghid_close(int device);
int ghid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int ghid_register(int device, int user, GHID_READ_CALLBACK fp_read, GHID_WRITE_CALLBACK fp_write,
    GHID_CLOSE_CALLBACK fp_close, GHID_REGISTER_SOURCE fp_register);
int ghid_poll(int device);
int ghid_write(int device, const void * buf, unsigned int count);
int ghid_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* GHID_H_ */
