/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxtime/include/gtime.h>
#include <gimx.h>
#include <stdlib.h>
#include "stats.h"

#define STATS_PERIOD 500000000LL //ns

struct stats {
    enum stats_type type;
    gtime last;
    unsigned int cpt;
    unsigned int periods[21]; // count periods from 0 to 20 ms
    int period;
};

void stats_clean(struct stats *s) {

    free(s);
}

struct stats* stats_init(enum stats_type type) {

    struct stats *s = calloc(1, sizeof(*s));
    if (s == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc");
        return NULL;
    }

    s->type = type;

    if (s->type == E_STATS_TYPE_CONTROLLER) {
        s->last = gtime_gettime();
    } else {
        s->period = -1;
    }

    return s;
}

void stats_update(struct stats *s) {

    s->cpt++;

    if (s->type == E_STATS_TYPE_MOUSE && s->period == -1) {
        gtime now = gtime_gettime();
        if (s->last != 0) {
            gtime delta = now - s->last;
            unsigned int index = (delta + 250000UL) / 1000000UL;
            if (index < sizeof(s->periods) / sizeof(*s->periods)) {
                if (gimx_params.debug.stats) {
                    ginfo("tnow = "GTIME_FS", delta = "GTIME_FS", index = %u\n", now, delta, index);
                }
                s->periods[index]++;
            }
        }
        s->last = now;
        // compute mouse frequency on first 250 reports (2s @125Hz, 2.5s @100Hz, 2.8s @90Hz)
        if (s->cpt > 250) {
            unsigned int i;
            for (i = 1; i < sizeof(s->periods) / sizeof(*s->periods); ++i) {
                if (s->periods[i] > s->cpt * 10 / 100) {
                    s->period = i * 1000;
                    break;
                }
            }
        }
    }
}

int stats_get_period(struct stats *s) {

    int ret = -1;

    if (s->type == E_STATS_TYPE_CONTROLLER) {
        gtime tnow = gtime_gettime();

        gtime tdiff = tnow - s->last;

        if (tdiff > STATS_PERIOD) {
            ret = GTIME_USEC(tdiff / s->cpt);
            s->last = tnow;
            s->cpt = 0;
        }
    } else if(s->type == E_STATS_TYPE_MOUSE) {
        ret = s->period;
    }

    return ret;
}
