/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef USB_SPOOF_H_
#define USB_SPOOF_H_

#include <libusb-1.0/libusb.h>
#include <connectors/serial.h>

#define X360_VENDOR 0x045e
#define X360_PRODUCT 0x028e

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

#define REQTYPE_VENDOR (2 << 5)

typedef struct
{
  struct libusb_control_setup setup;
  unsigned char data[BUFFER_SIZE-sizeof(struct libusb_control_setup)];
} control_request;

int usb_spoof_init_usb_device(int vendor, int product,
    uint16_t* bus_id, uint8_t* device_address, int libusb_debug);
void usb_spoof_release_usb_device();
int usb_spoof_forward_to_device(control_request* creq);
int usb_spoof_forward_to_adapter(int id, unsigned char* data, unsigned char length);

int usb_spoof_spoof_360_controller(int id);

#endif /* USB_SPOOF_H_ */
