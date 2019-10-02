/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxtime/include/gtime.h>
#include <gimx.h>
#include <stdlib.h>

#define STATS_PERIOD 500000000LL //ns

struct stats {
    gtime tlast;
    int cpt;
};

void stats_clean(struct stats * s) {

    free(s);
}

struct stats * stats_init() {

    struct stats * s = calloc(1, sizeof(*s));
    if (s == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc");
        return NULL;
    }

    s->tlast = gtime_gettime();

    return s;
}

void stats_update(struct stats * s) {
    s->cpt++;
}

int stats_get_frequency(struct stats * s) {

    int ret = -1;

    gtime tnow = gtime_gettime();

    gtimediff tdiff = tnow - s->tlast;

    if (tdiff > STATS_PERIOD) {
        ret = s->cpt * 1000000000UL / tdiff;
        s->tlast = tnow;
        s->cpt = 0;
    }

    return ret;
}
