/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SDLINPUT_H_
#define SDLINPUT_H_

#include <ginput.h>

#define SDLINPUT_WINDOW_NAME "SDLInputMsgWindow"

int sdlinput_init(unsigned char mkb_src, int(*callback)(GE_Event*));
void sdlinput_quit();

int sdlinput_joystick_register(const char* name, unsigned int effects, int (*haptic_cb)(const GE_Event * event));
void sdlinput_joystick_close(int);
const char* sdlinput_joystick_name(int);
const char* sdlinput_mouse_name(int);
const char* sdlinput_keyboard_name(int);

int sdlinput_joystick_get_haptic(int joystick);
int sdlinput_joystick_set_haptic(const GE_Event * event);

int sdlinput_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product);

void sdlinput_sync_process();

void sdlinput_grab(int mode);

#endif /* SDLINPUT_H_ */
