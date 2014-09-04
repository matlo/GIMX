/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef DS4_WRAPPER_H_
#define DS4_WRAPPER_H_

#include <GE.h>
#include <ds4.h>

void ds4_wrapper_set_event_callback(int (*fp)(GE_Event*));
void ds4_wrapper(s_report_ds4* current, s_report_ds4* previous, int device_id);

#endif /* DS4_WRAPPER_H_ */
