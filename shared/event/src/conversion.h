/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONVERSION_H_
#define CONVERSION_H_

#include <GE.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t get_key_from_buffer(const char*);
const char* get_chars_from_key(uint16_t);
const char* get_chars_from_button(int);
int get_mouse_event_id_from_buffer(const char*);

#ifdef __cplusplus
}
#endif

#endif /* CONVERSION_H_ */
