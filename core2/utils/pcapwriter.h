/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef PCAPWRITER_H_
#define PCAPWRITER_H_

#define USB_MAX_DATA_LENGTH 0xFF

#define URB_SUBMIT 0x53
#define URB_COMPLETE 0x43

#define URB_CONTROL 0x02

#define URB_STATUS_SUCCESS 0x00

#define u_int64_t unsigned long long
#define int64_t signed long long
#define int32_t signed long
#define u_int8_t unsigned char
#define u_int16_t unsigned short
#define u_int32_t unsigned long

#define guint32 unsigned long
#define guint16 unsigned short
#define gint32 signed long

typedef struct _usb_setup {
  u_int8_t bmRequestType;
  u_int8_t bRequest;
  u_int16_t wValue;
  u_int16_t wIndex;
  u_int16_t wLength;
} pcap_usb_setup;

typedef struct _usb_header {
   u_int64_t id;
   u_int8_t event_type;
   u_int8_t transfer_type;
   u_int8_t endpoint_number;
   u_int8_t device_address;
   u_int16_t bus_id;
   char setup_flag;
   char data_flag;
   int64_t ts_sec;
   int32_t ts_usec;
   int32_t status;
   u_int32_t urb_len;
   u_int32_t data_len;
   pcap_usb_setup setup;
} pcap_usb_header;

void pcapwriter_init(char*);
void pcapwriter_close();
void pcapwriter_write(pcap_usb_header*, unsigned int, unsigned char*);

#endif /* PCAPWRITER_H_ */
