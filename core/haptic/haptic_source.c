/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxcommon/include/gerror.h>
#include <string.h>
#include <stdlib.h>
#include <haptic/haptic_source.h>

static s_haptic_source ** sources = NULL;
static unsigned int nb_sources = 0;

void haptic_source_destructor(void) __attribute__((destructor));
void haptic_source_destructor(void) {

    free(sources);
    nb_sources = 0;
}

int haptic_source_register(s_haptic_source * parser) {

    void * ptr = realloc(sources, (nb_sources + 1) * sizeof(*sources));
    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("realloc")
        return -1;
    }
    sources = ptr;
    sources[nb_sources] = parser;
    ++nb_sources;
    return 0;
}

const s_haptic_source * haptic_source_get(s_haptic_core_ids ids) {

    unsigned int i;
    for (i = 0; i < nb_sources; ++i) {
        unsigned int j;
        for (j = 0; sources[i]->ids[j].vid != 0; ++j) {
            if (sources[i]->ids[j].vid == ids.vid && sources[i]->ids[j].pid == ids.pid) {
                return sources[i];
            }
        }
    }
    return NULL;
}
