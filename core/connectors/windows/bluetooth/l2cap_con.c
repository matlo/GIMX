/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/tcp_con.h>
#include <connectors/l2cap_con.h>
#include <connectors/bt_utils.h>
#include <hci.h>

#define ACL_MTU 1024
#define L2CAP_MTU 1024

int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, unsigned short psm, int options)
{
  bdaddr_t ba;
  str2ba(bdaddr_dest, &ba);
  bt_send_cmd(&l2cap_create_channel_mtu, ba.b, psm, L2CAP_MTU);

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

