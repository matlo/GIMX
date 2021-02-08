/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_SINK_H_
#define HAPTIC_SINK_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "haptic_core.h"
#include "haptic_common.h"

struct haptic_sink_state;

typedef enum {
    E_HAPTIC_SINK_CAP_NONE = 0x00,
    E_HAPTIC_SINK_CAP_RUMBLE = (1 << 0),
    E_HAPTIC_SINK_CAP_CONSTANT = (1 << 1),
    E_HAPTIC_SINK_CAP_SPRING = (1 << 2),
    E_HAPTIC_SINK_CAP_DAMPER = (1 << 3),
} e_haptic_sink_caps;

typedef struct {
    const char * name;
    s_haptic_core_ids * ids;
    e_haptic_sink_caps caps;
    struct haptic_sink_state * (* init)(int joystick);
    void (* clean)(struct haptic_sink_state * state);
    void (* process)(struct haptic_sink_state * state, const s_haptic_core_data * data);
    void (* update)(struct haptic_sink_state * state);
} s_haptic_sink;

void haptic_sink_register(s_haptic_sink * sink);

const s_haptic_sink * haptic_sink_get(int joystick);

typedef enum {
    slot_constant,
    slot_spring,
    slot_damper,
    slot_leds,
    slot_range,
    // << add new slots here
    slot_nb,
} e_slot;

typedef struct {
    e_slot items[slot_nb];
    unsigned int size;
} s_haptic_sink_fifo;

static inline void haptic_sink_fifo_push(s_haptic_sink_fifo * fifo, e_slot slot) {
    unsigned int i;
    for (i = 0; i < fifo->size; ++i) {
        if (fifo->items[i] == slot) {
            dprintf("< already queued: %d\n", slot);
            return;
        }
    }
    if (i < slot_nb) {
        dprintf("< push: %d\n", slot);
        fifo->items[i] = slot;
        ++(fifo->size);
    }
}

static inline e_slot haptic_sink_fifo_peek(const s_haptic_sink_fifo * fifo) {
    e_slot slot = slot_nb;
    if (fifo->size > 0) {
        slot = fifo->items[0];
        dprintf("< peek: %d\n", slot);
    }
    return slot;
}

static inline void haptic_sink_fifo_remove(s_haptic_sink_fifo * fifo, e_slot slot) {
    unsigned int i;
    for (i = 0; i < fifo->size; ++i) {
        if (fifo->items[i] == slot) {
            dprintf("< remove: %d\n", slot);
            --fifo->size;
            memmove(fifo->items + i, fifo->items + i + 1, (fifo->size - i) * sizeof(*fifo->items));
            break;
        }
    }
}

#endif /* HAPTIC_SINK_H_ */
