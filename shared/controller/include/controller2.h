/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER2_H_
#define CONTROLLER2_H_

#include <defs.h>
#include <report.h>

#define DEFAULT_MAX_AXIS_VALUE MAX_AXIS_VALUE_8BITS

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int axis;
    char props;
} s_axis_props;

typedef struct {
    const char* name;
    s_axis_props axis_props;
} s_axis_name_dir;

#define AXIS_PROP_NONE     0x00
#define AXIS_PROP_TOGGLE   0x01
#define AXIS_PROP_POSITIVE 0x02
#define AXIS_PROP_NEGATIVE 0x04
#define AXIS_PROP_CENTERED 0x08

const char* controller_get_name(e_controller_type type);
e_controller_type controller_get_type(const char* name);

void controller_register_params(e_controller_type type, s_controller_params* params);
void controller_register_axis_names(e_controller_type type, int nb, s_axis_name_dir* axis_names);
void controller_gpp_set_params(e_controller_type type);
int controller_get_min_refresh_period(e_controller_type type);
int controller_get_default_refresh_period(e_controller_type type);

inline int controller_get_max_signed(e_controller_type, int);
inline int controller_get_max_unsigned(e_controller_type, int);
inline int controller_get_mean_unsigned(e_controller_type, int);
inline double controller_get_axis_scale(e_controller_type, int);

s_axis_props controller_get_axis_index_from_name(const char*);

const char* controller_get_generic_axis_name_from_index(s_axis_props axis_props);

const char* controller_get_specific_axis_name_from_index(e_controller_type type, s_axis_props axis_props);

s_axis_props controller_get_axis_index_from_specific_name(e_controller_type type, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER2_H_ */
