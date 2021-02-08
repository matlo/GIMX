/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#ifdef WIN32
#define GIMX_PACKED __attribute__((gcc_struct, packed))
#else
#define GIMX_PACKED __attribute__((packed))
#endif

#define BUFFER_SIZE 254

typedef struct GIMX_PACKED
{
  unsigned char type;
  unsigned char length;
} s_header;

typedef struct GIMX_PACKED
{
  s_header header;
  unsigned char value[BUFFER_SIZE];
} s_packet;

#define HEADER_SIZE sizeof(s_header)

#define BYTE_NO_PACKET    0x00
#define BYTE_TYPE         0x11
#define BYTE_STATUS       0x22 // no more used
#define BYTE_START        0x33
#define BYTE_CONTROL_DATA 0x44
#define BYTE_RESET        0x55
#define BYTE_IDS          0x66
#define BYTE_VERSION      0x77
#define BYTE_BAUDRATE     0x88
#define BYTE_DEBUG        0x99
#define BYTE_OUT_REPORT   0xee
#define BYTE_IN_REPORT    0xff

#define BYTE_STATUS_NSTARTED 0x00
#define BYTE_STATUS_STARTED  0x01

#define BYTE_LEN_0_BYTE 0x00
#define BYTE_LEN_1_BYTE 0x01

#endif /* PROTOCOL_H_ */
