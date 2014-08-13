/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#ifndef WIN32
typedef int SERIALOBJECT;
#define SERIALOBJECT_UNDEF -1
#else
#include <windows.h>
typedef HANDLE SERIALOBJECT;
#define SERIALOBJECT_UNDEF NULL
#endif

#define HEADER_SIZE 0x02
#define BUFFER_SIZE 0xFF

#define BYTE_TYPE        0x11
#define BYTE_STATUS      0x22
#define BYTE_START_SPOOF 0x33
#define BYTE_SPOOF_DATA  0x44
#define BYTE_RESET       0x55
#define BYTE_SEND_REPORT 0xff

#define BYTE_TYPE_X360       0x01
#define BYTE_STATUS_NSPOOFED 0x00
#define BYTE_STATUS_SPOOFED  0x01
#define BYTE_STATUS_NSTARTED 0x00
#define BYTE_STATUS_STARTED  0x01

#define BYTE_LEN_0_BYTE 0x00
#define BYTE_LEN_1_BYTE 0x01

int serial_connect(int id, char* portname);
int serial_send(int id, void* pdata, unsigned int size);
int serial_sendv(int id, void* pdata1, unsigned int size1, void* pdata2, unsigned int size2);
int serial_recv(int id, void* pdata, unsigned int size);
int serial_close(int id);

void serial_add_source(int id);

#endif /* SERIAL_H_ */
