/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#ifndef WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <common/gerror.h>
#include <stdint.h>
#include <string.h>

#define STEAM_CONTROLLER_VID            0x28de
#define WIRELESS_STEAM_CONTROLLER_PID   0x1142
#define WIRED_STEAM_CONTROLLER_PID      0x1102

#define STEAM_CONTROLLER_NAME "Valve Software Steam Controller"

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

#define INVERT(VALUE) (VALUE == SHRT_MIN ? SHRT_MAX : -VALUE)

typedef struct PACKED {
    uint64_t : 16;
    uint16_t status;
    uint64_t : 32;
    uint8_t buttons[3];
    uint8_t left_trigger;
    uint8_t right_trigger;
    uint64_t : 24;
    int16_t left_x;
    int16_t left_y;
    int16_t right_x;
    int16_t right_y;
    uint64_t : 64;
    uint64_t : 64;
    uint64_t : 64;
    uint64_t : 64;
    uint64_t : 64;
} s_report;

#define HID_REPORT_SIZE sizeof(s_report)

static struct {
    int opened;
    int joystick;
    s_report previous;
} hid_devices[HIDINPUT_MAX_DEVICES];

static s_hidinput_ids ids[] = {
        // check wired controllers first
        { .vendor_id = STEAM_CONTROLLER_VID, .product_id = WIRED_STEAM_CONTROLLER_PID,    .interface_number = -1 },
        { .vendor_id = STEAM_CONTROLLER_VID, .product_id = WIRELESS_STEAM_CONTROLLER_PID, .interface_number =  1 },
        { .vendor_id = STEAM_CONTROLLER_VID, .product_id = WIRELESS_STEAM_CONTROLLER_PID, .interface_number =  2 },
        { .vendor_id = STEAM_CONTROLLER_VID, .product_id = WIRELESS_STEAM_CONTROLLER_PID, .interface_number =  3 },
        { .vendor_id = STEAM_CONTROLLER_VID, .product_id = WIRELESS_STEAM_CONTROLLER_PID, .interface_number =  4 },
        { .vendor_id = 0, .product_id = 0 },
};

static int (*event_callback)(GE_Event*) = NULL;

static void clear_device(int device) {

  memset(hid_devices + device, 0x00, sizeof(*hid_devices));
  hid_devices[device].joystick = -1;
}

static int close_device(int device) {

    if (hid_devices[device].opened != 0) {
        ghid_close(device);
    }

    if (hid_devices[device].joystick >= 0) {
        // TODO MLA: remove joystick
    }

    clear_device(device);

    return 0;
}

static int init(int(*callback)(GE_Event*)) {

    event_callback = callback;

    return 0;
}

static int process(int device, const void * report, unsigned int size) {

    if (size != HID_REPORT_SIZE) {
        return -1;
    }

    const s_report * current = report;
    const s_report * previous = (s_report *)&hid_devices[device].previous;

    if (current->status != htons(0x013c)) {
        return -1;
    }

    GE_Event button = { .jbutton = { .which = hid_devices[device].joystick } };

    uint8_t inhibit[3] = {};

    unsigned int i;
    for (i = 0; i < sizeof(current->buttons) / sizeof(*current->buttons); ++i) {
        uint8_t mask;
        for (mask = 0x80; mask != 0; mask >>= 1) {
            uint8_t value;
            if ((value = (current->buttons[i] & mask)) ^ (previous->buttons[i] & mask)) {
                if (i == 2 && mask == 0x40) {
                    inhibit[2] |= 0x02;
                }
            }
        }
    }

    for (i = 0; i < sizeof(current->buttons) / sizeof(*current->buttons); ++i) {
        uint8_t mask;
        for (mask = 0x80; mask != 0; mask >>= 1) {
            uint8_t value;
            if ((value = (current->buttons[i] & mask)) ^ (previous->buttons[i] & mask)) {
                if ((inhibit[i] & mask) == 0) {
                    button.jbutton.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
                    event_callback(&button);
                }
            }
            ++button.jbutton.button;
        }
    }

    GE_Event axis = { .jaxis = { .type = GE_JOYAXISMOTION, .which = hid_devices[device].joystick } };

    // triggers

    if (current->left_trigger != previous->left_trigger) {
        axis.jaxis.value = (int16_t)current->left_trigger * 32767 / 255;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if (current->right_trigger != previous->right_trigger) {
        axis.jaxis.value = (int16_t)current->right_trigger * 32767 / 255;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    uint8_t left_active_current = current->buttons[2] & 0x08;
    uint8_t left_active_previous = previous->buttons[2] & 0x08;

    // left pad (active when pad is touched)

    if (left_active_current || left_active_previous) {
        if (current->left_x != previous->left_x) {
            axis.jaxis.value = current->left_x;
            event_callback(&axis);
        }
    }

    ++axis.jaxis.axis;

    if (left_active_current || left_active_previous) {
        if (current->left_y != previous->left_y) {
            axis.jaxis.value = INVERT(current->left_y);
            event_callback(&axis);
        }
    }

    ++axis.jaxis.axis;

    // right pad

    if (current->right_x != previous->right_x) {
        axis.jaxis.value = current->right_x;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if (current->right_y != previous->right_y) {
        axis.jaxis.value = INVERT(current->right_y);
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    // stick (active when pad is not touched)

    if (!left_active_current) {
        if (current->left_x != previous->left_x) {
            axis.jaxis.value = current->left_x;
            event_callback(&axis);
        }
    } else if(!left_active_previous) {
        axis.jaxis.value = 0;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if (!left_active_current) {
        if (current->left_y != previous->left_y) {
            axis.jaxis.value = INVERT(current->left_y);
            event_callback(&axis);
        }
    } else if(!left_active_previous) {
        axis.jaxis.value = 0;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    hid_devices[device].previous = *current;

    return 0;
}

static int open(const struct ghid_device * dev) {

    int device = ghid_open_path(dev->path);
    if (device < 0) {
        return -1;
    }

    if (hid_devices[device].opened != 0) {
        ghid_close(device);
        return -1;
    }

    hid_devices[device].opened = 1;

    hid_devices[device].joystick = ginput_register_joystick(STEAM_CONTROLLER_NAME, GE_HAPTIC_NONE, NULL);
    if (hid_devices[device].joystick < 0) {
        close_device(device);
        return -1;
    }

    return device;
}

static s_hidinput_driver driver = {
        .ids = ids,
        .init = init,
        .open = open,
        .process = process,
        .close = close_device,
};

void steamcontroller_constructor(void) __attribute__((constructor));
void steamcontroller_constructor(void) {
    unsigned int device;
    for (device = 0; device < sizeof(hid_devices) / sizeof(*hid_devices); ++device) {
        clear_device(device);
    }
    if (hidinput_register(&driver) < 0) {
        exit(-1);
    }
}
