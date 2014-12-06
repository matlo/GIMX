/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/tcp_con.h>

/* BD Address */
typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

#define ACL_MTU 1024
#define L2CAP_MTU 1024

int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, unsigned short psm, int options)
{
    //TODO

    return -1;
}

int l2cap_send(int fd, const unsigned char* buf, int len, int blocking)
{
  //TODO

  return len;
}

int l2cap_recv(int fd, unsigned char* buf, int len)
{
  //TODO

  return 0;
}

int l2cap_listen(unsigned short psm, int options)
{
  //TODO

  return 0;
}

int l2cap_accept(int s, bdaddr_t* src, unsigned short* psm, unsigned short* cid)
{
  //TODO

  return -1;
}

int l2cap_is_connected(int fd)
{
  //TODO

  return 0;
}

