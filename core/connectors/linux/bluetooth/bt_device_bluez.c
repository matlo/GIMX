/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <connectors/bluetooth/bt_device_abs.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define HCI_REQ_TIMEOUT   1000

static int bt_device_bluez_device_init()
{
  //TODO MLA: move bt_mgmt_adapter_init here
  return 0;
}

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param device_number  the device number
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
static int bt_device_bluez_get_device_bdaddr(int device_number, bdaddr_t* ba)
{
  int ret = 0;

  int s = hci_open_dev (device_number);
  if(s >= 0)
  {
    ret = hci_read_bd_addr(s, ba, HCI_REQ_TIMEOUT);
    if(ret < 0)
    {
      perror("hci_read_bd_addr");
    }
    hci_close_dev(s);
  }
  else
  {
    perror("hci_open_dev");
    ret = -1;
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
static int bt_device_bluez_write_device_class(int device_number, uint32_t devclass)
{
  int ret = 0;

  int s = hci_open_dev (device_number);
  if(s >= 0)
  {
    ret = hci_write_class_of_dev(s, devclass, HCI_REQ_TIMEOUT);
    if(ret < 0)
    {
      perror("hci_write_class_of_dev");
    }
    hci_close_dev(s);
  }
  else
  {
    perror("hci_open_dev");
    ret = -1;
  }

  return ret;
}

static s_bt_device_abs bt_device_bluez =
{
    .init = bt_device_bluez_device_init,
    .get_bdaddr = bt_device_bluez_get_device_bdaddr,
    .write_device_class = bt_device_bluez_write_device_class,
};

void bt_device_bluez_init(void) __attribute__((constructor (101)));
void bt_device_bluez_init(void)
{
  bt_device_abs_register(E_BT_ABS_BLUEZ, &bt_device_bluez);
}
