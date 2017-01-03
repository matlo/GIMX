/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef RAWINPUT_H_
#define RAWINPUT_H_

#include "../../include/ginput.h"

#define RAWINPUT_CLASS_NAME "RawInputCatcher"
#define RAWINPUT_WINDOW_NAME "RawInputMsgWindow"

#ifdef __cplusplus
extern "C" {
#endif

int rawinput_init(int (*callback)(GE_Event*));
void rawinput_quit(void);

const char * rawinput_mouse_name(unsigned int mouse);
const char * rawinput_keyboard_name(unsigned int keyboard);

void rawinput_poll();

#ifdef __cplusplus
}
#endif

#endif
