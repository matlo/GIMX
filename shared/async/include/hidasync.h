/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HIDASYNC_H_
#define HIDASYNC_H_

#include <async.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_VENDOR_ID_LOGITECH          0x046d

#define USB_DEVICE_ID_LOGITECH_WINGMAN_FFG  0xc293
#define USB_DEVICE_ID_LOGITECH_WHEEL        0xc294
#define USB_DEVICE_ID_LOGITECH_MOMO_WHEEL    0xc295
#define USB_DEVICE_ID_LOGITECH_DFP_WHEEL    0xc298
#define USB_DEVICE_ID_LOGITECH_G25_WHEEL    0xc299
#define USB_DEVICE_ID_LOGITECH_DFGT_WHEEL    0xc29a
#define USB_DEVICE_ID_LOGITECH_G27_WHEEL    0xc29b
#define USB_DEVICE_ID_LOGITECH_WII_WHEEL    0xc29c
#define USB_DEVICE_ID_LOGITECH_MOMO_WHEEL2      0xca03
#define USB_DEVICE_ID_LOGITECH_VIBRATION_WHEEL  0xca04

struct hid_device;

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
    char * path;
    int next;
} s_hid_dev;

int hidasync_open_path(const char * device_path);
int hidasync_open_ids(unsigned short vendor, unsigned short product);
s_hid_dev * hidasync_enumerate(unsigned short vendor, unsigned short product);
void  hidasync_free_enumeration(s_hid_dev * devs);
const s_hid_info * hidasync_get_hid_info(int device);
int hidasync_close(int device);
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout);
int hidasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register);
int hidasync_write(int device, const void * buf, unsigned int count);
int hidasync_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* HIDASYNC_H_ */
