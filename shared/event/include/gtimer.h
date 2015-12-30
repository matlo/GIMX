/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GTIMER_H_
#define GTIMER_H_

#include <gpoll.h>

#ifdef __cplusplus
extern "C" {
#endif

int gtimer_start(int user, int usec, GPOLL_READ_CALLBACK fp_read, GPOLL_CLOSE_CALLBACK fp_close,
    GPOLL_REGISTER_FD fp_register);
int gtimer_close(int timer);
int gtimer_read(int timer);

#ifdef __cplusplus
}
#endif

#endif /* GTIMER_H_ */
