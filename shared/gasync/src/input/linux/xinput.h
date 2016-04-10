/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef XINPUT_H_
#define XINPUT_H_

#include <ginput.h>

int xinput_init(int (*callback)(GE_Event*));

void xinput_quit();

void xinput_grab(int mode);

const char* xinput_get_mouse_name(int index);

const char* xinput_get_keyboard_name(int index);

#endif /* XINPUT_H_ */
