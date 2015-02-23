/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define HCI_REQ_TIMEOUT   1000

/*
 * \brief This function gets the bluetooth device address for a given device number.
 *
 * \param device_number  the device number
 * \param bdaddr         the buffer to store the bluetooth device address
 *
 * \return 0 if successful, -1 otherwise
 */
int bt_get_device_bdaddr(int device_number, bdaddr_t* bdaddr)
{
  int ret = 0;

  int s = hci_open_dev (device_number);
  if(s >= 0)
  {
    ret = hci_read_bd_addr(s, bdaddr, HCI_REQ_TIMEOUT);
    if(ret < 0)
    {
      perror("hci_read_bd_addr");
    }
    close(s);
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
int bt_write_device_class(int device_number, uint32_t devclass)
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
    close(s);
  }
  else
  {
    perror("hci_open_dev");
    ret = -1;
  }

  return ret;
}

int bt_disconnect(char bdaddr[18])
{
  int err = 0, dd;
  struct hci_conn_info_req *cr = 0;

  // find the connection handle to the specified bluetooth device
  cr = (struct hci_conn_info_req*) malloc(
      sizeof(struct hci_conn_info_req) + sizeof(struct hci_conn_info));
  str2ba(bdaddr, &cr->bdaddr);
  cr->type = ACL_LINK;
  dd = hci_open_dev(hci_get_route(&cr->bdaddr));
  if (dd < 0)
  {
    perror("hci_open_dev");
    err = -1;
    goto cleanup;
  }
  err = ioctl(dd, HCIGETCONNINFO, (unsigned long) cr);
  if (err < 0)
  {
    perror("ioctl");
    goto cleanup;
  }

  hci_disconnect(dd, cr->conn_info->handle, HCI_OE_USER_ENDED_CONNECTION, HCI_REQ_TIMEOUT);

  //wait up to 5s for the disconnect to be completed
  int i = 0;
  while(i < 50 && !(ioctl(dd, HCIGETCONNINFO, (unsigned long) cr)))
  {
    usleep(100000);
    ++i;
  }

  if(i == 50)
  {
    err = -1;
  }

  cleanup: free(cr);
  if (dd >= 0)
    close(dd);

  return err;
}
