/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>

#define HCI_REQ_TIMEOUT   1000

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param device_number  the device number
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
int get_device_bdaddr(int device_number, char bdaddr[18])
{
  int ret = 0;

  bdaddr_t bda;

  int s = hci_open_dev (device_number);

  if(hci_read_bd_addr(s, &bda, HCI_REQ_TIMEOUT) < 0)
  {
    ret = -1;
  }
  else
  {
    ba2str(&bda, bdaddr);
  }

  close(s);

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
int write_device_class(int device_number, uint32_t devclass)
{
  int ret = 0;

  int s = hci_open_dev (device_number);

  if(hci_write_class_of_dev(s, devclass, HCI_REQ_TIMEOUT) < 0)
  {
    ret = -1;
  }

  close(s);

  return ret;
}

