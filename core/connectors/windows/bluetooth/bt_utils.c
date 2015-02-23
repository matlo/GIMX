/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <connectors/tcp_con.h>
#include <connectors/bt_utils.h>
#include <hci.h>
#include <errno.h>

#define BTSTACK_TIMEOUT   1 // 1 second

#define BTSTACK_ADDR "127.0.0.1"
#define BTSTACK_PORT 13333

int bachk(const char *str)
{
  if (!str)
    return -1;

  if (strlen(str) != 17)
    return -1;

  while (*str) {
    if (!isxdigit(*str++))
      return -1;

    if (!isxdigit(*str++))
      return -1;

    if (*str == 0)
      break;

    if (*str++ != ':')
      return -1;
  }

  return 0;
}

void baswap(bdaddr_t *dst, bdaddr_t *src)
{
  unsigned char *d = (unsigned char *) dst;
  unsigned char *s = (unsigned char *) src;
  unsigned int i;

  for (i = 0; i < 6; i++)
  {
    d[i] = s[5 - i];
  }
}

int ba2str(const bdaddr_t *ba, char *str)
{
  return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
    ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int str2ba(const char *str, bdaddr_t *ba)
{
  bdaddr_t b;
  int i;

  if (bachk(str) < 0) {
    memset(ba, 0, sizeof(*ba));
    return -1;
  }

  for (i = 0; i < 6; i++, str += 3)
    b.b[i] = strtol(str, NULL, 16);

  baswap(ba, &b);

  return 0;
}

static uint8_t hci_cmd_buffer[HCI_ACL_BUFFER_SIZE];

// send hci cmd packet
int bt_send_cmd(int fd, const hci_cmd_t *cmd, ...) {

  if(fd < 0)
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
  if(tcp_send(fd, header, 6) < 0)
  {
    fprintf(stderr, "Failed to send hci command.\n");
    return -1;
  }
  if(tcp_send(fd, hci_cmd_buffer, len) < 0)
  {
    fprintf(stderr, "Failed to send hci command.\n");
    return -1;
  }

  return 0;
}

int bt_recv(int fd, recv_data_t* recv_data)
{
  int status = 0;

  if(fd < 0)
  {
    fprintf(stderr, "No connection to btstack.\n");
    return -1;
  }

  int ret;

  if(recv_data->read < sizeof(packet_header_t))
  {
    recv_data->remaining = sizeof(packet_header_t) - recv_data->read;
  }

  ret = tcp_recv(fd, recv_data->buffer+recv_data->read, recv_data->remaining);
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

int bt_recv_packet(int fd, recv_data_t* recv_data)
{
  fd_set fdread, fderr;
  recv_data->read = 0;
  recv_data->remaining = 0;
  while (1)
  {
    struct timeval tv = { .tv_sec = BTSTACK_TIMEOUT };
    FD_ZERO(&fdread);
    FD_SET(fd,&fdread);
    fderr = fdread;

    int ret = select(fd+1, &fdread, NULL, &fderr, &tv);

    if(ret < 0)
    {
      psockerror("select");
      break;
    }
    else if(ret > 0)
    {
      if(FD_ISSET(fd, &fderr))
      {
        fprintf(stderr, "connection with btstack is broken\n");
        break;
      }
      else if(FD_ISSET(fd, &fdread))
      {
        int status = bt_recv(fd, recv_data);
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

static recv_data_t recv_data = {};

int bt_connect(int fd)
{
  int ret = 0;

  fd = tcp_connect(inet_addr(BTSTACK_ADDR), BTSTACK_PORT);
  if(fd < 0)
  {
    return -1;
  }

  ret = bt_send_cmd(fd, &btstack_set_power_mode, HCI_POWER_ON );

  while(ret >= 0)
  {
    ret = bt_recv_packet(fd, &recv_data);
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
    tcp_close(fd);
    fd = -1;
  }
  else
  {
    ret = fd;
  }

  return ret;
}

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param device_number  the device number
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_get_device_bdaddr(int fd, int device_number, bdaddr_t* bdaddr)
{
  int ret;

  ret = bt_send_cmd(fd, &hci_read_bd_addr);

  while(ret >= 0)
  {
    ret = bt_recv_packet(fd, &recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr))
        {
          int i;
          for(i=0; i<sizeof(bdaddr->b); ++i)
          {
            bdaddr->b[i] = packet[OFFSET_OF_DATA_IN_COMMAND_COMPLETE + 1 + i];
          }
          break;
        }
      }
    }
  }

  return ret;
}

/*
 * \brief This function writes the device class for a given device number.
 *
 * \param device_number  the device number
 * \param devclass       the device class to write
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_write_device_class(int fd, int device_number, uint32_t devclass)
{
  int ret = bt_send_cmd(fd, &hci_write_class_of_device, devclass);

  while(ret >= 0)
  {
    ret = bt_recv_packet(fd, &recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if ( COMMAND_COMPLETE_EVENT(packet, hci_write_class_of_device) ) {
          break;
        }
      }
    }
  }

  return ret;
}

int bt_disconnect(int fd, unsigned short handle)
{
  int ret = bt_send_cmd(fd, &hci_disconnect, handle, 0x13);

  while(ret >= 0)
  {
    ret = bt_recv_packet(fd, &recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if ( COMMAND_COMPLETE_EVENT(packet, hci_disconnect) ) {
          break;
        }
      }
    }
  }

  ret = tcp_close(fd);

  return ret;
}
