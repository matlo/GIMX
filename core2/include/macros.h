/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef MACROS_H
#define MACROS_H

#include <GE.h>

void macro_lookup(GE_Event*);
void macro_process();

void macro_set_controller_device(int, int, int);

void macros_init();
void macros_read();
void free_macros();

uint16_t get_key_from_buffer(const char*);

#endif
