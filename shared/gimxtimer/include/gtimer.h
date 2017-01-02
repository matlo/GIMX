/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GTIMER_H_
#define GTIMER_H_

#include <gimxpoll/include/gpoll.h>

#ifndef WIN32
typedef GPOLL_REGISTER_FD GTIMER_REGISTER_SOURCE;
typedef GPOLL_REMOVE_FD GTIMER_REMOVE_SOURCE;
#else
typedef GPOLL_REGISTER_HANDLE GTIMER_REGISTER_SOURCE;
typedef GPOLL_REMOVE_HANDLE GTIMER_REMOVE_SOURCE;
#endif

typedef struct {
    GPOLL_READ_CALLBACK fp_read;        // called on timer firing
    GPOLL_CLOSE_CALLBACK fp_close;      // called on timer failure
    GTIMER_REGISTER_SOURCE fp_register; // to register the timer to event sources
    GTIMER_REMOVE_SOURCE fp_remove;     // to remove the timer from event sources
} GTIMER_CALLBACKS;

#ifdef __cplusplus
extern "C" {
#endif

int gtimer_start(int user, unsigned int usec, const GTIMER_CALLBACKS * callbacks);
int gtimer_close(int timer);

#ifdef __cplusplus
}
#endif

#endif /* GTIMER_H_ */
