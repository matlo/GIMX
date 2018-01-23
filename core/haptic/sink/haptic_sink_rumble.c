/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdlib.h>
#include <gimxinput/include/ginput.h>
#include <haptic/haptic_common.h>
#include <haptic/haptic_sink.h>

struct haptic_sink_state
{
    int joystick;
};

static void dump_event(const GE_Event * event) {

    switch (event->type) {
    case GE_JOYRUMBLE:
        dprintf("< RUMBLE, weak=%hu, strong=%hu\n", event->jrumble.weak, event->jrumble.strong);
        fflush(stdout);
        break;
    default:
        dprintf("< UNKNOWN\n");
        fflush(stdout);
        break;
    }
}

static struct haptic_sink_state * haptic_sink_rumble_init(int joystick) {

    if (joystick >= 0) {
        int haptic = ginput_joystick_get_haptic(joystick);
        if (haptic & GE_HAPTIC_RUMBLE) {
            void * ptr = calloc(1, sizeof(struct haptic_sink_state));
            if (ptr != NULL) {
                struct haptic_sink_state * state = (struct haptic_sink_state *) ptr;
                state->joystick = joystick;
                return state;
            } else {
                PRINT_ERROR_ALLOC_FAILED("calloc")
            }
        }
    }
    return NULL;
}

static void haptic_sink_rumble_clean(struct haptic_sink_state * state) {

    free(state);
}

static void haptic_sink_rumble_process(struct haptic_sink_state * state, const s_haptic_core_data * data) {

    GE_Event event = {
        .jrumble = {
            .type = GE_JOYRUMBLE,
            .which = state->joystick,
            .weak = data->rumble.weak,
            .strong = data->rumble.strong
        }
    };
    ginput_queue_push(&event);
    if(gimx_params.debug.haptic) {
        dump_event(&event);
    }
}

static void haptic_sink_rumble_update(struct haptic_sink_state * state __attribute__((unused))) {

    // nothing to do here
}

static s_haptic_core_ids haptic_sink_rumble_ids[] = {
        /* This is a generic source, don't add anything here */
        { .vid = 0x0000,      .pid = 0x0000       }, // end of table
};

static s_haptic_sink sink_rumble = {
        .name = "haptic_sink_rumble",
        .ids = haptic_sink_rumble_ids,
        .caps = E_HAPTIC_SINK_CAP_RUMBLE,
        .init = haptic_sink_rumble_init,
        .clean = haptic_sink_rumble_clean,
        .process = haptic_sink_rumble_process,
        .update = haptic_sink_rumble_update
};

void haptic_sink_rumble_constructor(void) __attribute__((constructor));
void haptic_sink_rumble_constructor(void) {

    haptic_sink_register(&sink_rumble);
}
