/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_COMMON_H_
#define HAPTIC_COMMON_H_

#include <gimx.h>

#define dprintf(...) \
    do { \
        if(gimx_params.debug.haptic) { \
            printf(__VA_ARGS__); \
            fflush(stdout); \
        } \
    } while (0)

#endif /* HAPTIC_COMMON_H_ */
