/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GPP_CON_H_
#define GPP_CON_H_

#include <defs.h>

int gpp_connect(int id, const char* device);

int gpp_send(int id, e_controller_type type, int axis[AXIS_MAX]);

void gpp_disconnect(int id);

#endif /* GPP_CON_H_ */
