/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef USB_SPOOF_H_
#define USB_SPOOF_H_

#ifndef WIN32
#include <libusb-1.0/libusb.h>
#else
#include <libusbx-1.0/libusb.h>
#endif
#include <serial.h>

#define X360_VENDOR 0x045e
#define X360_PRODUCT 0x028e

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

#define REQTYPE_VENDOR (2 << 5)

#define BUFFER_SIZE 0xFF

#define BYTE_TYPE        0x11
#define BYTE_STATUS      0x22
#define BYTE_START_SPOOF 0x33
#define BYTE_SPOOF_DATA  0x44
#define BYTE_SEND_REPORT 0xff

#define BYTE_TYPE_X360       0x01
#define BYTE_STATUS_NSPOOFED 0x00
#define BYTE_STATUS_SPOOFED  0x01

#define BYTE_LEN_0_BYTE 0x00
#define BYTE_LEN_1_BYTE 0x01

typedef struct
{
  unsigned char bRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
} control_request_header;

typedef struct
{
  control_request_header header;
  unsigned char data[BUFFER_SIZE-sizeof(control_request_header)];
} control_request;

int usb_spoof_init_usb_device(int vendor, int product,
    uint16_t* bus_id, uint8_t* device_address, int libusb_debug);
void usb_spoof_release_usb_device();
int usb_spoof_get_adapter_type(int fd);
int usb_spoof_get_adapter_status(int fd);
int usb_spoof_forward_to_device(control_request* creq);
int usb_spoof_forward_to_adapter(int fd, unsigned char* data, unsigned char length);

int usb_spoof_spoof_360_controller();

#endif /* USB_SPOOF_H_ */
