/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef UHIDASYNC_H_
#define UHIDASYNC_H_

#include <hidasync.h>

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

int uhidasync_create(const s_hid_info * hidDesc);
int uhidasync_close(int device);
int uhidasync_write(int device, const void * buf, unsigned int count);
int uhidasync_is_opened(int device);

#endif /* UHIDASYNC_H_ */
