/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_TWEAKS_H_
#define HAPTIC_TWEAKS_H_

#include <haptic/haptic_core.h>

void haptic_rotation_apply(const s_haptic_core_wheel_rotation* rotation, s_haptic_core_data* data);
void haptic_tweak_apply(const s_haptic_core_tweaks * tweaks, s_haptic_core_data * data);

#endif /* HAPTIC_TWEAKS_H_ */
