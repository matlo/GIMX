/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#include <gpoll.h>
#include <gerror.h>
#include <stdlib.h>
#include <string.h>

static s_hidinput_driver ** drivers = NULL;
static unsigned int nb_drivers = 0;

static struct {
    s_hidinput_driver * driver;
    int opened;
    int read_pending;
    struct {
        int user;
        int (* write)(int user, int transfered);
        int (* close)(int user);
    } callbacks;
} hid_devices[HIDINPUT_MAX_DEVICES];

static inline int hidinput_check_device(int device, const char * file, unsigned int line, const char * func) {
  if (device < 0 || device >= HIDINPUT_MAX_DEVICES) {
    fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
    return -1;
  }
  if (hid_devices[device].opened == 0) {
    fprintf(stderr, "%s:%d %s: no such device\n", file, line, func);
    return -1;
  }
  return 0;
}
#define HIDINPUT_CHECK_DEVICE(device,retValue) \
  if(hidinput_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static void clear_device(int device) {

  memset(hid_devices + device, 0x00, sizeof(*hid_devices));
}

static int close_device(int device) {

    if (hid_devices[device].opened >= 0) {
        ghid_close(device);
        if (hid_devices[device].driver != NULL) {
            hid_devices[device].driver->close(device);
        }
    }

    clear_device(device);

    return 0;
}

static int read_callback(int device, const void * buf, int status) {

    HIDINPUT_CHECK_DEVICE(device, -1)

    int ret = 0;

    hid_devices[device].read_pending = 0;

    if (status > 0) {
        if (hid_devices[device].driver->process(device, buf, status) < 0) {
          ret = -1;
        }
    }

    return ret;
}

static int write_callback(int device, int status) {

    HIDINPUT_CHECK_DEVICE(device, -1)

    return hid_devices[device].callbacks.write(hid_devices[device].callbacks.user, status);
}

static int close_callback(int device) {

    HIDINPUT_CHECK_DEVICE(device, -1)

    int ret = hid_devices[device].callbacks.close(hid_devices[device].callbacks.user);

    close_device(device);

    return ret;
}

void hidinput_destructor(void) __attribute__((destructor));
void hidinput_destructor(void) {

    free(drivers);
    nb_drivers = 0;
}

int hidinput_register(s_hidinput_driver * driver) {

    void * ptr = realloc(drivers, (nb_drivers + 1) * sizeof(*drivers));
    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc")
        return -1;
    }
    drivers = ptr;
    drivers[nb_drivers] = driver;
    ++nb_drivers;
    return 0;
}

int hidinput_init(int(*callback)(GE_Event*)) {

    if (callback == NULL) {
      PRINT_ERROR_OTHER("callback is NULL")
      return -1;
    }

    unsigned int driver;
    for (driver = 0; driver < nb_drivers; ++driver) {
        drivers[driver]->init(callback);
    }

    s_hid_dev * hid_devs = ghid_enumerate(0x0000, 0x0000);
    s_hid_dev * current;
    for (current = hid_devs; current != NULL; ++current) {
        for (driver = 0; driver < nb_drivers; ++driver) {
            unsigned int id;
            for (id = 0; drivers[driver]->ids[id].vendor_id != 0; ++id) {
                if (drivers[driver]->ids[id].vendor_id == current->vendor_id
                        && drivers[driver]->ids[id].product_id == current->product_id
                        && (drivers[driver]->ids[id].interface_number == -1
                                || drivers[driver]->ids[id].interface_number == current->interface_number)) {
                    int hid = drivers[driver]->open(current);
                    if (hid >= 0) {
                        hid_devices[hid].opened = 1;
                        hid_devices[hid].driver = drivers[driver];
                        if (ghid_register(hid, hid, read_callback, write_callback, close_callback, HIDINPUT_REGISTER_FUNCTION) < 0) {
                            close_device(hid);
                        }
                    }
                }
            }
        }
        if (current->next == 0) {
            break;
        }
    }
    ghid_free_enumeration(hid_devs);

    return 0;
}

int hidinput_poll() {

    int ret = 0;
    unsigned int device;
    for (device = 0; device < sizeof(hid_devices) / sizeof(*hid_devices); ++device) {
        if (hid_devices[device].opened != 0) {
            if (hid_devices[device].read_pending == 0) {
                if (ghid_poll(device) < 0) {
                    ret = -1;
                } else {
                    hid_devices[device].read_pending = 1;
                }
            }
        }
    }
    return ret;
}

void hidinput_quit() {

    unsigned int device;
    for (device = 0; device < sizeof(hid_devices) / sizeof(*hid_devices); ++device) {
        close_device(device);
    }
}

int hidinput_set_callbacks(int device, int user, int (* write_cb)(int user, int transfered), int (* close_cb)(int user)) {

    HIDINPUT_CHECK_DEVICE(device, -1)

    hid_devices[device].callbacks.user = user;
    hid_devices[device].callbacks.write = write_cb;
    hid_devices[device].callbacks.close = close_cb;

    return 0;
}
