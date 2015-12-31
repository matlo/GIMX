/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <ginput.h>

extern volatile int done;

void display_devices();
int process_event(GE_Event*);

int timer_close(int user);
int timer_read(int user) ;

char * hid_select();

#endif /* COMMON_H_ */
