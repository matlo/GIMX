/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

inline int timer_getfd();
int timer_start(struct timespec*);
int timer_close(int unused);
int timer_read(int unused);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H_ */
