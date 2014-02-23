/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controllers/controller.h>
#include "controllers/ds3.h"
#include "emuclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <connectors/udp_con.h>
#include <config.h>

static s_controller controller[MAX_CONTROLLERS] = {};

/*
 * These tables are used to retrieve the default controller for a device and vice versa.
 */
static int controller_device[E_DEVICE_TYPE_NB][MAX_CONTROLLERS];
static int device_controller[E_DEVICE_TYPE_NB][MAX_DEVICES];

void controller_init()
{
  unsigned int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller[i].type = C_TYPE_DEFAULT;
    controller[i].serial = SERIALOBJECT_UNDEF;
    controller[i].dst_fd = -1;
    controller[i].src_fd = -1;
  }
  for(j=0; j<E_DEVICE_TYPE_NB; ++j)
  {
    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
      controller_device[j][i] = -1;
    }
    for(i=0; i<MAX_DEVICES; ++i)
    {
      device_controller[j][i] = -1;
    }
  }
}

/*
 * Set the port of the controller.
 * If it's already used for another controller, do nothing.
 */
int controller_set_port(unsigned char index, char* portname)
{
  unsigned char i;
  if(index >= MAX_CONTROLLERS)
  {
    return -1;
  }
  for(i=0; i<index; ++i)
  {
    if(controller[i].portname && !strcmp(controller[i].portname, portname))
    {
      return -1;
    }
  }
  controller[index].portname = portname;
  return 0;
}

inline s_controller* get_controller(unsigned char index)
{
  if(index < MAX_CONTROLLERS)
  {
    return controller+index;
  }
  return NULL;
}

void set_axis_value(unsigned char c, int axis, int value)
{
  if(axis >= 0 && axis < AXIS_MAX)
  {
    controller[c].axis[axis] = value;
  }
}

extern int joystick_max_unsigned_axis_value[AXIS_MAX];
extern int ds2_max_unsigned_axis_value[AXIS_MAX];
extern int ds3_max_unsigned_axis_value[AXIS_MAX];
extern int ds4_max_unsigned_axis_value[AXIS_MAX];
extern int xbox_max_unsigned_axis_value[AXIS_MAX];
extern int x360_max_unsigned_axis_value[AXIS_MAX];
extern int xone_max_unsigned_axis_value[AXIS_MAX];

typedef struct
{
  int min_refresh_period;
  int default_refresh_period;
  int* max_unsigned_axis_value;
} s_controller_params;

s_controller_params controller_params[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] =
    {
        .min_refresh_period = 1000,
        .default_refresh_period = 4000,
        .max_unsigned_axis_value = joystick_max_unsigned_axis_value
    },
    [C_TYPE_360_PAD] =
    {
        .min_refresh_period = 1000,
        .default_refresh_period = 8000,
        .max_unsigned_axis_value = x360_max_unsigned_axis_value
    },
    [C_TYPE_SIXAXIS] =
    {
        .min_refresh_period = 1000,
        .default_refresh_period = 10000,
        .max_unsigned_axis_value = ds3_max_unsigned_axis_value
    },
    [C_TYPE_PS2_PAD] =
    {
        .min_refresh_period = 16000,
        .default_refresh_period = 16000,
        .max_unsigned_axis_value = ds2_max_unsigned_axis_value
    },
    [C_TYPE_XBOX_PAD] =
    {
        .min_refresh_period = 4000,
        .default_refresh_period = 8000,
        .max_unsigned_axis_value = xbox_max_unsigned_axis_value
    },
    [C_TYPE_DS4] =
    {
        .min_refresh_period = 1000,
        .default_refresh_period = 12500,
        .max_unsigned_axis_value = ds4_max_unsigned_axis_value
    },
    [C_TYPE_XONE_PAD] =
    {
        /*
         * TODO XONE
         */
        .min_refresh_period = 1000,
        .default_refresh_period = 4000,
        .max_unsigned_axis_value = xone_max_unsigned_axis_value
    },
    [C_TYPE_GPP] =
    {
        .min_refresh_period = 1000,//to be updated according to the controller type
        .default_refresh_period = 4000,//to be updated according to the controller type
        .max_unsigned_axis_value = ds3_max_unsigned_axis_value
    },
    [C_TYPE_DEFAULT] =
    {
        .min_refresh_period = 11250,
        .default_refresh_period = 11250,
        .max_unsigned_axis_value = ds3_max_unsigned_axis_value
    },
};

void controller_gpp_set_refresh_periods(e_controller_type type)
{
  controller_params[C_TYPE_GPP].min_refresh_period = controller_params[type].min_refresh_period;
  controller_params[C_TYPE_GPP].default_refresh_period = controller_params[type].default_refresh_period;
}

int get_min_refresh_period(e_controller_type type)
{
  return controller_params[type].min_refresh_period;
}

int get_default_refresh_period(e_controller_type type)
{
  return controller_params[type].default_refresh_period;
}

inline int get_max_unsigned(e_controller_type type, int axis)
{
  return controller_params[type].max_unsigned_axis_value[axis];
}

inline int get_max_signed(e_controller_type type, int axis)
{
  if(axis < abs_axis_0)
  {
    /*
     * relative axis
     */
    return get_max_unsigned(type, axis) / 2 + 1;
  }
  else
  {
    /*
     * absolute axis
     */
    return get_max_unsigned(type, axis);
  }
}

inline int get_mean_unsigned(e_controller_type type, int axis)
{
  return get_max_unsigned(type, axis) / 2 + 1;
}

inline double get_axis_scale(e_controller_type type, int axis)
{
  return (double) get_max_unsigned(type, axis) / DEFAULT_MAX_AXIS_VALUE;
}

