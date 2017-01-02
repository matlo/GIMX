/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef REPORT2EVENT_H_
#define REPORT2EVENT_H_

#include <gimxcontroller/include/controller2.h>
#include <ginput.h>

void report2event_set_callback(int (*fp)(GE_Event*));
void report2event(e_controller_type type, int adapter_id, s_report* current,
    s_report* previous, int joystick_id);

#endif /* REPORT2EVENT_H_ */
