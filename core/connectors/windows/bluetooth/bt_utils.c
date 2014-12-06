/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <stdint.h>

#define HCI_REQ_TIMEOUT   1000

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
