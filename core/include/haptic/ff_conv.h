/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_CONV_H_
#define FF_CONV_H_

#include <haptic/ff_lg.h>

int ff_conv_init(int device, unsigned short pid);
int ff_conv(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE], GE_Event events[FF_LG_FSLOTS_NB]);

#endif /* FF_CONV_H_ */
