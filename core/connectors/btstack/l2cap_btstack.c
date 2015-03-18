/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <connectors/tcp_con.h>
#include <connectors/bluetooth/btstack_common.h>
#include <connectors/bluetooth/l2cap_abs.h>
#ifdef WIN32
#include <connectors/windows/bt_bdaddr.h>
#endif
#include <hci.h>
#include <GE.h>

#define ACL_MTU 1024
#define L2CAP_MTU 1024

typedef struct
{
  unsigned int user;
  bdaddr_t ba;
  unsigned short psm;
  unsigned short cid;
  unsigned short omtu;
  uint16_t handle;
  L2CAP_ABS_CONNECT_CALLBACK connect_callback;
  L2CAP_ABS_READ_CALLBACK read_callback;
  L2CAP_ABS_PACKET_CALLBACK packet_callback;
  L2CAP_ABS_CLOSE_CALLBACK close_callback;
} s_channel;

static struct
{
  unsigned int nb;
  s_channel entries[L2CAP_ABS_MAX_PEERS*L2CAP_ABS_MAX_CHANNELS];
} channels = {};

typedef struct
{
  int fd;
  L2CAP_ABS_LISTEN_ACCEPT_CALLBACK read_callback;
  L2CAP_ABS_CLOSE_CALLBACK close_callback;
} s_listen_channel;

static struct
{
  unsigned int nb;
  s_listen_channel entries[L2CAP_ABS_MAX_CHANNELS];
} listen_channels = {};

static int l2cap_btstack_connect_channel(bdaddr_t event_addr, uint16_t psm, uint16_t cid, uint16_t handle, uint8_t status)
{
  int result = -1;

  int channel;
  for(channel = 0; channel < channels.nb; ++channel)
  {
    if(!bacmp(&channels.entries[channel].ba, &event_addr) && channels.entries[channel].psm == psm)
    {
      if(status == 0)
      {
        result = channels.entries[channel].connect_callback(channels.entries[channel].user);
        if(result == 0)
        {
          channels.entries[channel].cid = cid;
          channels.entries[channel].handle = handle;
        }
      }
      else
      {
        channels.entries[channel].close_callback(channels.entries[channel].user);
      }
      break;
    }
  }

  return result;
}

static int l2cap_btstack_process_packet(uint16_t cid, unsigned char * packet, int len)
{
  int result = -1;

  int channel;
  for(channel = 0; channel < channels.nb; ++channel)
  {
    if(channels.entries[channel].cid == cid)
    {
      result = channels.entries[channel].packet_callback(channels.entries[channel].user, channels.entries[channel].psm, packet, len);
      break;
    }
  }

  return result;
}

static int packet_handler(int unused)
{
  static recv_data_t recv_data = {};

  bdaddr_t event_addr;
  uint16_t psm = 0, cid, handle;

  int ret = btstack_common_recv(&recv_data);
  if(ret == 1)
  {
    ret = 0;

    uint16_t packet_type = READ_BT_16(recv_data.buffer, 0);
    cid = READ_BT_16(recv_data.buffer, 2);
    unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
    int len = READ_BT_16( recv_data.buffer, 4);

    switch(packet_type)
    {
    case L2CAP_DATA_PACKET:
      if ((ret = l2cap_btstack_process_packet(cid, packet, len)) == -1)
      {
        fprintf(stderr, "error processing data\n");
      }
      break;
    case HCI_EVENT_PACKET:
      switch (packet[0])
      {
      case L2CAP_EVENT_CHANNEL_OPENED:
        // inform about new l2cap connection
        memcpy(&event_addr, packet+3, sizeof(event_addr));
        char bdaddr[18];
        ba2str(&event_addr, bdaddr);
        psm = READ_BT_16(packet, 11);
        cid = READ_BT_16(packet, 13);
        handle = READ_BT_16(packet, 9);

        if(l2cap_btstack_connect_channel(event_addr, psm, cid, handle, packet[2]) == 0)
        {
          printf("Channel successfully opened: %s, handle 0x%02x, psm 0x%02x, local cid 0x%02x, remote cid 0x%02x\n",
              bdaddr, handle, psm, cid,  READ_BT_16(packet, 15));
        }
        else
        {
          printf("L2CAP connection to device %s failed. status code %u\n", bdaddr, packet[2]);
        }
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }

  }
  else if(ret < 0)
  {
    ret = 1;
  }
  return ret;
}

int l2cap_btstack_connect(const char * bdaddr_src, const char * bdaddr_dest, unsigned short psm, int options,
    int user, L2CAP_ABS_CONNECT_CALLBACK connect_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
  bdaddr_t dst;
  str2ba(bdaddr_dest, &dst);
  bdaddr_t dst_swapped;
  baswap(&dst_swapped, &dst);
  if( btstack_common_send_cmd(&btstack_l2cap_create_channel_mtu, dst_swapped.b, psm, L2CAP_MTU) < 0)
  {
    return -1;
  }

  //TODO MLA: this is required only once, maybe this should be moved?
  GE_AddSource(btstack_common_getfd(), 0, &packet_handler, NULL, &packet_handler);

  int channel = channels.nb;

  bacpy(&channels.entries[channel].ba, &dst);
  channels.entries[channel].psm = psm;
  channels.entries[channel].user = user;
  channels.entries[channel].connect_callback = connect_callback;
  channels.entries[channel].close_callback = close_callback;

  ++channels.nb;

  return channel;
}

static int l2cap_btstack_send(int channel, const unsigned char* buf, int len, int ignored)
{
  if(!channels.entries[channel].cid)
  {
    fprintf(stderr, "connection is still pending\n");
    return -1;
  }

  return btstack_common_send_packet(L2CAP_DATA_PACKET, channels.entries[channel].cid, buf, len);
}

static int l2cap_btstack_listen(int user, unsigned short psm, int options,
    L2CAP_ABS_LISTEN_ACCEPT_CALLBACK read_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
  //TODO MLA
  fprintf(stderr, "btstack_listen is not implemented yet\n");
  return -1;
}

static int l2cap_btstack_close(int channel)
{
  //TODO MLA
  fprintf(stderr, "btstack_close is not implemented yet\n");
  return -1;
}

static void l2cap_btstack_add_source(int channel, int user,
    L2CAP_ABS_READ_CALLBACK read_callback, L2CAP_ABS_PACKET_CALLBACK packet_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
  channels.entries[channel].user = user;
  channels.entries[channel].packet_callback = packet_callback;
}

static int l2cap_btstack_recv (int channel, unsigned char* buf, int len)
{
  fprintf(stderr, "btstack uses packet callbacks!\n");
  return -1;
}

static int l2cap_btstack_disconnect(int channel)
{
  return btstack_common_disconnect(channels.entries[channel].handle);
}

static s_l2cap_abs l2cap_btstack =
{
    .connect = l2cap_btstack_connect,
    .listen = l2cap_btstack_listen,
    .send = l2cap_btstack_send,
    .recv = l2cap_btstack_recv,
    .close = l2cap_btstack_close,
    .add_source = l2cap_btstack_add_source,
    .disconnect = l2cap_btstack_disconnect,
};

void l2cap_btstack_init(void) __attribute__((constructor (101)));
void l2cap_btstack_init(void)
{
  l2cap_abs_register(E_BT_ABS_BTSTACK, &l2cap_btstack);
}
