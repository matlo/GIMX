/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <GE.h>
#include <controller2.h>

#define MAX_BUFFERSIZE 256

typedef enum
{
  E_MOUSE_MODE_AIMING = 0, //default mode
  E_MOUSE_MODE_DRIVING
}e_mouse_mode;

typedef enum
{
  E_DEVICE_TYPE_UNKNOWN,
  E_DEVICE_TYPE_KEYBOARD,
  E_DEVICE_TYPE_MOUSE,
  E_DEVICE_TYPE_JOYSTICK,
  E_DEVICE_TYPE_NB = E_DEVICE_TYPE_JOYSTICK
}e_device_type;

typedef enum
{
  E_EVENT_TYPE_UNKNOWN,
  E_EVENT_TYPE_BUTTON,
  E_EVENT_TYPE_AXIS,
  E_EVENT_TYPE_AXIS_DOWN,
  E_EVENT_TYPE_AXIS_UP
}e_event_type;

typedef enum
{
    E_SHAPE_CIRCLE,
    E_SHAPE_RECTANGLE
}e_shape;

typedef struct
{
  int change;
  int changed;
  double merge_x[MAX_BUFFERSIZE];
  double merge_y[MAX_BUFFERSIZE];
  int index;
  double x;
  double y;
  double residue_x;
  double residue_y;
  int postpone[GE_MOUSE_BUTTONS_MAX];
}s_mouse_control;

typedef struct
{
  double* mx;
  double* my;
  double* ex;
  double* ey;
  int rd;
  int vel;
  int* dzx;
  int* dzy;
  e_shape* dzs;
  unsigned int buffer_size;
  double filter;
  e_mouse_mode mode;
  int dpi;
}s_mouse_cal;

typedef struct
{
  int nb_mappers;

  int button;
  int axis;
  int threshold;
  double multiplier;
  double exponent;
  e_shape shape;
  int dead_zone;

  s_axis_props axis_props;
}s_mapper;

typedef struct
{
  int device_type;
  int device_id;
  int button;
  int switch_back;
  int delay; //ms
}s_trigger;

typedef struct
{
  int device_up_type;
  int device_up_id;
  int up_button;
  int device_down_type;
  int device_down_id;
  int down_button;
  double dead_zone;
  e_shape shape;
  double step;
  double value;
  double max_value;
}s_intensity;

void cfg_trigger_init();
void cfg_trigger_lookup(GE_Event*);
void cfg_config_activation();
void cfg_intensity_lookup(GE_Event*);
void cfg_process_event(GE_Event*);
inline s_mouse_control* cfg_get_mouse_control(int);
int cfg_is_joystick_used(int);
void cfg_process_motion_event(GE_Event*);
void cfg_process_motion();
inline s_trigger* cfg_get_trigger(int, int);
inline s_intensity* cfg_get_axis_intensity(int, int, int);
inline s_mapper** cfg_get_joystick_axes(int, int, int);
inline s_mapper** cfg_get_joystick_buttons(int, int, int);
inline s_mapper** cfg_get_mouse_axes(int, int, int);
inline s_mapper** cfg_get_mouse_buttons(int, int, int);
inline s_mapper** cfg_get_keyboard_buttons(int, int, int);

#endif /* CONFIG_H_ */
