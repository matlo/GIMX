/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef USB_CON_H_
#define USB_CON_H_

#include <GE.h>

int usb_init(int usb_number, unsigned short vendor, unsigned short product);
int usb_close(int usb_number);
int usb_send_control(int usb_number, unsigned char* buffer, unsigned char length);
int usb_send_interrupt_out(int usb_number, unsigned char endpoint, unsigned char* buffer, unsigned char length);
int usb_handle_events(int unused);
void usb_poll_interrupts();
void usb_set_event_callback(int (*fp)(GE_Event*));

#endif /* USB_CON_H_ */
