/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONVERSION_H_
#define CONVERSION_H_

#include <SDL/SDL.h>

#define MAX_NAME_LENGTH 32

#define AXIS_X 0
#define AXIS_Y 1

#define MOUSE_AXIS_X "x"
#define MOUSE_AXIS_Y "y"

#define MOUSE_BUTTONS_MAX 12

typedef struct {
    int value;
    int index;
} s_axis_index;

SDLKey get_key_from_buffer(const char*);
const char* get_axis_name(int);
int get_button_index_from_name(const char*);
s_axis_index get_axis_index_from_name(const char*);
const char* get_chars_from_key(SDLKey);
const char* get_chars_from_button(int);
unsigned int get_mouse_event_id_from_buffer(const char*);

#endif /* CONVERSION_H_ */
