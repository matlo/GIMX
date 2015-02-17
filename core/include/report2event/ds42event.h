/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef DS42EVENT_H_
#define DS42EVENT_H_

#include <GE.h>
#include <controller2.h>

void ds42event(int adapter_id, s_report* current, s_report* previous,
    int joystick_id, int (*callback)(GE_Event*));

#endif /* DS42EVENT_H_ */
