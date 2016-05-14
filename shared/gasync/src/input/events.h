/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include <ginput.h>

#define MAX_EVENTS 256

int ev_init(unsigned char mkb_src, int(*callback)(GE_Event*));
void ev_quit();

int ev_joystick_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short));
void ev_joystick_close(int);
const char* ev_joystick_name(int);
const char* ev_mouse_name(int);
const char* ev_keyboard_name(int);

int ev_joystick_has_ff_rumble(int joystick);
int ev_joystick_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong);

#ifndef WIN32
int ev_joystick_get_hid(int joystick);
#else
int ev_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product);
#endif

void ev_grab_input(int);
void ev_pump_events();
void ev_sync_process();

#endif /* EVENTS_H_ */
