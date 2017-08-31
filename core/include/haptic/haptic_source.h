/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_SOURCE_H_
#define HAPTIC_SOURCE_H_

#include <stdint.h>
#include <stddef.h>

#include "haptic_core.h"

struct haptic_source_state;

typedef struct {
    const char * name;
    const s_haptic_core_ids * ids;
    struct haptic_source_state * (* init)(s_haptic_core_ids ids);
    void (* clean)(struct haptic_source_state * state);
    void (* process)(struct haptic_source_state * state, size_t size, const unsigned char * data);
    int (* get)(struct haptic_source_state * state, s_haptic_core_data * data);
} s_haptic_source;

int haptic_source_register(s_haptic_source * source);

const s_haptic_source * haptic_source_get(s_haptic_core_ids ids);

#endif /* HAPTIC_SOURCE_H_ */
