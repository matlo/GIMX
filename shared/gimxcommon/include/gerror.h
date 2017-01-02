/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GERROR_H_
#define GERROR_H_

#include <stdio.h>
#include <libusb-1.0/libusb.h>

#ifdef WIN32
void gerror_print_last(const char * msg);

#define PRINT_ERROR_GETLASTERROR(msg) \
  fprintf(stderr, "%s:%d %s: %s failed with error", __FILE__, __LINE__, __func__, msg); \
  gerror_print_last("");
#endif

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
static const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

#define PRINT_ERROR_LIBUSB(libusbfunc,ret) fprintf(stderr, "%s:%d %s: %s failed with error: %s\n", __FILE__, __LINE__, __func__, libusbfunc, libusb_strerror(ret));

#define PRINT_TRANSFER_ERROR(transfer) fprintf(stderr, "libusb_transfer failed with status %s (endpoint=0x%02x)\n", libusb_error_name(transfer->status), transfer->endpoint);

#endif /* GERROR_H_ */
