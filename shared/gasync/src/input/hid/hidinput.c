/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#include <ghid.h>
#include <gpoll.h>
#include <gerror.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static s_hidinput_driver ** drivers = NULL;
static unsigned int nb_drivers = 0;

#define MAX_DEVICES 8
#define MAX_REPORT_SIZE 64

static struct {
    int hid;
    int joystick;
    s_hidinput_driver * driver;
    unsigned char prev[MAX_REPORT_SIZE];
} hid_devices[MAX_DEVICES] = {};

static void init_device(int device) {

  memset(hid_devices + device, 0x00, sizeof(*hid_devices));
  hid_devices[device].hid = -1;
  hid_devices[device].joystick = -1;
}

void hidinput_constructor(void) __attribute__((constructor));
void hidinput_constructor(void) {

  unsigned int i;
  for (i = 0; i < sizeof(hid_devices) / sizeof(*hid_devices); ++i) {
      init_device(i);
  }
}

void hidinput_destructor(void) __attribute__((destructor));
void hidinput_destructor(void) {

    hidinput_quit();
    free(drivers);
    nb_drivers = 0;
}

int hidinput_register(s_hidinput_driver * driver) {

    void * ptr = realloc(drivers, nb_drivers * sizeof(*drivers));
    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc")
        return -1;
    }
    drivers = ptr;
    drivers[nb_drivers] = driver;
    ++nb_drivers;
    return 0;
}

static int close_device(int device) {

    if (hid_devices[device].hid >= 0) {
        ghid_close(hid_devices[device].hid);
    }

    if (hid_devices[device].joystick >= 0) {
        // TODO MLA: remove joystick
    }

    init_device(device);

    return 0;
}

static int read_callback(int device, const void * buf, int status) {

    if (status > 0) {
        int ret = hid_devices[device].driver->process(hid_devices[device].joystick, buf, status, hid_devices[device].prev);
        if (ret == 0) {
            memcpy(hid_devices[device].prev, buf, status);
        }
    }

    ghid_poll(hid_devices[device].hid);

    //TODO MLA: handle errors (status < 0)

    return 0;
}

static int write_callback(int device, int status) {

    return 0;
}

static int close_callback(int device) {

    close_device(device);

    return 0;
}

static int add_device(int device, int joystick, int driver) {

    unsigned int i;
    for (i = 0; i < sizeof(hid_devices) / sizeof(*hid_devices); ++i) {
        if (hid_devices[i].hid < 0) {
            hid_devices[i].hid = device;
            hid_devices[i].driver = drivers[driver];
            hid_devices[i].joystick = joystick;
            return i;
        }
    }
    PRINT_ERROR_OTHER("no slot available")
    return -1;
}

int hidinput_init(int(*callback)(GE_Event*)) {

    if (callback == NULL) {
      PRINT_ERROR_OTHER("callback is NULL")
      return -1;
    }

    unsigned int dev_index;
    for (dev_index = 0; dev_index < sizeof(hid_devices) / sizeof(*hid_devices); ++dev_index) {
        hid_devices[dev_index].hid = -1;
    }

    unsigned int driver;
    for (driver = 0; driver < nb_drivers; ++driver) {
        drivers[driver]->init(callback);
        unsigned int id;
        for (id = 0; drivers[driver]->ids[id].vendor != 0; ++id) {
            s_hid_dev * hid_devs = ghid_enumerate(drivers[driver]->ids[id].vendor, drivers[driver]->ids[id].product);
            s_hid_dev * current;
            for (current = hid_devs; current != NULL; ++current) {
                int hid = ghid_open_path(current->path);
                if (hid < 0) {
                    return -1;
                }
                int joystick = ginput_register_joystick(drivers[driver]->ids[id].name, NULL);
                if (joystick < 0) {
                    ghid_close(hid);
                    return -1;
                }
                int device = add_device(hid, joystick, driver);
                if (device < 0) {
                    ghid_close(hid);
                    // TODO MLA: remove joystick
                    return -1;
                }
                if (ghid_register(hid, device, read_callback, write_callback, close_callback, REGISTER_FUNCTION) < 0) {
                    close_device(device);
                    return -1;
                }
#ifndef WIN32
                if (ghid_poll(hid) < 0) {
                    close_device(device);
                    return -1;
                }
#endif
                if (current->next == 0) {
                    break;
                }
            }
            ghid_free_enumeration(hid_devs);
        }
    }
    return 0;
}

void hidinput_quit() {

    unsigned int i;
    for (i = 0; i < sizeof(hid_devices) / sizeof(*hid_devices); ++i) {
        if(hid_devices[i].hid >= 0) {
            close_device(i);
        }
    }
}
