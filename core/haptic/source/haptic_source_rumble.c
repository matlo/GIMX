/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdint.h>
#include <stdlib.h>
#include <gimxcontroller/include/controller.h>
#include <haptic/haptic_source.h>
#include <limits.h>

static struct {
    struct {
        uint16_t vid;
        uint16_t pid;
    } ids;
    struct {
        unsigned char offset;
        unsigned char max;
    } weak;
    struct {
        unsigned char offset;
        unsigned char max;
    } strong;
} props[] = {
        { .ids = { .vid = DS4_VENDOR,  .pid = DS4_PRODUCT  }, .weak = { 4, 0xff }, .strong = { 5, 0xff } },
        { .ids = { .vid = X360_VENDOR, .pid = X360_PRODUCT }, .weak = { 4, 0xff }, .strong = { 3, 0xff } },
        { .ids = { .vid = DS3_VENDOR,  .pid = DS3_PRODUCT  }, .weak = { 3, 0x01 }, .strong = { 5, 0xff } },
        { .ids = { .vid = XONE_VENDOR, .pid = XONE_PRODUCT }, .weak = { 7, 0xff }, .strong = { 6, 0xff } },
        { .ids = { .vid = 0x2508,      .pid = 0x0001       }, .weak = { 7, 0xff }, .strong = { 6, 0xff } }, // GPP/Cronus/Titan
};

struct haptic_source_state
{
    unsigned int props_index;
    int updated;
    uint16_t weak;
    uint16_t strong;
};

static struct haptic_source_state * haptic_source_rumble_init(s_haptic_core_ids ids) {

    unsigned int i;
    for (i = 0; i < sizeof(props) / sizeof(*props); ++i) {
        if (props[i].ids.vid == ids.vid && props[i].ids.pid == ids.pid) {
            void * ptr = calloc(1, sizeof(struct haptic_source_state));
            if (ptr != NULL) {
                struct haptic_source_state * state = (struct haptic_source_state *) ptr;
                state->props_index = i;
                return state;
            } else {
                PRINT_ERROR_ALLOC_FAILED("calloc")
                return NULL;
            }
        }
    }
    return NULL;
}

static void haptic_source_rumble_clean(struct haptic_source_state * state) {

    free(state);
}

static void haptic_source_rumble_process(struct haptic_source_state * state, size_t size __attribute((unused)), const unsigned char * data) {

    uint16_t weak = data[props[state->props_index].weak.offset] * USHRT_MAX / props[state->props_index].weak.max;
    uint16_t strong = data[props[state->props_index].strong.offset] * USHRT_MAX / props[state->props_index].strong.max;

    if (weak != state->weak || strong != state->strong) {
        state->updated = 1;
        state->weak = weak;
        state->strong = strong;
        dprintf("> RUMBLE, weak=%hu, strong=%hu\n", weak, strong);
    }
}

static int haptic_source_rumble_get(struct haptic_source_state * state, s_haptic_core_data * data) {

    int ret = 0;

    if (state->updated) {
        data->type = E_DATA_TYPE_RUMBLE;
        data->rumble.weak = state->weak;
        data->rumble.strong = state->strong;
        state->updated = 0;
        ret = 1;
    }

    return ret;
}

static s_haptic_core_ids haptic_source_rumble_ids[] = {
        { .vid = DS4_VENDOR,  .pid = DS4_PRODUCT  },
        { .vid = X360_VENDOR, .pid = X360_PRODUCT },
        { .vid = DS3_VENDOR,  .pid = DS3_PRODUCT  },
        { .vid = XONE_VENDOR, .pid = XONE_PRODUCT },
        { .vid = 0x2508,      .pid = 0x0001       }, // GPP/Cronus/Titan
        { .vid = 0x0000,      .pid = 0x0000       }, // end of table
};

static s_haptic_source source_rumble = {
        .name = "haptic_source_rumble",
        .ids = haptic_source_rumble_ids,
        .init = haptic_source_rumble_init,
        .clean = haptic_source_rumble_clean,
        .process = haptic_source_rumble_process,
        .get = haptic_source_rumble_get
};

void haptic_source_rumble_constructor(void) __attribute__((constructor));
void haptic_source_rumble_constructor(void) {

    if (haptic_source_register(&source_rumble) == -1) {
        PRINT_ERROR_OTHER("failed to register source")
    }
}
