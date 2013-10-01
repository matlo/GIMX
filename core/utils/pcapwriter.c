/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "pcapwriter.h"

#define URB_SUBMIT 0x53
#define URB_COMPLETE 0x43
#define URB_CONTROL 0x02

#ifdef WIN32
#define EINPROGRESS -115
#endif

#define SETUP_LEN 8

typedef struct pcap_hdr_s {
  guint32 magic_number;   /* magic number */
  guint16 version_major;  /* major version number */
  guint16 version_minor;  /* minor version number */
  gint32  thiszone;       /* GMT to local correction */
  guint32 sigfigs;        /* accuracy of timestamps */
  guint32 snaplen;        /* max length of captured packets, in octets */
  guint32 network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
  guint32 ts_sec;         /* timestamp seconds */
  guint32 ts_usec;        /* timestamp microseconds */
  guint32 incl_len;       /* number of octets of packet saved in file */
  guint32 orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

static pcap_hdr_t capture_header =
{
  .magic_number = 0xa1b2c3d4,
  .version_major = 0x0002,
  .version_minor = 0x0004,
  .thiszone = 0x00000000,
  .sigfigs = 0x00000000,
  .snaplen = 0x0000FFFF,
  .network = 0x000000BD
};

static pcaprec_hdr_t packet_header = {};

static FILE* file = NULL;

void pcapwriter_init(char* file_name)
{
  file = fopen(file_name, "w");

  if(!file)
  {
    perror("pcapwriter_init");
  }
  else
  {
    fwrite((char*)&capture_header, 1, sizeof(capture_header), file);
  }
}

void pcapwriter_close()
{
  if(file)
  {
    fclose(file);
  }
}

void pcapwriter_write(pcap_usb_header* usb_header, unsigned int data_length, unsigned char data[data_length])
{
  struct timeval tv;
  unsigned int length = 0;

  if(!file)
  {
    perror("pcapwriter_write");
    return;
  }

  gettimeofday(&tv, NULL);

  packet_header.ts_sec = tv.tv_sec;
  packet_header.ts_usec = tv.tv_usec;

  packet_header.incl_len = sizeof(pcap_usb_header);
  packet_header.orig_len = sizeof(pcap_usb_header);

  usb_header->ts_sec = tv.tv_sec;
  usb_header->ts_usec = tv.tv_usec;

  if(data_length && data)
  {
    if(data_length <= USB_MAX_DATA_LENGTH)
    {
      length = data_length;
      packet_header.incl_len += data_length;
      packet_header.orig_len += data_length;
    }
    else
    {
      length = USB_MAX_DATA_LENGTH;
      packet_header.incl_len += USB_MAX_DATA_LENGTH;
      packet_header.orig_len += data_length;
    }
  }
  else
  {
    length = 0;
  }

  fwrite((char*)&packet_header, 1, sizeof(packet_header), file);
  fwrite((char*)usb_header, 1, sizeof(pcap_usb_header), file);
  fwrite((char*)data, 1, length, file);
}