typedef struct {
    const char* name;
    s_axis_index aindex;
} s_axis_name_index;

static const s_axis_name_index axis_name_index[] =
{
    {.name="rstick x",     {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick y",     {.value=-1,  .index=sa_rstick_y}},
    {.name="lstick x",     {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick y",     {.value=-1,  .index=sa_lstick_y}},
    {.name="rstick left",  {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick right", {.value= 1,  .index=sa_rstick_x}},
    {.name="rstick up",    {.value=-1,  .index=sa_rstick_y}},
    {.name="rstick down",  {.value= 1,  .index=sa_rstick_y}},
    {.name="lstick left",  {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick right", {.value= 1,  .index=sa_lstick_x}},
    {.name="lstick up",    {.value=-1,  .index=sa_lstick_y}},
    {.name="lstick down",  {.value= 1,  .index=sa_lstick_y}},
    {.name="acc x",        {.value=-1,  .index=sa_acc_x}},
    {.name="acc y",        {.value=-1,  .index=sa_acc_y}},
    {.name="acc z",        {.value=-1,  .index=sa_acc_z}},
    {.name="gyro",         {.value=-1,  .index=sa_gyro}},
    {.name="acc x -",      {.value=-1,  .index=sa_acc_x}},
    {.name="acc y -",      {.value=-1,  .index=sa_acc_y}},
    {.name="acc z -",      {.value=-1,  .index=sa_acc_z}},
    {.name="gyro -",       {.value=-1,  .index=sa_gyro}},
    {.name="acc x +",      {.value= 1,  .index=sa_acc_x}},
    {.name="acc y +",      {.value= 1,  .index=sa_acc_y}},
    {.name="acc z +",      {.value= 1,  .index=sa_acc_z}},
    {.name="gyro +",       {.value= 1,  .index=sa_gyro}},
    {.name="up",           {.value= 0,  .index=sa_up}},
    {.name="down",         {.value= 0,  .index=sa_down}},
    {.name="right",        {.value= 0,  .index=sa_right}},
    {.name="left",         {.value= 0,  .index=sa_left}},
    {.name="r1",           {.value= 0,  .index=sa_r1}},
    {.name="r2",           {.value= 0,  .index=sa_r2}},
    {.name="l1",           {.value= 0,  .index=sa_l1}},
    {.name="l2",           {.value= 0,  .index=sa_l2}},
    {.name="circle",       {.value= 0,  .index=sa_circle}},
    {.name="square",       {.value= 0,  .index=sa_square}},
    {.name="cross",        {.value= 0,  .index=sa_cross}},
    {.name="triangle",     {.value= 0,  .index=sa_triangle}},
};

s_axis_index get_axis_index_from_name(const char* name)
{
  int i;
  s_axis_index none = {-1, -1};
  for(i=0; i<sizeof(axis_name_index)/sizeof(s_axis_name_index); ++i)
  {
    if(!strcmp(axis_name_index[i].name, name))
    {
      return axis_name_index[i].aindex;
    }
  }
  return none;
}

void controller_dump_state(s_controller* c)
{
  int i;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int* axis = c->axis;

  printf("%ld %ld.%06ld", (c-controller)/sizeof(s_controller), tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", ds3_get_axis_name(i), axis[i]);
  }

  printf("\n");
}

int controller_network_read(int id)
{
  unsigned char buf[sizeof(controller->axis)];
  int nread = 0;
  int ret;
  while(nread != sizeof(buf))
  {
    if((ret = read(controller[id].src_fd, buf+nread, sizeof(buf)-nread)) < 0)
    {
      if(errno != EAGAIN)
      {
        return -1;
      }
    }
    else
    {
      nread += ret;
    }
  }
  memcpy(controller[id].axis, buf, sizeof(controller->axis));
  controller[id].send_command = 1;
  return 0;
}

/*
 * Set the default device for a controller.
 */
void controller_set_device(int controller, e_device_type device_type, int device_id)
{
  int type_index = device_type-1;

  if(controller < 0 || controller >= MAX_CONTROLLERS
  || type_index < 0 || type_index >= E_DEVICE_TYPE_NB
  || device_id < 0 || type_index > MAX_DEVICES)
  {
    fprintf(stderr, "set_controller_device error\n");
    return;
  }
  if(controller_device[type_index][controller] < 0)
  {
    controller_device[type_index][controller] = device_id;
    device_controller[type_index][device_id] = controller;
  }
  else if(controller_device[type_index][controller] != device_id)
  {
    gprintf(_("macros are not not available for: "));
    if(device_type == E_DEVICE_TYPE_KEYBOARD)
    {
      gprintf(_("keyboard %s (%d)\n"), GE_KeyboardName(device_id), GE_KeyboardVirtualId(device_id));
    }
    else if(device_type == E_DEVICE_TYPE_MOUSE)
    {
      gprintf(_("mouse %s (%d)\n"), GE_MouseName(device_id), GE_MouseVirtualId(device_id));
    }
    else if(device_type == E_DEVICE_TYPE_JOYSTICK)
    {
      gprintf(_("joystick %s (%d)\n"), GE_JoystickName(device_id), GE_JoystickVirtualId(device_id));
    }
  }
}

/*
 * Get the default device for a controller.
 */
int controller_get_device(e_device_type device_type, int controller)
{
  return controller_device[device_type-1][controller];
}

/*
 * Get the default controller for a device.
 */
int controller_get_controller(e_device_type device_type, int device_id)
{
  return device_controller[device_type-1][device_id];
}
