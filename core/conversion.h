/*
 * conversion.h
 *
 *  Created on: 8 janv. 2011
 *      Author: matlo
 */

#ifndef CONVERSION_H_
#define CONVERSION_H_

#include <SDL/SDL.h>

#define MAX_NAME_LENGTH 32

#define AXIS_X 0
#define AXIS_Y 1

#define MOUSE_AXIS_X "x"
#define MOUSE_AXIS_Y "y"

#define MOUSE_BUTTONS_MAX 15

typedef struct {
    int baindex;
    int tavalue;
    int tindex;
    int taindex;
} s_axis_index;

SDLKey get_key_from_buffer(const char*);
int get_button_index_from_name(const char*);
s_axis_index get_axis_index_from_name(const char*);
const char* get_chars_from_key(SDLKey);
unsigned int get_mouse_event_id_from_buffer(const char*);

#endif /* CONVERSION_H_ */
