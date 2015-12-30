/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef _360PAD2EVENT_H_
#define _360PAD2EVENT_H_

#include <controller2.h>
#include <ginput.h>

void _360Pad2event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*));

#endif /* _360PAD2EVENT_H_ */
