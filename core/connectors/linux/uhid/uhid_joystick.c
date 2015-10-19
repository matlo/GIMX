/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <uhid_joystick.h>
#include <hidasync.h>
#include <uhidasync.h>
#include <stdio.h>

#define UHID_JOYSTICK_MAX_DEVICES 256

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);

static struct {
    int hid;
    int uhid;

} uhid_joystick_devices[UHID_JOYSTICK_MAX_DEVICES] = { };

void uhid_joystick_init(void) __attribute__((constructor (101)));
void uhid_joystick_init(void) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        uhid_joystick_devices[i].hid = -1;
        uhid_joystick_devices[i].uhid = -1;
    }
}

static void uhid_joystick_close(int id) {

    if (uhid_joystick_devices[id].hid >= 0) {
        hidasync_close(uhid_joystick_devices[id].hid);
        uhid_joystick_devices[id].hid = -1;
    }
    if (uhid_joystick_devices[id].uhid >= 0) {
        uhidasync_close(uhid_joystick_devices[id].uhid);
        uhid_joystick_devices[id].uhid = -1;
    }
}

void uhid_joystick_clean(void) __attribute__((destructor (101)));
void uhid_joystick_clean(void) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        uhid_joystick_close(i);
    }
}

inline int uhid_joystick_check_device(int device, const char * file, unsigned int line, const char * func) {
    if (device < 0 || device >= UHID_JOYSTICK_MAX_DEVICES) {
        PRINT_ERROR_OTHER("invalid device")
        return -1;
    }
    if (uhid_joystick_devices[device].hid < 0) {
        PRINT_ERROR_OTHER("no such device")
        return -1;
    }
    return 0;
}
#define UHID_JOYSTICK_CHECK_DEVICE(device,retValue) \
  if(uhid_joystick_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static int add_device(int hid, int uhid) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if (uhid_joystick_devices[i].hid == -1
            && uhid_joystick_devices[i].uhid == -1) {
            uhid_joystick_devices[i].hid = hid;
            uhid_joystick_devices[i].uhid = uhid;
            return i;
        }
    }
    return -1;
}

static unsigned short lg_wheel_products[] = {
    USB_DEVICE_ID_LOGITECH_WINGMAN_FFG,
    USB_DEVICE_ID_LOGITECH_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFP_WHEEL,
    USB_DEVICE_ID_LOGITECH_G25_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFGT_WHEEL,
    USB_DEVICE_ID_LOGITECH_G27_WHEEL,
    USB_DEVICE_ID_LOGITECH_WII_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL2,
    USB_DEVICE_ID_LOGITECH_VIBRATION_WHEEL,
};

static int is_logitech_wheel(unsigned short vendor, unsigned short product) {

    if(vendor != USB_VENDOR_ID_LOGITECH) {
        return 0;
    }
    unsigned int i;
    for(i = 0; i < sizeof(lg_wheel_products) / sizeof(*lg_wheel_products); ++i) {
        if(lg_wheel_products[i] == product) {
            return 1;
        }
    }
    return 0;
}

int uhid_joystick_open_all() {

    s_hid_dev * hid_devs = hidasync_enumerate(0x0000, 0x0000);

    s_hid_dev * current;
    for(current = hid_devs; current != NULL; ++current) {

        if(is_logitech_wheel(current->vendor_id, current->product_id)) {

            int hid = hidasync_open_path(current->path);
            if(hid >= 0) {

                const s_hid_info * hid_info = hidasync_get_hid_info(hid);
                int uhid = uhidasync_create(hid_info);
                if(uhid >= 0) {

                    if(add_device(hid, uhid) < 0) {

                        PRINT_ERROR_OTHER("cannot add device")
                        hidasync_close(hid);
                        uhidasync_close(uhid);
                        continue;
                    }
                }
            }
        }
        if(current->next == 0) {
            break;
        }
    }

    hidasync_free_enumeration(hid_devs);

    return 0;
}

int uhid_joystick_get_hid_id(int uhid_id) {

    if(uhid_id < 0) {

        PRINT_ERROR_OTHER("uhid_id is invalid")
        return -1;
    }

    int i;
    for(i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if(uhid_joystick_devices[i].uhid == uhid_id) {
            return uhid_joystick_devices[i].hid;
        }
    }

    return -1;
}

int uhid_joystick_close_unused() {

    int i;
    for(i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if(uhid_joystick_devices[i].uhid < 0
            && uhid_joystick_devices[i].hid < 0) {
            continue;
        }
        if(uhidasync_is_opened(i) == 0) {
            uhid_joystick_close(i);
        }
    }

    return 0;
}

int uhid_joystick_close_all() {

    uhid_joystick_clean();

    return 0;
}
