/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GUSB_H_
#define GUSB_H_

#include <libusb-1.0/libusb.h>
#include <linux/usb/ch9.h>
#include "gpoll.h"

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

struct usb_hid_descriptor {
  unsigned char bLength;
  unsigned char bDescriptorType;
  unsigned short bcdHID;
  unsigned char bCountryCode;
  unsigned char bNumDescriptors;
  struct {
    unsigned char bReportDescriptorType;
    unsigned short wReportDescriptorLength;
  } rdesc[0];
} PACKED;

typedef enum {
  E_TRANSFER_TIMED_OUT = -1,
  E_TRANSFER_STALL = -2,
  E_TRANSFER_ERROR = -3,
} e_transfer_status;

typedef int (* USBASYNC_READ_CALLBACK)(int user, unsigned char endpoint, const void * buf, int status);
typedef int (* USBASYNC_WRITE_CALLBACK)(int user, unsigned char endpoint, int status);
typedef int (* USBASYNC_CLOSE_CALLBACK)(int user);

struct p_altInterface {
  struct usb_interface_descriptor * descriptor;
  struct usb_hid_descriptor * hidDescriptor;
  unsigned char bNumEndpoints;
  struct usb_endpoint_descriptor ** endpoints; //bNumEndpoints elements
};

struct p_interface {
  unsigned char bNumAltInterfaces;
  struct p_altInterface * altInterfaces; //bNumAltInterfaces elements
};

struct p_configuration {
  unsigned char * raw; //descriptor->wTotalLength bytes
  struct usb_config_descriptor * descriptor;
  struct p_interface * interfaces; //descriptor->bNumInterfaces elements
};

struct p_other {
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
  unsigned char * data;
};

typedef struct {
    struct usb_device_descriptor device;
    struct p_configuration * configurations; //device.bNumConfigurations elements
    struct usb_string_descriptor langId0;
    unsigned int nbOthers;
    struct p_other * others; //nbOthers elements
} s_usb_descriptors;

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
    char * path;
    int next;
} s_usb_dev;

int gusb_open_ids(unsigned short vendor, unsigned short product);
s_usb_dev * gusb_enumerate(unsigned short vendor, unsigned short product);
void gusb_free_enumeration(s_usb_dev * usb_devs);
int gusb_open_path(const char * path);
s_usb_descriptors * gusb_get_usb_descriptors(int device);
int gusb_close(int device);
int gusb_read_timeout(int device, unsigned char endpoint, void * buf, unsigned int count, unsigned int timeout);
int gusb_register(int device, int user, USBASYNC_READ_CALLBACK fp_read, USBASYNC_WRITE_CALLBACK fp_write,
    USBASYNC_CLOSE_CALLBACK fp_close, GPOLL_REGISTER_FD fp_register);
int gusb_write(int device, unsigned char endpoint, const void * buf, unsigned int count);
int gusb_write_timeout(int device, unsigned char endpoint, const void * buf, unsigned int count,
    unsigned int timeout);
int gusb_poll(int device, unsigned char endpoint);

#endif /* GUSB_H_ */
