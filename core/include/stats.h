/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef STATS_H_
#define STATS_H_

struct stats;

struct stats * stats_init();
void stats_clean(struct stats * s);
void stats_update(struct stats * s);
int stats_get_frequency(struct stats * s);

#endif /* STATS_H_ */
