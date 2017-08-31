/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimx.h>
#include <gimxhid/include/ghid.h>
#include <gimxcontroller/include/controller.h>
#include <gimxcommon/include/glist.h>
#include <gimxcommon/include/gerror.h>
#include <string.h>
#include <stdlib.h>
#include <haptic/haptic_core.h>

#include <haptic/haptic_sink.h>
#include <haptic/haptic_source.h>
#include <haptic/haptic_tweaks.h>

struct haptic_core {
    struct {
        s_haptic_core_ids ids;
        const s_haptic_source * ptr;
        struct haptic_source_state * state;
    } source;
    struct {
        int joystick;
        const s_haptic_sink * ptr;
        struct haptic_sink_state * state;
    } sink;
    s_haptic_core_tweaks tweaks;
    GLIST_LINK(struct haptic_core)
};

GLIST_INST(struct haptic_core, ff_cores, haptic_core_clean)

struct haptic_core * haptic_core_init(s_haptic_core_ids source_ids, int sink_joystick) {

    struct haptic_core * core = calloc(1, sizeof(*core));
    if (core == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc")
        return NULL;
    }

    core->source.ptr = haptic_source_get(source_ids);
    core->sink.ptr = haptic_sink_get(core->sink.joystick);

    if (core->source.ptr == NULL || core->sink.ptr == NULL) {
        free(core);
        return NULL;
    }

    core->source.state = core->source.ptr->init(source_ids);
    core->sink.state = core->sink.ptr->init(sink_joystick);

    GLIST_ADD(ff_cores, core)

    return core;
}

int haptic_core_clean(struct haptic_core * core) {

    if (core->source.ptr != NULL) {
        core->source.ptr->clean(core->source.state);
    }
    if (core->sink.ptr != NULL) {
        core->sink.ptr->clean(core->sink.state);
    }

    GLIST_REMOVE(ff_cores, core)

    free(core);

    return 0;
}

void haptic_core_process_report(struct haptic_core * core, size_t size, const unsigned char * data) {

    if (core->source.ptr != NULL) {
        core->source.ptr->process(core->source.state, size, data);
    }
}

void haptic_core_update(struct haptic_core * core) {

    if (core->source.ptr == NULL
            || core->sink.ptr == NULL) {
        return;
    }

    s_haptic_core_data data;
    while (core->source.ptr->get(core->source.state, &data)) {
        haptic_tweak_apply(&core->tweaks, &data);
        core->sink.ptr->process(core->sink.state, &data);
    }

    core->sink.ptr->update(core->sink.state);
}

void haptic_core_set_tweaks(struct haptic_core * core, int invert) {

    ginfo("FFB invert: %s\n", invert ? "yes" : "no");

    core->tweaks.invert = invert;
}
