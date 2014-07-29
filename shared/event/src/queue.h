/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <GE.h>

void queue_init();
int queue_push_event(GE_Event* ev);
int queue_peep_events(GE_Event *events, int numevents);

#endif
