/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <SDL/SDL.h>
#include "sixaxis.h"
#include "conversion.h"

#define MAX_CONTROLLERS 7
#define MAX_CONFIGURATIONS 8
#define MAX_DEVICES 256
#define MAX_CONTROLS 256

#define MAX_BUFFERSIZE 256

#define X_NODE_ROOT "root"
#define X_NODE_CONTROLLER "controller"
#define X_NODE_CONFIGURATION "configuration"
#define X_NODE_TRIGGER "trigger"
#define X_NODE_INTENSITY_LIST "intensity_list"
#define X_NODE_INTENSITY "intensity"
#define X_NODE_BUTTON_MAP "button_map"
#define X_NODE_AXIS_MAP "axis_map"
#define X_NODE_DEVICE "device"
#define X_NODE_EVENT "event"
#define X_NODE_AXIS "axis"
#define X_NODE_BUTTON "button"
#define X_NODE_UP "up"
#define X_NODE_DOWN "down"

#define X_ATTR_ID "id"
#define X_ATTR_DPI "dpi"
#define X_ATTR_TYPE "type"
#define X_ATTR_NAME "name"
#define X_ATTR_BUTTON_ID "button_id"
#define X_ATTR_THRESHOLD "threshold"
#define X_ATTR_DEADZONE "dead_zone"
#define X_ATTR_MULTIPLIER "multiplier"
#define X_ATTR_EXPONENT "exponent"
#define X_ATTR_SHAPE "shape"
#define X_ATTR_BUFFERSIZE "buffer_size"
#define X_ATTR_FILTER "filter"
#define X_ATTR_SWITCH_BACK "switch_back"
#define X_ATTR_DELAY "delay"
#define X_ATTR_STEPS "steps"
#define X_ATTR_CONTROL "control"

#define X_ATTR_VALUE_KEYBOARD "keyboard"
#define X_ATTR_VALUE_MOUSE "mouse"
#define X_ATTR_VALUE_JOYSTICK "joystick"
#define X_ATTR_VALUE_BUTTON "button"
#define X_ATTR_VALUE_AXIS "axis"
#define X_ATTR_VALUE_AXIS_DOWN "axis down"
#define X_ATTR_VALUE_AXIS_UP "axis up"
#define X_ATTR_VALUE_CIRCLE "Circle"
#define X_ATTR_VALUE_RECTANGLE "Rectangle"
#define X_ATTR_VALUE_YES "yes"
#define X_ATTR_VALUE_NO "no"

typedef enum
{
  E_DEVICE_TYPE_UNKNOWN,
  E_DEVICE_TYPE_KEYBOARD,
  E_DEVICE_TYPE_MOUSE,
  E_DEVICE_TYPE_JOYSTICK
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
  int postpone[MOUSE_BUTTONS_MAX];
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
  unsigned int* bsx;
  double* fix;
  unsigned int* bsy;
  double* fiy;
  int dpi;
}s_mouse_cal;

typedef struct
{
  int change;
  int send_command;
  int ts_axis[SA_MAX][2]; //issue 15
} s_controller;

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
  unsigned int buffer_size;
  double filter;

  int controller_axis;
  int controller_axis_value; //only for button to axis mapping
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
void cfg_trigger_lookup(SDL_Event*);
void cfg_config_activation();
void cfg_intensity_lookup(SDL_Event*);
void cfg_process_event(SDL_Event*);
inline s_mouse_control* cfg_get_mouse_control(int);
int cfg_is_joystick_used(int);
void cfg_process_motion_event(SDL_Event*);
void cfg_process_motion();
inline s_trigger* cfg_get_trigger(int, int);
inline s_intensity* cfg_get_axis_intensity(int, int, int);
inline s_mapper** cfg_get_joystick_axes(int, int, int);
inline s_mapper** cfg_get_joystick_buttons(int, int, int);
inline s_mapper** cfg_get_mouse_axes(int, int, int);
inline s_mapper** cfg_get_mouse_buttons(int, int, int);
inline s_mapper** cfg_get_keyboard_buttons(int, int, int);

#endif /* CONFIG_H_ */
