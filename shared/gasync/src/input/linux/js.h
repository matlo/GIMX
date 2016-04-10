/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef JS_H_
#define JS_H_

#include <ginput.h>

int js_init(int (*callback)(GE_Event*));
int js_close(int id);
void js_quit();
const char* js_get_name(int index);
int js_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short));

int js_has_ff_rumble(int joystick);
int js_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong);

int js_get_uhid_id(int index);

#endif /* JS_H_ */
