/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TIMER_H_
#define TIMER_H_

#ifndef WIN32
#define TIMER int
#else
#include <windows.h>
#define TIMER HANDLE
#endif


#ifdef __cplusplus
extern "C" {
#endif

TIMER timer_get();
TIMER timer_start(int usec);
int timer_close(int unused);
int timer_read(int unused);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H_ */
