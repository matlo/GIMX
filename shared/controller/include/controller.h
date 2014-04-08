/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <defs.h>
#include <report.h>

#define DEFAULT_MAX_AXIS_VALUE MAX_AXIS_VALUE_8BITS

#ifdef __cplusplus
extern "C" {
#endif

const char* controller_get_name(e_controller_type type);
e_controller_type controller_get_type(const char* name);

void controller_register_params(e_controller_type type, s_controller_params* params);
void controller_gpp_set_params(e_controller_type type);
int controller_get_min_refresh_period(e_controller_type type);
int controller_get_default_refresh_period(e_controller_type type);

inline int controller_get_max_signed(e_controller_type, int);
inline int controller_get_max_unsigned(e_controller_type, int);
inline int controller_get_mean_unsigned(e_controller_type, int);
inline double controller_get_axis_scale(e_controller_type, int);

typedef struct {
    int index;
    char dir;
} s_axis_index;

s_axis_index controller_get_axis_index_from_name(const char*);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER_H_ */
