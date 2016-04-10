/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SDLINPUT_H_
#define SDLINPUT_H_

#include <ginput.h>

#define MAX_EVENTS 256

int sdlinput_init(unsigned char mkb_src, int(*callback)(GE_Event*));
void sdlinput_quit();

int sdlinput_joystick_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short));
void sdlinput_joystick_close(int);
const char* sdlinput_joystick_name(int);
const char* sdlinput_mouse_name(int);
const char* sdlinput_keyboard_name(int);

int sdlinput_joystick_has_ff_rumble(int joystick);
int sdlinput_joystick_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong);

#ifndef WIN32
int sdlinput_joystick_get_uhid_id(int joystick);
#else
int sdlinput_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product);
#endif

void sdlinput_sync_process();

#endif /* SDLINPUT_H_ */
