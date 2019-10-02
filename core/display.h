/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <gimxcontroller/include/controller.h>
#include <stats.h>

void display_init();
void display_end();
void display_run(e_controller_type type, int axis[], struct stats * s);
void display_calibration();

#endif /* DISPLAY_H_ */
