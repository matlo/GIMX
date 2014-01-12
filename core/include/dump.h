/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DUMP_H
#define DUMP_H

#include "connectors/sixaxis.h"

void sixaxis_dump_state(struct sixaxis_state *state, int id);

#endif
