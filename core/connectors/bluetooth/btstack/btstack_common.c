/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include "btstack_common.h"
#include <connectors/tcp_con.h>
#include <stdio.h>

#define BTSTACK_TIMEOUT   1 // 1 second

#define BTSTACK_ADDR "127.0.0.1"
#define BTSTACK_PORT 13333

static int btstack_fd = -1;

static recv_data_t recv_data = { {}, 0, 0 };

static uint8_t hci_cmd_buffer[HCI_ACL_BUFFER_SIZE];

int btstack_common_getfd()
{
  return btstack_fd;
}

int btstack_common_init()
{
  int ret = 0;

  btstack_fd = tcp_connect(inet_addr(BTSTACK_ADDR), BTSTACK_PORT);
  if(btstack_fd < 0)
  {
    return -1;
  }

  ret = btstack_common_send_cmd(&btstack_set_power_mode, HCI_POWER_ON );

  while(ret >= 0)
  {
    ret = btstack_common_recv_packet(&recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if(packet[0] == BTSTACK_EVENT_STATE)
        {
          if (packet[2] == HCI_STATE_WORKING)
          {
            break;
          }
        }
      }
    }
  }

  if(ret < 0)
  {
    tcp_close(btstack_fd);
    btstack_fd = -1;
  }

  return ret;
}

int btstack_common_recv(recv_data_t* recv_data)
{
  int status = 0;

  if(btstack_fd < 0)
  {
    fprintf(stderr, "No connection to btstack.\n");
    return -1;
  }

  int ret;

  if(recv_data->read < sizeof(packet_header_t))
  {
    recv_data->remaining = sizeof(packet_header_t) - recv_data->read;
  }

  ret = tcp_recv(btstack_fd, recv_data->buffer+recv_data->read, recv_data->remaining);
  if(ret > 0)
  {
    recv_data->read += ret;
    recv_data->remaining -= ret;
  }
  else if( ret < 0)
  {
    status = -1;
  }

  if(recv_data->read == sizeof(packet_header_t))
  {
    recv_data->remaining = READ_BT_16( recv_data->buffer, 4);
  }
  else if(!recv_data->remaining)
  {
    status = 1;
    recv_data->read = 0;
  }

  return status;
}

int btstack_common_recv_packet(recv_data_t* data)
{
  fd_set fdread, fderr;
  data->read = 0;
  data->remaining = 0;
  while (1)
  {
    struct timeval tv = { .tv_sec = BTSTACK_TIMEOUT };
    FD_ZERO(&fdread);
    FD_SET(btstack_fd,&fdread);
    fderr = fdread;

    int ret = select(btstack_fd+1, &fdread, NULL, &fderr, &tv);

    if(ret < 0)
    {
      psockerror("select");
      break;
    }
    else if(ret > 0)
    {
      if(FD_ISSET(btstack_fd, &fderr))
      {
        fprintf(stderr, "connection with btstack is broken\n");
        break;
      }
      else if(FD_ISSET(btstack_fd, &fdread))
      {
        int status = btstack_common_recv(data);
        if(status)
        {
          return status;
        }
      }
    }
    else
    {
      break;
    }
  }
  return -1;
}

int btstack_common_send_packet(uint16_t type, uint16_t cid, const unsigned char * buf, uint32_t len)
{
  uint8_t header[sizeof(packet_header_t)];
  bt_store_16(header, 0, type);
  bt_store_16(header, 2, cid);
  bt_store_16(header, 4, len);

  // Linux, MinGW and Cygwin
  if(tcp_send(btstack_fd, (const void*)header, 6) < 0)
  {
    return -1;
  }
  if(tcp_send(btstack_fd, (const void*)buf, len) < 0)
  {
    return -1;
  }

  return len;
}

// send hci cmd packet
int btstack_common_send_cmd(const hci_cmd_t *cmd, ...)
{

  if(btstack_fd < 0)
  {
    fprintf(stderr, "No connection to btstack.\n");
    return -1;
  }

  va_list argptr;
  va_start(argptr, cmd);
  uint16_t len = hci_create_cmd_internal(hci_cmd_buffer, cmd, argptr);
  va_end(argptr);

  uint8_t header[sizeof(packet_header_t)];
  bt_store_16(header, 0, HCI_COMMAND_DATA_PACKET);
  bt_store_16(header, 2, 0);
  bt_store_16(header, 4, len);
  if(tcp_send(btstack_fd, header, 6) < 0)
  {
    fprintf(stderr, "Failed to send hci command.\n");
    return -1;
  }
  if(tcp_send(btstack_fd, hci_cmd_buffer, len) < 0)
  {
    fprintf(stderr, "Failed to send hci command.\n");
    return -1;
  }

  return 0;
}

int btstack_common_disconnect(uint16_t handle)
{
  int ret = btstack_common_send_cmd(&btstack_hci_disconnect, handle, 0x13);

  while(ret >= 0)
  {
    ret = btstack_common_recv_packet(&recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if ( COMMAND_COMPLETE_EVENT(packet, btstack_hci_disconnect) ) {
          break;
        }
      }
    }
  }

  ret = tcp_close(btstack_fd);

  return ret;
}
