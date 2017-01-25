/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_CONV_H_
#define FF_CONV_H_

#include <haptic/ff_lg.h>

int ff_conv_init(int device, unsigned short pid);
void ff_conv_process_report(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
int ff_conv_get_event(int device, GE_Event * event);
void ff_conv_ack(int device);

#endif /* FF_CONV_H_ */
