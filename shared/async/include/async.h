/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef ASYNC_H_
#define ASYNC_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define ASYNC_MAX_DEVICES 256
#define ASYNC_MAX_WRITE_QUEUE_SIZE 2

typedef int (* ASYNC_READ_CALLBACK)(int user, const void * buf, unsigned int count);
typedef int (* ASYNC_WRITE_CALLBACK)(int user, int transfered);
typedef int (* ASYNC_CLOSE_CALLBACK)(int user);
#ifndef WIN32
typedef void (* ASYNC_REGISTER_SOURCE)(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
#else
typedef void (* ASYNC_REGISTER_SOURCE)(HANDLE handle, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
#endif

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

typedef struct
{
  struct
  {
    char * buf;
    unsigned int count;
  } data[ASYNC_MAX_WRITE_QUEUE_SIZE];
  unsigned int nb;
} s_queue;

typedef struct {
#ifdef WIN32
    HANDLE handle;
#else
    int fd;
#endif
    char * path;
    struct
    {
#ifdef WIN32
      OVERLAPPED overlapped;
#endif
      char * buf;
      unsigned int count;
      unsigned int bread;
      unsigned int size;
    } read;
#ifdef WIN32
    struct
    {
      OVERLAPPED overlapped;
      s_queue queue;
      unsigned int size;
    } write;
#endif
    struct {
        int user;
        ASYNC_READ_CALLBACK fp_read;
        ASYNC_WRITE_CALLBACK fp_write;
        ASYNC_CLOSE_CALLBACK fp_close;
    } callback;
#ifdef WIN32
    s_hid_info hidInfo;
#endif
    struct
    {
#ifdef WIN32
      unsigned char restoreParams;
      DCB prevParams;
      unsigned char restoreTimeouts;
      COMMTIMEOUTS prevTimeouts;
#endif
    } serial;
} s_device;

extern s_device devices[ASYNC_MAX_DEVICES];

void async_print_error(const char * file, int line, const char * msg);
#define ASYNC_PRINT_ERROR(msg) async_print_error(__FILE__, __LINE__, msg);

inline int async_check_device(int device, const char * file, unsigned int line, const char * func);
#define ASYNC_CHECK_DEVICE(device,retValue) \
  if(async_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

int async_open_path(const char * path, int print);
int async_close(int device);
int async_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int async_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);
int async_set_read_size(int device, unsigned int size);
int async_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int async_write(int device, const void * buf, unsigned int count);
int async_set_overlapped(int device);

#endif /* ASYNC_H_ */
