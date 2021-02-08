/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxinput/include/ginput.h>
#include <haptic/haptic_sink.h>
#ifndef WIN32
#include <gimxhid/include/ghid.h>
#endif
#include <string.h>
#include <stdlib.h>

static s_haptic_sink ** sinks = NULL;
static unsigned int nb_sinks = 0;

void haptic_sink_destructor(void) __attribute__((destructor));
void haptic_sink_destructor(void) {

    free(sinks);
    nb_sinks = 0;
}

void haptic_sink_register(s_haptic_sink * sink) {

    void * ptr = realloc(sinks, (nb_sinks + 1) * sizeof(*sinks));
    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc");
        return;
    }
    sinks = ptr;
    sinks[nb_sinks] = sink;
    ++nb_sinks;
}

const s_haptic_sink * haptic_sink_get(int joystick) {

    unsigned int i;

    s_haptic_core_ids ids = { 0x0000, 0x0000 };

#ifndef WIN32
    struct ghid_device * hid = ginput_joystick_get_hid(joystick);
    if (hid != NULL) {
        const s_hid_info * info = ghid_get_hid_info(hid);
        ids.vid = info->vendor_id;
        ids.pid = info->product_id;
    }
#else
    ginput_joystick_get_usb_ids(joystick, &ids.vid, &ids.pid);
#endif

    // lookup specific sinks

    if (ids.vid != 0x0000) {
        for (i = 0; i < nb_sinks; ++i) {
            unsigned int j;
            for (j = 0; sinks[i]->ids[j].vid != 0; ++j) {
                if (sinks[i]->ids[j].vid == ids.vid && sinks[i]->ids[j].pid == ids.pid) {
                    return sinks[i];
                }
            }
        }
    }

    // probe generic sinks

    for (i = 0; i < nb_sinks; ++i) {
        if (sinks[i]->ids[0].vid == 0x0000 && sinks[i]->ids[0].pid == 0x0000) {
            struct haptic_sink_state * state = sinks[i]->init(joystick);
            if (state != NULL) {
                sinks[i]->clean(state);
                return sinks[i];
            }
        }
    }
    return NULL;
}
