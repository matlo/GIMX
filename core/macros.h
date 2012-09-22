/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef MACROS_H
#define MACROS_H

#include <SDL/SDL.h>

void macro_lookup(int, int, int, int);
void macro_process();

void macro_set_controller_device(int, int, int);

void initialize_macros();
void free_macros();

SDLKey get_key_from_buffer(const char*);

#endif
