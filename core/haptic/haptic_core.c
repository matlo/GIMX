/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimx.h>
#include <gimxhid/include/ghid.h>
#include <gimxcontroller/include/controller.h>
#include <gimxcommon/include/glist.h>
#include <string.h>
#include <stdlib.h>
#include <haptic/haptic_core.h>

#include <haptic/haptic_sink.h>
#include <haptic/haptic_source.h>
#include <haptic/haptic_tweaks.h>

struct haptic_core {
    struct {
        const s_haptic_source * ptr;
        struct haptic_source_state * state;
    } source;
    struct {
        const s_haptic_sink * ptr;
        struct haptic_sink_state * state;
    } sink;
    s_haptic_core_tweaks tweaks;
    GLIST_LINK(struct haptic_core);
};

static GLIST_INST(struct haptic_core, ff_cores);
GLIST_DESTRUCTOR(ff_cores, haptic_core_clean)

struct haptic_core * haptic_core_init(s_haptic_core_ids source_ids, int sink_joystick) {

    struct haptic_core * core = calloc(1, sizeof(*core));
    if (core == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc");
        return NULL;
    }

    core->source.ptr = haptic_source_get(source_ids);
    if (core->source.ptr != NULL) {
        ginfo("Haptic core has source %s for device %04x:%04x\n", core->source.ptr->name, source_ids.vid, source_ids.pid);
    } else {
        ginfo("No haptic source found for device %04x:%04x\n", source_ids.vid, source_ids.pid);
    }

    core->sink.ptr = haptic_sink_get(sink_joystick);
    if (core->sink.ptr != NULL) {
        ginfo("Haptic core has sink %s for joystick %d (%s)\n", core->sink.ptr->name, sink_joystick, ginput_joystick_name(sink_joystick));
    } else {
        ginfo("No haptic sink found for joystick %d (%s)\n", sink_joystick, ginput_joystick_name(sink_joystick));
    }

    if (core->source.ptr == NULL || core->sink.ptr == NULL) {
        free(core);
        return NULL;
    }

    core->source.state = core->source.ptr->init(source_ids);
    core->sink.state = core->sink.ptr->init(sink_joystick);

    core->tweaks.invert = 0;
    core->tweaks.gain.rumble = 100;
    core->tweaks.gain.constant = 100;
    core->tweaks.gain.spring = 100;
    core->tweaks.gain.damper = 100;

    GLIST_ADD(ff_cores, core);

    return core;
}

int haptic_core_clean(struct haptic_core * core) {

    if (core == NULL) {
        return 0;
    }

    if (core->source.ptr != NULL) {
        core->source.ptr->clean(core->source.state);
    }
    if (core->sink.ptr != NULL) {
        core->sink.ptr->clean(core->sink.state);
    }

    GLIST_REMOVE(ff_cores, core);

    free(core);

    return 0;
}

void haptic_core_process_report(struct haptic_core * core, size_t size, const unsigned char * data) {

    if (core != NULL && core->source.ptr != NULL) {
        core->source.ptr->process(core->source.state, size, data);
    }
}

void haptic_core_update(struct haptic_core * core) {

    if (core == NULL || core->source.ptr == NULL || core->sink.ptr == NULL) {
        return;
    }

    s_haptic_core_data data;
    while (core->source.ptr->get(core->source.state, &data)) {
        haptic_tweak_apply(&core->tweaks, &data);
        core->sink.ptr->process(core->sink.state, &data);
    }

    core->sink.ptr->update(core->sink.state);
}

void haptic_core_set_tweaks(struct haptic_core * core, const s_haptic_core_tweaks * tweaks) {

    if (core == NULL) {
        return;
    }

    if (gimx_params.status) {
        if (core->sink.ptr->caps != E_HAPTIC_SINK_CAP_NONE) {
            ginfo("FFB invert: %s\n", tweaks->invert ? "yes" : "no");
            ginfo("FFB gain:");
        }
        if (core->sink.ptr->caps & E_HAPTIC_SINK_CAP_RUMBLE) {
            ginfo(" rumble=%d", tweaks->gain.rumble);
        }
        if (core->sink.ptr->caps & E_HAPTIC_SINK_CAP_CONSTANT) {
            ginfo(" constant=%d", tweaks->gain.constant);
        }
        if (core->sink.ptr->caps & E_HAPTIC_SINK_CAP_SPRING) {
            ginfo(" spring=%d", tweaks->gain.spring);
        }
        if (core->sink.ptr->caps & E_HAPTIC_SINK_CAP_DAMPER) {
            ginfo(" damper=%d", tweaks->gain.damper);
        }
        if (core->sink.ptr->caps != E_HAPTIC_SINK_CAP_NONE) {
            ginfo("\n");
        }
    }

    core->tweaks = *tweaks;
}
