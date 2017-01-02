/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DS42EVENT_H_
#define DS42EVENT_H_

#include <ginput.h>
#include "../../../shared/gimxcontroller/include/controller.h"

void ds42event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*));

#endif /* DS42EVENT_H_ */
