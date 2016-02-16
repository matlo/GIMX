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
  unsigned int buffer_size;
  double filter;
  e_mouse_mode mode;
}s_mouse_options;

typedef struct
{
  unsigned int axis;
  int coef[4];
} s_js_corr;

typedef struct
{
  double* mx;
  double* my;
  double* ex;
  double* ey;
  int rd;
  int vel;
  unsigned int* dzx;
  unsigned int* dzy;
  e_shape* dzs;
  s_mouse_options options;
  unsigned int dpi;
}s_mouse_cal;

typedef struct
{
  int button;
  int axis;
  int threshold;
  double multiplier;
  double exponent;
  e_shape shape;
  unsigned int dead_zone;

  s_axis_props axis_props;
}s_mapper;

typedef struct
{
  int nb_mappers;
  s_mapper* mappers;
} s_mapper_table;

typedef struct
{
  unsigned int controller_id;
  unsigned int config_id;
  struct
  {
    e_device_type type;
    int id;
#ifndef WIN32
    int uhid_id;
#else
    struct
    {
      unsigned short vendor;
      unsigned short product;
    } usb_ids;
#endif
  } device;
  struct
  {
    e_event_type type;
    int id;
  } event;
  union
  {
    s_mapper mapper;
    struct
    {
      int switch_back;
      int delay;
    } trigger;
    s_mouse_options mouse_options;
    s_js_corr joystick_correction;
  } params;
}s_config_entry;

typedef struct
{
  struct
  {
    struct
    {
      int type;
      int id;
    } device;
    int button; // -1 means no down button
  } down;
  struct
  {
    struct
    {
      int type;
      int id;
    } device;
    int button; // -1 means no up button
  } up;
  struct
  {
    unsigned int dead_zone;
    e_shape shape;
    unsigned int steps;
  } params;
  double value;
  double max_value;
  double step;
  double dead_zone;
}s_intensity;

void cfg_trigger_init();
void cfg_trigger_lookup(GE_Event*);
void cfg_config_activation();
void cfg_intensity_lookup(GE_Event*);
void cfg_process_event(GE_Event*);
s_mouse_control* cfg_get_mouse_control(int);
void cfg_process_rumble_event(GE_Event* event);
void cfg_process_rumble();
int cfg_is_joystick_used(int);
void cfg_process_motion_event(GE_Event*);
void cfg_process_motion();
void cfg_set_trigger(s_config_entry* entry);
void cfg_set_controller_dpi(int controller, unsigned int dpi);
void cfg_set_axis_intensity(s_config_entry* entry, int axis, s_intensity* intensity);
void cfg_intensity_init();
int cfg_add_binding(s_config_entry* entry);
s_mapper_table* cfg_get_mouse_axes(int, int, int);
void cfg_clean();
void cfg_read_calibration();
void cfg_add_js_corr(uint8_t device, s_js_corr * corr);

#endif /* CONFIG_H_ */
