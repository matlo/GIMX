/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <haptic/common/ff_lg.h>
#include <gimxhid/include/ghid.h>
#include <controller.h>
#include <gimx.h>
#include <limits.h>
#include <gimxcommon/include/gerror.h>
#include <haptic/haptic_common.h>
#include <haptic/haptic_sink.h>

struct haptic_sink_state {
    struct ghid_device * hid;
    int write_pending;
    unsigned short pid;
    uint8_t caps;
    struct {
        s_haptic_core_data data;
        int updated; // checked before removing the slot from the fifo
    } slots[slot_nb];
    s_haptic_sink_fifo fifo;
    s_ff_lg_report last_report;
};

static inline void clear_report(struct haptic_sink_state * state) {

  memset(state->last_report.data, 0x00, sizeof(state->last_report.data));
}

static void haptic_sink_lg_ack(struct haptic_sink_state * state) {

    dprintf("> ack\n");

    uint8_t * data = state->last_report.data + 1;

    if (data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

        uint8_t slots = data[0] & FF_LG_FSLOT_MASK;
        uint8_t cmd = data[0] & FF_LG_CMD_MASK;

        int i;

        switch (cmd) {
        case FF_LG_CMD_DOWNLOAD_AND_PLAY:
        case FF_LG_CMD_STOP:
            for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
                e_slot slot = (1 << i);
                if (slots & (slot << 4)) {
                    if (!state->slots[slot].updated) {
                        haptic_sink_fifo_remove(&state->fifo, slot);
                    } else {
                        dprintf("do not remove %d\n", slot);
                    }
                }
            }
            break;
        }

    } else {

        e_slot slot = slot_nb;

        switch (data[1]) {
        case FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES:
        case FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES:
        case FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE:
            slot = slot_range;
            break;
        case FF_LG_EXT_CMD_SET_RPM_LEDS:
            slot = slot_leds;
            break;
        }

        if (slot != slot_nb) {
            if (!state->slots[slot].updated) {
                haptic_sink_fifo_remove(&state->fifo, slot);
            } else {
                dprintf("do not remove %d\n", slot);
            }
        }
    }
}

static inline void send_report(struct haptic_sink_state * state) {

    int res = ghid_write(state->hid, state->last_report.data, sizeof(state->last_report.data));
    if (res == 0) {
        state->write_pending = 1;
    } else if (res > 0) {
        haptic_sink_lg_ack(state);
        // not sure this may happen
        // do not call haptic_sink_lg_update
    }
}

static uint8_t fslot_nbits [] = {
    [0b0000] = 0,
    [0b0001] = 1,
    [0b0010] = 1,
    [0b0011] = 2,
    [0b0100] = 1,
    [0b0101] = 2,
    [0b0110] = 2,
    [0b0111] = 3,
    [0b1000] = 1,
    [0b1001] = 2,
    [0b1010] = 2,
    [0b1011] = 3,
    [0b1100] = 2,
    [0b1101] = 3,
    [0b1110] = 3,
    [0b1111] = 4
};

void haptic_sink_lg_update(struct haptic_sink_state * state) {

    if (state->write_pending != 0) {
        return;
    }

    unsigned int i;
    uint8_t mask = 0;
    uint8_t * data = state->last_report.data + 1;

    // check if there are multiple forces to stop

    for (i = 0; i < state->fifo.size; ++i) {
        e_slot slot = state->fifo.items[i];
        if (!state->slots[slot].data.playing) {
            mask |= (1 << slot);
        }
    }

    if (fslot_nbits[mask] > 1) {
        clear_report(state);
        data[0] = (mask << 4) | FF_LG_CMD_STOP;
        dprintf("< %s %02x\n", ff_lg_get_cmd_name(data[0]), data[0]);
        for (i = 0; i < state->fifo.size; ++i) {
            e_slot slot = state->fifo.items[i];
            if (!state->slots[slot].data.playing) {
                state->slots[slot].updated = 0;
            }
        }
        send_report(state);
        return;
    }

    // process fifo

    e_slot slot = haptic_sink_fifo_peek(&state->fifo);
    if (slot != slot_nb) {
        clear_report(state);
        dprintf("< ");
        ff_lg_convert_slot(&state->slots[slot].data, slot, &state->last_report, state->caps);
        if(gimx_params.debug.haptic) {
            ff_lg_decode_command(data);
        }
        send_report(state);
    }

#ifndef WIN32
    if (state->last_report.data[1] != 0) {
        // keep sending something to ensure bandwidth reservation
        for (i = 0; i < state->fifo.size; ++i) {
            e_slot slot = state->fifo.items[i];
            // check if at least one force is running
            if (state->slots[slot].data.playing) {
                dprintf("keep sending last command\n");
                send_report(state);
                return;
            }
        }
    }
#endif
}

