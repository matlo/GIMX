/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <bluetooth/bluetooth.h>

/*
This include doesn't work... I don't now why and I will solve that latter.

#include <bluetooth/hci_lib.h>

Next extern declarations remove the compilation warnings.
*/
extern int hci_open_dev(int dev_id);
extern int hci_read_bd_addr(int dd, bdaddr_t *bdaddr, int to);
extern int hci_write_class_of_dev(int dd, uint32_t cls, int to);

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
    bdaddr_t bda;

    int s = hci_open_dev (device_number);

    if(hci_read_bd_addr(s, &bda, 1000) < 0)
    {
        return -1;
    }

    ba2str(&bda, bdaddr);
    return 0;
}

/*
 * \brief This function writes the bluetooth device class to 0x508 for a given device number.
 *
 * \param device_number  the device number
 *
 * \return 0 if successful, -1 otherwise
 */
int write_device_class(int device_number)
{
    int s = hci_open_dev (device_number);

    if(hci_write_class_of_dev(s, 0x508, 1000) < 0)
    {
        return -1;
    }

    return 0;
}
