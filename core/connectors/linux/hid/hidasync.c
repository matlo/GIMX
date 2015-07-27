/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "connectors/hidasync.h"
#include <stdio.h>
#include <linux/hidraw.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <stdlib.h>

static struct {
    int fd;
    unsigned short vendor;
    unsigned short product;
    char * path;
    struct {
        int user;
        int (*fp_read)(int, const char * buf, unsigned int count);
    } callback;
} devices[HIDASYNC_MAX_DEVICES] = { };

#define CHECK_DEVICE(device) \
    if(device < 0 || device >= HIDASYNC_MAX_DEVICES || devices[device].fd < 0) { \
        fprintf(stderr, "%s:%d %s: no such device (%d)\n", __FILE__, __LINE__, __func__, device); \
        return -1; \
    }

static int add_device(const char * device_path, int fd, unsigned short vendor, unsigned short product) {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        if(devices[i].path && !strcmp(devices[i].path, device_path)) {
            fprintf(stderr, "%s:%d add_device %s: device already opened\n", __FILE__, __LINE__, device_path);
            return -1;
        }
    }
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        if(devices[i].fd == -1) {
            devices[i].path = strdup(device_path);
            if(devices[i].path != NULL) {
                devices[i].fd = fd;
                devices[i].vendor = vendor;
                devices[i].product = product;
                return i;
            }
            else {
                fprintf(stderr, "%s:%d add_device %s: can't duplicate path\n", __FILE__, __LINE__, device_path);
                return -1;
            }
        }
    }
    return -1;
}

/*
 * \brief Initialize internal structures. \
 * This function has to be called once before using any other hidasync function.
 */
void hidsaync_init() {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        devices[i].fd = -1;
    }
}

/*
 * \brief Close all opened devices.
 */
void hidasync_quit() {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        hidasync_close(i);
    }
}

/*
 * \brief Open a hid device.
 *
 * \param device_path  the path of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. bad path, device already opened).
 */
int hidasync_open_path(const char * device_path) {
    int ret = -1;
    if(device_path != NULL) {
        int fd = open(device_path, O_RDWR | O_NONBLOCK);
        if(fd >= 0) {
            struct hidraw_devinfo info;
            if(ioctl(fd, HIDIOCGRAWINFO, &info) != -1) {
                ret = add_device(device_path, fd, info.vendor, info.product);
                if(ret == -1) {
                    close(fd);
                }
            }
            else {
                fprintf(stderr, "%s:%d ioctl HIDIOCGRAWINFO (%s): %m\n", __FILE__, __LINE__, device_path);
                close(fd);
            }
        }
        else {
            fprintf(stderr, "%s:%d open %s: %m\n", __FILE__, __LINE__, device_path);
        }
    }
    return ret;
}

#define DEV "/dev"
#define HIDRAW_DEV_NAME "hidraw"

