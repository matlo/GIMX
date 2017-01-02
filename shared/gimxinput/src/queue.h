/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <ginput.h>

void queue_init();
int queue_push_event(GE_Event* ev);
int queue_pop_events(GE_Event *events, int numevents);

#endif
