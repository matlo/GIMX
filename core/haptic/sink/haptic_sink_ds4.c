/*
 Copyright (c) 2020 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <gimxhid/include/ghid.h>
#include <controller.h>
#include <gimx.h>
#include <limits.h>
#include <haptic/haptic_common.h>
#include <haptic/haptic_sink.h>

#define DS4_VENDOR 0x054c
#define DS4_PRODUCT 0x05c4
#define DS4_PRODUCT_2 0x09cc
#define DS4_OUTPUT_REPORT_SIZE 32

struct haptic_sink_state {
    struct ghid_device *hid;
    int write_pending;
    int updated;
    uint8_t report[DS4_OUTPUT_REPORT_SIZE];
};

static inline void send_report(struct haptic_sink_state *state) {

    int res = ghid_write(state->hid, state->report, sizeof(state->report));
    if (res == 0) {
        state->write_pending = 1;
    } else if (res > 0) {
        // not sure this may happen
        // do not call haptic_sink_ds4_update
    }
}

void haptic_sink_ds4_update(struct haptic_sink_state *state) {

    if (state->write_pending != 0) {
        return;
    }

    if (state->updated) {
        send_report(state);
        state->updated = 0;
    }
}

static int hid_write_cb(void *user, int status) {

    struct haptic_sink_state *state = (struct haptic_sink_state*) user;

    state->write_pending = 0;

    haptic_sink_ds4_update(state);

    return (status < 0) ? -1 : 0;
}

static int hid_close_cb(void *user) {

    struct haptic_sink_state *state = (struct haptic_sink_state*) user;

    ghid_close(state->hid);
    state->hid = NULL;

    return 1;
}

static struct haptic_sink_state* haptic_sink_ds4_init(int joystick) {


    struct ghid_device * hid;

#ifndef WIN32
    hid = ginput_joystick_get_hid(joystick);
    if (hid == NULL) {
        return NULL;
    }
#else
    s_haptic_core_ids ids = { 0x0000, 0x0000 };
    if (ginput_joystick_get_usb_ids(joystick, &ids.vid, &ids.pid) == -1) {
        return NULL;
    }
    hid = ghid_open_ids(ids.vid, ids.pid);
    if (hid == NULL) {
        return NULL;
    }
#endif

    void *ptr = calloc(1, sizeof(struct haptic_sink_state));

    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc");
        ghid_close(hid);
        return NULL;
    }

    struct haptic_sink_state *state = (struct haptic_sink_state*) ptr;

    state->hid = hid;

    GHID_CALLBACKS ghid_callbacks = {
            .fp_read = NULL,
            .fp_write = hid_write_cb,
            .fp_close = hid_close_cb,
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if (ghid_register(hid, state, &ghid_callbacks) < 0) {
        ghid_close(hid);
        free(ptr);
        return NULL;
    }

    state->report[0] = 0x05;
    state->report[1] = 0x07;

    return state;
}

static void haptic_sink_ds4_clean(struct haptic_sink_state *state) {

    if (state->report[4] || state->report[5]) {
        // stop rumble
        state->report[4] = 0;
        state->report[5] = 0;
        ghid_write_timeout(state->hid, state->report, sizeof(state->report), 1000);
    }

    ghid_close(state->hid);

    free(state);
}

static void haptic_sink_ds4_process(struct haptic_sink_state *state, const s_haptic_core_data *data) {

    if (data->type == E_DATA_TYPE_RUMBLE) {
        uint8_t weak = data->rumble.weak * UCHAR_MAX / USHRT_MAX;
        uint8_t strong = data->rumble.strong * UCHAR_MAX / USHRT_MAX;
        if (state->report[4] != weak || state->report[5] != strong) {
            state->report[4] = weak;
            state->report[5] = strong;
            state->updated = 1;
            dprintf("< RUMBLE, weak=%hu, strong=%hu\n", weak, strong);
        }
    }
}

static s_haptic_core_ids haptic_sink_ds4_ids[] = {
#ifdef WIN32
        { .vid = DS4_VENDOR, .pid = DS4_PRODUCT   },
        { .vid = DS4_VENDOR, .pid = DS4_PRODUCT_2 },
#endif
        { .vid = 0x0000,     .pid = 0x0000        }, // end of table
};

static s_haptic_sink sink_ds4 = {
        .name = "haptic_sink_ds4",
        .ids = haptic_sink_ds4_ids,
        .caps = E_HAPTIC_SINK_CAP_RUMBLE,
        .init = haptic_sink_ds4_init,
        .clean = haptic_sink_ds4_clean,
        .process = haptic_sink_ds4_process,
        .update = haptic_sink_ds4_update
};

void haptic_sink_ds4_constructor(void) __attribute__((constructor));
void haptic_sink_ds4_constructor(void) {

    haptic_sink_register(&sink_ds4);
}
