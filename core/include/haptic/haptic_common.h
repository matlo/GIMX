/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_COMMON_H_
#define HAPTIC_COMMON_H_

#include <gimx.h>

#define dprintf(...) if(gimx_params.debug.haptic) printf(__VA_ARGS__)

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#endif /* HAPTIC_COMMON_H_ */
