/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <guhid.h>
#include <common/gerror.h>

#ifndef __ARM_ARCH_6__
#include <linux/uhid.h>
#else
#include "rpi/uhid.h"
#endif
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/inotify.h>
#include <linux/input.h>
#include <limits.h>

#define UHID_MAX_DEVICES 256

#define UHID_PATH "/dev/uhid"

static struct {
    int fd;
    int opened;
} uhidasync_devices[UHID_MAX_DEVICES] = { };

void uhidasync_init(void) __attribute__((constructor));
void uhidasync_init(void) {
    int i;
    for (i = 0; i < UHID_MAX_DEVICES; ++i) {
        uhidasync_devices[i].fd = -1;
    }
}

void uhidasync_clean(void) __attribute__((destructor));
void uhidasync_clean(void) {
    int i;
    for (i = 0; i < UHID_MAX_DEVICES; ++i) {
        if (uhidasync_devices[i].fd >= 0) {
            guhid_close(i);
        }
    }
}

static inline int uhidasync_check_device(int device, const char * file, unsigned int line, const char * func) {
    if (device < 0 || device >= UHID_MAX_DEVICES) {
        fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
        return -1;
    }
    if (uhidasync_devices[device].fd < 0) {
        fprintf(stderr, "%s:%d %s: no such device\n", file, line, func);
        return -1;
    }
    return 0;
}
#define UHIDASYNC_CHECK_DEVICE(device,retValue) \
  if(uhidasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static int add_device(int fd) {
    int i;
    for (i = 0; i < UHID_MAX_DEVICES; ++i) {
        if (uhidasync_devices[i].fd == -1) {
            uhidasync_devices[i].fd = fd;
            return i;
        }
    }
    return -1;
}

static int uhid_write(int fd, const struct uhid_event *ev) {

    ssize_t ret = write(fd, ev, sizeof(*ev));
    if (ret < 0) {
        PRINT_ERROR_ERRNO("write")
        return -1;
    } else if (ret != sizeof(*ev)) {
        fprintf(stderr, "Wrong size written to uhid: %zu != %zu\n", ret, sizeof(ev));
        return -1;
    } else {
        return 0;
    }
}

typedef struct __attribute__((packed))
{
    unsigned char bSize : 2;
    unsigned char bType : 2;
    unsigned char bTag : 4;
} s_item_header;

#define BTYPE_LOCAL 2

#define BTAG_LONG_ITEM 0b1111

typedef struct __attribute__((packed))
{
    s_item_header header;
    unsigned char data[0];
} s_short_item;

#define USAGE_JOYSTICK             0x04
#define USAGE_GAMEPAD              0x05
#define USAGE_MULTIAXIS_CONTROLLER 0x08

typedef struct __attribute__((packed))
{
    s_item_header header;
    unsigned char bDataSize;
    unsigned char bLongItemTag;
    unsigned char data[0];
} s_long_item;

static void fix_rdesc_usage(unsigned char * rdesc, unsigned short size) {
  unsigned char * pos;
  unsigned char dataSize, headerSize;
  for (pos = rdesc; pos + 1 < rdesc + size; pos += (dataSize + headerSize)) {
    s_item_header * header = (s_item_header *)pos;
    if (header->bTag == BTAG_LONG_ITEM) {
      s_long_item * item = (s_long_item *)pos;
      dataSize = item->bDataSize;
      headerSize = sizeof(item->header) + sizeof(item->bDataSize) + sizeof(item->bLongItemTag);
      if(pos + headerSize + dataSize >= rdesc + size) {
        PRINT_ERROR_OTHER("invalid report descriptor")
        break;
      }
    } else {
      s_short_item * item = (s_short_item *)pos;
      static unsigned char sizes[] = { 0, 1, 2, 4 };
      dataSize = sizes[item->header.bSize];
      headerSize = sizeof(item->header);
      if(pos + headerSize + dataSize >= rdesc + size) {
        PRINT_ERROR_OTHER("invalid report descriptor")
        break;
      }
      if (dataSize == 1 && item->header.bType == BTYPE_LOCAL) {
        switch (item->data[0])
        {
        case USAGE_JOYSTICK:
        case USAGE_GAMEPAD:
          item->data[0] = USAGE_MULTIAXIS_CONTROLLER;
          break;
        default:
          break;
        }
      }
    }
  }
}

/*
 * Setup an inotify watch (creation only) on /dev/input.
 */
static int setup_watch() {

    int ifd = inotify_init();
    if (ifd < 0) {
        PRINT_ERROR_ERRNO("inotify_init")
        return -1;
    }

    if (inotify_add_watch(ifd, "/dev/input", IN_CREATE) < 0) {
        PRINT_ERROR_ERRNO("inotify_add_watch")
        close(ifd);
        return -1;
    }

    return ifd;
}

/*
 * Wait up to 5 seconds for a specific event device to appear.
 */
static int wait_watch(int ifd, const char * uniq) {

    int ret = 0;

    fd_set readfds;
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    struct inotify_event * event;
    // enough to read at least one event, but we may get more than one
    unsigned char buf[sizeof(*event) + PATH_MAX + 1] = { };

    while (ret == 0) {
        FD_ZERO(&readfds);
        FD_SET(ifd, &readfds);
        int status = select(ifd + 1, &readfds, NULL, NULL, &tv);
        if (status > 0) {
            if (FD_ISSET(ifd, &readfds)) {
                int res = read(ifd, buf, sizeof(buf));
                if (res < 0) {
                  PRINT_ERROR_ERRNO("read")
                  ret = -1;
                } else {
                    int i;
                    for (i = 0; i < res; i += (sizeof(*event) + event->len)) {
                        event = (struct inotify_event *) (buf + i);
                        if (event->len > 0) {
                            char path[sizeof("/dev/input/") + event->len];
                            strcpy(path, "/dev/input/");
                            strncat(path, event->name, event->len);
                            int dev_fd = open(path, O_RDONLY);
                            if (dev_fd >= 0) {
                                char buf[64] = { };
                                if (ioctl(dev_fd, EVIOCGUNIQ(sizeof(buf)), &buf) != -1) {
                                    if (!strncmp(buf, uniq, sizeof(buf))) {
                                        ret = 1;
                                    }
                                }
                                close(dev_fd);
                            }
                        }
                    }
                }
            }
        } else if (status == 0) {
            PRINT_ERROR_OTHER("select timed out")
            ret = -1;
        } else {
            if (errno == EINTR) {
                continue;
            } else {
                PRINT_ERROR_ERRNO("select")
                ret = -1;
            }
        }
    }

    return ret;
}

int guhid_create(const s_hid_info * hid_info, int hid) {

    if (hid_info == NULL) {

        PRINT_ERROR_OTHER("invalid argument")
        return -1;
    }

    int fd = open(UHID_PATH, O_RDWR | O_NONBLOCK);

    if (fd < 0) {
        PRINT_ERROR_ERRNO("open")
        return -1;
    }

    int device = add_device(fd);

    if (device < 0) {
        close(fd);
        return -1;
    }

    // Change Joystick and Gamepad usages to Multiaxis Controller usage.
    // This prevents the kernel from applying deadzones.
    fix_rdesc_usage(hid_info->reportDescriptor, hid_info->reportDescriptorLength);

    struct uhid_event ev = {
            .type = UHID_CREATE,
            .u.create = {
                    .rd_data = hid_info->reportDescriptor,
                    .rd_size = hid_info->reportDescriptorLength,
                    .version = hid_info->version,
                    .country = hid_info->countryCode,
                    // Make sure no device specific driver is loaded.
                    .bus = BUS_VIRTUAL,
                    .vendor = hid_info->vendor_id,
                    .product = hid_info->product_id,
            }
    };

    char * dest = (char *) ev.u.create.name;
    if(hid_info->manufacturerString) {
        strncat(dest, hid_info->manufacturerString, sizeof(ev.u.create.name) - 1);
    }
    if(hid_info->productString) {
        if(hid_info->manufacturerString) {
            strncat(dest, " ", sizeof(ev.u.create.name) - strlen(dest) - 1);
        }
        strncat(dest, hid_info->productString, sizeof(ev.u.create.name) - strlen(dest) - 1);
    }

    if (!strlen(dest)) {
        snprintf(dest, sizeof(ev.u.create.name), "HID %04x:%04x", hid_info->vendor_id, hid_info->product_id);
    }

    snprintf((char *) ev.u.create.uniq, sizeof(ev.u.create.uniq), "GIMX %d %d", getpid(), hid);

    int ifd = setup_watch();
    if (ifd < 0) {
        guhid_close(device);
        return -1;
    }

    if (uhid_write(fd, &ev) < 0) {
        close(ifd);
        guhid_close(device);
        return -1;
    }

    if (wait_watch(ifd, (char *) ev.u.create.uniq) < 0) {
        close(ifd);
        guhid_close(device);
        return -1;
    }

    close(ifd);

    return device;
}

static int uhid_read(int device) {

    struct uhid_event ev;
    ssize_t ret;

    memset(&ev, 0, sizeof(ev));
    ret = read(uhidasync_devices[device].fd, &ev, sizeof(ev));
    if (ret == -1) {
        if (errno == EAGAIN) {
            return 0;
        }
        PRINT_ERROR_ERRNO("read")
        return -1;
    }

    switch (ev.type) {
    case UHID_START:
        break;
    case UHID_STOP:
        break;
    case UHID_OPEN:
        uhidasync_devices[device].opened = 1;
        break;
    case UHID_CLOSE:
        uhidasync_devices[device].opened = 0;
        break;
    case UHID_OUTPUT:
        break;
    case UHID_OUTPUT_EV:
        break;
    default:
        break;
    }

    return ret;
}

int guhid_is_opened(int device) {

    UHIDASYNC_CHECK_DEVICE(device, 0)

    while(uhid_read(device) > 0) {}

    return uhidasync_devices[device].opened;
}

int guhid_close(int device) {

    UHIDASYNC_CHECK_DEVICE(device, -1)

    struct uhid_event ev = { .type = UHID_DESTROY };
    uhid_write(uhidasync_devices[device].fd, &ev);
    close(uhidasync_devices[device].fd);
    uhidasync_devices[device].fd = -1;
    uhidasync_devices[device].opened = 0;

    return 1;
}

int guhid_write(int device, const void * buf, unsigned int count) {

    UHIDASYNC_CHECK_DEVICE(device, -1)

    if (count > UHID_DATA_MAX) {

        PRINT_ERROR_OTHER("count is higher than UHID_DATA_MAX")
        return -1;
    }

    struct uhid_event ev = { .type = UHID_INPUT, .u.input.size = count };
    memcpy(ev.u.input.data, buf, count);

    return uhid_write(uhidasync_devices[device].fd, &ev);
}