static int hid_write_cb(void * user, int status) {

    struct haptic_sink_state * state = (struct haptic_sink_state *) user;

    state->write_pending = 0;

    if (status != -1) {
        haptic_sink_lg_ack(state);
    }

    haptic_sink_lg_update(state);

    return (status < 0) ? -1 : 0;
}

static int hid_close_cb(void * user) {

    struct haptic_sink_state * state = (struct haptic_sink_state *) user;

#ifdef WIN32
    ghid_close(state->hid);
#endif
    state->hid = NULL;

    return 1;
}

static struct haptic_sink_state * haptic_sink_lg_init(int joystick) {

    s_haptic_core_ids ids = { 0x0000, 0x0000 };

    struct ghid_device * hid;

#ifndef WIN32
    hid = ginput_joystick_get_hid(joystick);
    if (hid != NULL) {
        const s_hid_info * info = ghid_get_hid_info(hid);
        ids.vid = info->vendor_id;
        ids.pid = info->product_id;
    } else {
        return NULL;
    }
#else
    if (ginput_joystick_get_usb_ids(joystick, &ids.vid, &ids.pid) == -1) {
        return NULL;
    }
    hid = ghid_open_ids(ids.vid, ids.pid);
    if (hid == NULL) {
        return NULL;
    }
#endif

    void * ptr = calloc(1, sizeof(struct haptic_sink_state));

    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc")
#ifdef WIN32
        ghid_close(hid);
#endif
        return NULL;
    }

    struct haptic_sink_state * state = (struct haptic_sink_state *) ptr;

    state->hid = hid;
    state->pid = ids.pid;

    state->caps = ff_lg_get_caps(ids.pid);

    GHID_CALLBACKS ghid_callbacks = {
            .fp_read = NULL,
            .fp_write = hid_write_cb,
            .fp_close = hid_close_cb,
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if(ghid_register(hid, state, &ghid_callbacks) < 0)
    {
      return NULL;
    }

    return state;
}

static void haptic_sink_lg_clean(struct haptic_sink_state * state) {

    uint8_t mask = 0;
    uint8_t * data = state->last_report.data + 1;

    // stop playing forces

    if (state->slots[slot_constant].data.playing) {
        mask |= (1 << slot_constant);
    }
    if (state->slots[slot_spring].data.playing) {
        mask |= (1 << slot_spring);
    }
    if (state->slots[slot_damper].data.playing) {
        mask |= (1 << slot_damper);
    }

    if (mask) {
        clear_report(state);
        data[0] = (mask << 4) | FF_LG_CMD_STOP;
        ghid_write_timeout(state->hid, state->last_report.data, sizeof(state->last_report.data), 1000);
    }

#ifdef WIN32
    ghid_close(state->hid);
#endif

    free(state);
}

static void haptic_sink_lg_process(struct haptic_sink_state * state, const s_haptic_core_data * data) {

    e_slot slot = slot_nb;

    switch(data->type) {
    case E_DATA_TYPE_NONE:
        break;
    case E_DATA_TYPE_RUMBLE:
        break;
    case E_DATA_TYPE_CONSTANT:
        slot = slot_constant;
        break;
    case E_DATA_TYPE_SPRING:
        slot = slot_spring;
        break;
    case E_DATA_TYPE_DAMPER:
        slot = slot_damper;
        break;
    case E_DATA_TYPE_LEDS:
        if (state->caps & FF_LG_CAPS_LEDS) {
            slot = slot_leds;
        } else {
            static int warn = 1;
            if (warn == 1) {
                gwarn("skipping unsupported set leds commands\n");
                warn = 0;
            }
        }
        break;
    case E_DATA_TYPE_RANGE:
        if (state->caps & FF_LG_CAPS_RANGE) {
            slot = slot_range;
        } else {
            static int warn = 1;
            if (warn == 1) {
                gwarn("skipping unsupported change wheel range commands\n");
                warn = 0;
            }
        }
        break;
    }

    if (slot != slot_nb) {
        state->slots[slot].data = *data;
        state->slots[slot].updated = 1;
        haptic_sink_fifo_push(&state->fifo, slot);
    }
}


static s_haptic_core_ids haptic_sink_lg_ids[] = {
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP  },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE     },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL        },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DFP_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G25_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL        },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G27_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2       },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G29_WHEEL         },
        { .vid = 0x0000,      .pid = 0x0000       }, // end of table
};

static s_haptic_sink sink_lg = {
        .name = "haptic_sink_lg",
        .ids = haptic_sink_lg_ids,
        .init = haptic_sink_lg_init,
        .clean = haptic_sink_lg_clean,
        .process = haptic_sink_lg_process,
        .update = haptic_sink_lg_update
};

void haptic_sink_lg_constructor(void) __attribute__((constructor));
void haptic_sink_lg_constructor(void) {

    haptic_sink_register(&sink_lg);
}
