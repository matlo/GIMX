/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include "defs.h"

#define MAX_AXIS_VALUE_8BITS 255
#define MAX_AXIS_VALUE_10BITS 1023
#define MAX_AXIS_VALUE_14BITS 16383
#define MAX_AXIS_VALUE_16BITS 65535

#define CENTER_AXIS_VALUE_8BITS (MAX_AXIS_VALUE_8BITS/2+1)
#define CENTER_AXIS_VALUE_14BITS (MAX_AXIS_VALUE_14BITS/2+1)
#define CENTER_AXIS_VALUE_16BITS (MAX_AXIS_VALUE_16BITS/2+1)

#define AXIS_NAME_FORMAT "xxx_axis_yy"

inline int clamp(int min, int val, int max);

void control_register_names(e_controller_type type, const char** names);
const char* control_get_name(e_controller_type type, e_controller_axis_index index);
int control_get_index(const char* name);

#endif /* CONTROLS_H_ */
