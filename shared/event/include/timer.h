/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>

int timer_getfd();
int timer_start(struct timespec* period);
void timer_close();

#endif /* TIMER_H_ */
