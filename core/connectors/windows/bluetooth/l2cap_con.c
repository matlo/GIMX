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

int l2cap_connect(int fd, const char *bdaddr_dest, unsigned short psm)
{
  bdaddr_t dst;
  str2ba(bdaddr_dest, &dst);
  bdaddr_t dst_swapped;
  baswap(&dst_swapped, &dst);
  if( bt_send_cmd(fd, &l2cap_create_channel_mtu, dst_swapped.b, psm, L2CAP_MTU) < 0)
  {
    return -1;
  }
  return 0;
}

int l2cap_send(int fd, unsigned short channel, const unsigned char* buf, int len)
{
  uint8_t header[sizeof(packet_header_t)];
  bt_store_16(header, 0, L2CAP_DATA_PACKET);
  bt_store_16(header, 2, channel);
  bt_store_16(header, 4, len);

  // Linux, MinGW and Cygwin
  if(tcp_send(fd, (const void*)header, 6) < 0)
  {
    return -1;
  }
  if(tcp_send(fd, (const void*)buf, len) < 0)
  {
    return -1;
  }

  return len;
}
