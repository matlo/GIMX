/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#include <stdlib.h>
#include <gerror.h>
#include <stdint.h>

#define STEAM_CONTROLLER_VID            0x28de
#define WIRELESS_STEAM_CONTROLLER_PID   0x1142
#define WIRED_STEAM_CONTROLLER_PID      0x1042

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

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

typedef struct PACKED {
    uint64_t : 64;
    uint64_t buttons : 24;
    uint64_t left_trigger : 8;
    uint64_t right_trigger : 8;
    uint64_t : 24;
    int64_t left_x : 16;
    int64_t left_y : 16;
    int64_t right_x : 16;
    int64_t right_y : 16;
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

    GE_Event button = { .jbutton = { .which = joystick } };

    if (current->buttons) {
      printf("buttons: %03x\n", current->buttons);
    }

    uint64_t mask;
    for (mask = 0x0800; mask != 0; mask >>= 1) {
        uint64_t value;
        if ((value = (current->buttons & mask)) ^ (previous->buttons & mask)) {
            button.jbutton.type = value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
            event_callback(&button);
        }
        ++button.jbutton.button;
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

    // pads

    if ((current->buttons & 0x008) && current->left_x != previous->left_x) {
        axis.jaxis.value = current->left_x;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if ((current->buttons & 0x008) && current->left_y != previous->left_y) {
        axis.jaxis.value = current->left_y;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

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

    // stick

    if (!(current->buttons & 0x008) && current->left_x != previous->left_x) {
        axis.jaxis.value = current->left_x;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    if (!(current->buttons & 0x008) && current->left_y != previous->left_y) {
        axis.jaxis.value = current->left_y;
        event_callback(&axis);
    }

    ++axis.jaxis.axis;

    return 0;
}

static s_hidinput_driver driver = {
        .ids = ids,
        .init = init,
        .process = process,
};

void steamcontroller_constructor(void) __attribute__((constructor (101)));
void steamcontroller_constructor(void) {
    if (hidinput_register(&driver) < 0) {
        exit(-1);
    }
}
