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

#define HCI_REQ_TIMEOUT   1000

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

// connection to btstack
static int fd = -1;

typedef struct {
    uint16_t type;
    uint16_t channel;
    uint16_t length;
    uint8_t  data[0];
} packet_header_t;

static uint8_t hci_cmd_buffer[HCI_ACL_BUFFER_SIZE];

// send hci cmd packet
int bt_send_cmd(const hci_cmd_t *cmd, ...) {

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

int bt_connect()
{
  in_addr_t ip = inet_addr(BTSTACK_ADDR);
  fd = tcp_connect(ip, BTSTACK_PORT);
  return fd;
}

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param device_number  the device number
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_get_device_bdaddr(int device_number, char bdaddr[18])
{
  int ret = 0;

  bt_send_cmd(&hci_read_bd_addr);

  //TODO

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
int bt_write_device_class(int device_number, uint32_t devclass)
{
  int ret = 0;

  //TODO

  return ret;
}

int bt_disconnect(char bdaddr[18])
{
  //TODO

  return 0;
}
