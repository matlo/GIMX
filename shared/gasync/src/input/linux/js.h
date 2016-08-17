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
int js_register(const char* name, unsigned int effects, int (*haptic_cb)(const GE_Event * event));

int js_get_haptic(int joystick);
int js_set_haptic(const GE_Event * haptic);

int js_get_hid(int index);

#endif /* JS_H_ */