static int is_hidraw_device(const struct dirent *dir) {
  return strncmp(HIDRAW_DEV_NAME, dir->d_name, sizeof(HIDRAW_DEV_NAME)-1) == 0;
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
int hidasync_open_ids(unsigned short vendor, unsigned short product)
{
  int ret = -1;

  char path[sizeof("/dev/hidraw255")];
  struct dirent ** namelist_hidraw;
  int n_hidraw;
  int i;

  // scan /dev for hidrawX devices
  n_hidraw = scandir(DEV, &namelist_hidraw, is_hidraw_device, alphasort);
  if (n_hidraw >= 0)
  {
    for (i = 0; i < n_hidraw; ++i)
    {
      snprintf(path, sizeof(path), "%s/%s", DEV, namelist_hidraw[i]->d_name);

      int device = hidasync_open_path(path);

      if(device >= 0) {
        if(devices[device].vendor == vendor && devices[device].product == product)
        {
          ret = device;
          break;
        }
        hidasync_close(device);
      }
    }
  }

  return ret;
}

/*
 * \brief Get the USB ids of a hid device.
 *
 * \param device  the identifier of the hid device
 * \param vendor  where to store the vendor id
 * \param product where to store the product id
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int hidasync_get_ids(int device, unsigned short * vendor, unsigned short * product) {

    CHECK_DEVICE(device)

    *vendor = devices[device].vendor;
    *product = devices[device].product;

    return 0;
}

/*
 * \brief Close a hid device.
 *
 * \param device  the identifier of the hid device to close.
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int hidasync_close(int device) {

    CHECK_DEVICE(device)

    if (close(devices[device].fd) == -1) {
        fprintf(stderr, "%s:%d close: %m\n", __FILE__, __LINE__);
    }
    devices[device].fd = -1;
    free(devices[device].path);
    devices[device].path = NULL;

    return 0;
}

/*
 * \brief Read from a hid device, with a timeout.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in seconds
 *
 * \return the number of bytes actually read
 */
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

    int bread = 0;
    fd_set readfds;
    struct timeval tv = { .tv_sec = timeout, .tv_usec = 0 };

    while (bread != count) {
        FD_ZERO(&readfds);
        FD_SET(devices[device].fd, &readfds);
        int status = select(devices[device].fd + 1, &readfds, NULL, NULL, &tv);
        if (status > 0) {
            if (FD_ISSET(devices[device].fd, &readfds)) {
                unsigned int res = read(devices[device].fd, buf, count - bread);
                if (res > 0) {
                    bread += res;
                }
                else {
                    fprintf(stderr, "%s:%d read: %m\n", __FILE__, __LINE__);
                }
            }
        } else if (status == EINTR) {
            continue;
        } else {
            fprintf(stderr, "%s:%d select: %m\n", __FILE__, __LINE__);
            break;
        }
    }

    return bread;
}

/*
 * \brief the callback for hid devices that are added as event sources.
 *
 * \param id  the instance id
 *
 * \return 0 in case of a success, -1 in case of an error
 */
static int hidasync_read(int device) {

    CHECK_DEVICE(device)

    int ret = 0;
    char * buf[HIDASYNC_MAX_TRANSFER_SIZE];

    unsigned int res = read(devices[device].fd, buf, sizeof(buf));
    if(res == -1) {
        fprintf(stderr, "%s:%d read: %m\n", __FILE__, __LINE__);
        ret = -1;
    }
    else {
        ret = devices[device].callback.fp_read(devices[device].callback.user, (const char *)buf, res);
    }

    return ret;
}

/*
 * \brief Register the device as an event source, and set the external callback to call on data reception.
 *
 * \param device      the hid device
 * \param user        the user to pass to the external callback
 * \param fp_read     the external callback
 * \param fp_register the function to register the device as an event source
 *
 * \return 0 in case of success, or -1 in case of error
 */
int hidasync_register(int device, int user, HIDASYNC_READ_CALLBACK fp_read, HIDASYNC_REGISTER_SOURCE fp_register) {

    CHECK_DEVICE(device)

    devices[device].callback.user = user;
    devices[device].callback.fp_read = fp_read;

    fp_register(devices[device].fd, device, hidasync_read, NULL, hidasync_close);

    return 0;
}

/*
 * \brief Send data to a hid device.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer containing the data to send
 * \param count   the maximum number of bytes to send
 *
 * \return 0 in case of success, or -1 in case of error
 */
int hidasync_write(int device, const void * buf, unsigned int count) {

    CHECK_DEVICE(device)

    int ret = 0;

    unsigned int res = write(devices[device].fd, buf, count);
    if (res == -1) {
        fprintf(stderr, "%s:%d write: %m\n", __FILE__, __LINE__);
        ret = -1;
    }
    else if(res != count) {
        fprintf(stderr, "%s:%d write: only %u written (requested %u)\n", __FILE__, __LINE__, res, count);
    }

    return ret;
}
