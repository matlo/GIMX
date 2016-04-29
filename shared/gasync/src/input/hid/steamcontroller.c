/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#ifndef WIN32
#include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
#endif
#include <stdlib.h>
#include <gerror.h>
#include <stdint.h>
#include <string.h>

#define STEAM_CONTROLLER_VID            0x28de
#define WIRELESS_STEAM_CONTROLLER_PID   0x1142
#define WIRED_STEAM_CONTROLLER_PID      0x1042

#define WIRELESS_INTERFACE_NUMBER 4

#define HID_REPORT_SIZE 64

static s_hidinput_ids ids[] = {
        // check wired controllers first
        { .vendor = STEAM_CONTROLLER_VID, .product = WIRED_STEAM_CONTROLLER_PID, .name = "Steam Controller" },
        { .vendor = STEAM_CONTROLLER_VID, .product = WIRELESS_STEAM_CONTROLLER_PID, .name = "Steam Controller" },
        { .vendor = 0, .product = 0 },
};

static int (*event_callback)(GE_Event*) = NULL;

static int init(int(*callback)(GE_Event*)) {

    event_callback = callback;

    return 0;
}

/*
 * Each USB dongle results in 5 consecutive interfaces:
 * - the 1st one is a boot keyboard and mouse
 * - each other interface is a wireless controller.
 */
static int probe(int hid) {

#ifdef WIN32
    return 0; // on Windows the HID API does not provide access to mice and keyboards
#else
    static unsigned char count = 0;

    const s_hid_info * hid_info = ghid_get_hid_info(hid);

    if (hid_info == NULL) {
        return -1;
    }

    if (hid_info->product_id == WIRED_STEAM_CONTROLLER_PID) {
        count = 0;
        return 0;
    }

    ++count;

    if (count == 1) {
        return -1; // skip 1st interface
    }

    if (count > WIRELESS_INTERFACE_NUMBER) {
        count = 0;
    }

    return 0;
#endif
}

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

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

static int process(int joystick, const void * report, unsigned int size, const void * prev) {

    if (size != HID_REPORT_SIZE) {
        return -1;
    }

    const s_report * current = report;
    const s_report * previous = prev;

    if (current->status != htons(0x013c)) {
        return -1;
    }

    if (previous->status == 0x0000) {
        // skip first report so as to allow detecting pad buttons
        return 0;
    }

    GE_Event button = { .jbutton = { .which = joystick } };

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

    GE_Event axis = { .jaxis = { .type = GE_JOYAXISMOTION, .which = joystick } };

    // triggers

    if (current->left_trigger != previous->left_trigger) {
        axis.jaxis.value = current->left_trigger;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if (current->right_trigger != previous->right_trigger) {
        axis.jaxis.value = current->right_trigger;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    // left pad (active when pad is touched)

    if ((current->buttons[2] & 0x08)) { // pad touched
        if (current->left_x != previous->left_x) {
            axis.jaxis.value = current->left_x;
            event_callback(&axis);
        }
    }

    ++axis.jaxis.axis;

    if ((current->buttons[2] & 0x08)) { // pad touched
        if (current->left_y != previous->left_y) {
            axis.jaxis.value = current->left_y;
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
        axis.jaxis.value = current->right_y;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    // stick (active when pad is not touched)

    if (!(current->buttons[2] & 0x08)) {
        if (current->left_x != previous->left_x) {
            axis.jaxis.value = current->left_x;
            event_callback(&axis);
        }
    }

    ++axis.jaxis.axis;

    if (!(current->buttons[2] & 0x08)) {
        if (current->left_y != previous->left_y) {
            axis.jaxis.value = current->left_y;
            event_callback(&axis);
        }
    }

    ++axis.jaxis.axis;

    return 0;
}

static s_hidinput_driver driver = {
        .ids = ids,
        .init = init,
        .probe = probe,
        .process = process,
};

void steamcontroller_constructor(void) __attribute__((constructor));
void steamcontroller_constructor(void) {
    if (hidinput_register(&driver) < 0) {
        exit(-1);
    }
}
