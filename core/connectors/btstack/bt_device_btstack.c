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
#include <connectors/bluetooth/bt_device_abs.h>
#include <hci.h>

#define ACL_MTU 1024
#define L2CAP_MTU 1024

static recv_data_t recv_data = { {}, 0, 0 };

int bt_device_btstack_device_init()
{
  return btstack_common_init();
}

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_device_btstack_get_device_bdaddr(int ignored, bdaddr_t* bdaddr)
{
  int ret;

  ret = btstack_common_send_cmd(&btstack_hci_read_bd_addr);

  while(ret >= 0)
  {
    ret = btstack_common_recv_packet(&recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if (COMMAND_COMPLETE_EVENT(packet, btstack_hci_read_bd_addr))
        {
          unsigned int i;
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
 * \param devclass       the device class to write
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_device_btstack_write_device_class(int ignored, uint32_t devclass)
{
  int ret = btstack_common_send_cmd(&btstack_hci_write_class_of_device, devclass);

  while(ret >= 0)
  {
    ret = btstack_common_recv_packet(&recv_data);
    if(ret > 0)
    {
      int type = READ_BT_16(recv_data.buffer, 0);
      unsigned char* packet = recv_data.buffer+sizeof(packet_header_t);
      if(type == HCI_EVENT_PACKET)
      {
        if ( COMMAND_COMPLETE_EVENT(packet, btstack_hci_write_class_of_device) ) {
          break;
        }
      }
    }
  }

  return ret;
}

static s_bt_device_abs bt_device_btstack =
{
    .init = bt_device_btstack_device_init,
    .get_bdaddr = bt_device_btstack_get_device_bdaddr,
    .write_device_class = bt_device_btstack_write_device_class,
};

void bt_device_btstack_init(void) __attribute__((constructor (101)));
void bt_device_btstack_init(void)
{
  bt_device_abs_register(E_BT_ABS_BTSTACK, &bt_device_btstack);
}
