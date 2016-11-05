/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ghid.h>
#include "gusbhid.h"

int ghid_init() {

    return gusbhid_init();
}

int ghid_exit() {

    return gusbhid_exit();
}

/*
 * \brief Open a hid device.
 *
 * \param device_path  the path of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. bad path, device already opened).
 */
int ghid_open_path(const char * device_path) {

    return gusbhid_open_path(device_path);
}

/*
 * \brief Open a hid device.
 *
 * \param vendor   the vendor id of the hid device to open.
 * \param product  the product id of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. no device found).
 */
int ghid_open_ids(unsigned short vendor, unsigned short product) {

  return gusbhid_open_ids(vendor, product);
}

/*
 * \brief Enumerate hid devices.
 *
 * \param vendor  the vendor id to look for
 * \param product the product id to look for (ignored if vendor is 0)
 *
 * \return the hid devices
 */
struct ghid_device * ghid_enumerate(unsigned short vendor, unsigned short product) {

    return gusbhid_enumerate(vendor, product);
}

/*
 * \brief Free the memory allocated by hidasync_enumerate.
 *
 * \param hid_devs  the hid devices returned by hidasync_enumerate
 */
void ghid_free_enumeration(struct ghid_device * hid_devs) {

    gusbhid_free_enumeration(hid_devs);
}

/*
 * \brief Get info for a hid device.
 *
 * \param device  the identifier of the hid device
 *
 * \return the hid info
 */
const s_hid_info * ghid_get_hid_info(int device) {

    return gusbhid_get_hid_info(device);
}

/*
 * \brief Read from a hid device, with a timeout. Use this function in a synchronous context.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in milliseconds
 *
 * \return the number of bytes actually read
 */
int ghid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  return gusbhid_read_timeout(device, buf, count, timeout);
}

/*
 * \brief Register the device as an event source, and set the external callbacks. \
 * This function triggers an asynchronous context. \
 * The fp_read callback is responsible for setting the next read size.
 *
 * \param device      the hid device
 * \param user        the user to pass to the external callback
 * \param callbacks   the device callbacks
 *
 * \return 0 in case of success, or -1 in case of error
 */
int ghid_register(int device, int user, const GHID_CALLBACKS * callbacks) {
    
    return gusbhid_register(device, user, callbacks);
}

/*
 * \brief Read from a hid device, asynchronously.
 *
 * \param device the hid device
 *
 * \return 0 in case of success, or -1 in case of error
 */
int ghid_poll(int device) {

    return gusbhid_poll(device);
}

/*
 * \brief Write to a serial device, with a timeout. Use this function in a synchronous context.
 *
 * \param device  the identifier of the serial device
 * \param buf     the buffer containing the data to write
 * \param count   the number of bytes in buf
 * \param timeout the maximum time to wait for the completion, in milliseconds
 *
 * \return the number of bytes actually written (0 in case of timeout)
 */
int ghid_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

    return gusbhid_write_timeout(device, buf, count, timeout);
}

/*
 * \brief Send data to a serial device. Use this function in an asynchronous context.
 *
 * \param device  the identifier of the serial device
 * \param buf     the buffer containing the data to send
 * \param count   the maximum number of bytes to send
 *
 * \return -1 in case of error, 0 in case of pending write, or the number of bytes written
 */
int ghid_write(int device, const void * buf, unsigned int count) {

    return gusbhid_write(device, buf, count);
}

/*
 * \brief Close a hid device.
 *
 * \param device  the identifier of the hid device to close.
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int ghid_close(int device) {

    return gusbhid_close(device);
}

