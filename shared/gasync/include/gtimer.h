/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GTIMER_H_
#define GTIMER_H_

#include "gpoll.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
int gtimer_start(int user, int usec, GPOLL_READ_CALLBACK fp_read, GPOLL_CLOSE_CALLBACK fp_close,
    GPOLL_REGISTER_FD fp_register);
#else
int gtimer_start(int user, int usec, GPOLL_READ_CALLBACK fp_read, GPOLL_CLOSE_CALLBACK fp_close,
    GPOLL_REGISTER_HANDLE fp_register);
#endif
int gtimer_close(int timer);

#ifdef __cplusplus
}
#endif

#endif /* GTIMER_H_ */
