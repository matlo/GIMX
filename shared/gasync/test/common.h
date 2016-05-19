/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <ginput.h>

int is_done();
void set_done();

void setup_handlers();

void display_devices();
int process_event(GE_Event*);

int timer_close(int user);
int timer_read(int user) ;

char * hid_select();

#endif /* COMMON_H_ */
