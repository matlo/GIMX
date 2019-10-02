/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef MACROS_H
#define MACROS_H

#include <gimxinput/include/ginput.h>

void macro_lookup(GE_Event*);
unsigned int macro_process();

void macros_init();
void macros_clean();
int macros_process_line(const char* line);

#endif
