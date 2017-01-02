/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef _360PAD2EVENT_H_
#define _360PAD2EVENT_H_

#include <gimxinput/include/ginput.h>
#include <gimxcontroller/include/controller.h>

void _360Pad2event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*));

#endif /* _360PAD2EVENT_H_ */
