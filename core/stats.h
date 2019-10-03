/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef STATS_H_
#define STATS_H_

struct stats;

enum stats_type {
    E_STATS_TYPE_CONTROLLER,
    E_STATS_TYPE_MOUSE
};

struct stats * stats_init(enum stats_type type);
void stats_clean(struct stats * s);
void stats_update(struct stats * s);
int stats_get_frequency(struct stats * s);

#endif /* STATS_H_ */
