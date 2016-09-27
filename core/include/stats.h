/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef STATS_H_
#define STATS_H_

void stats_init(int id);
void stats_update(int id);
int stats_get_frequency(int id);

#endif /* STATS_H_ */
