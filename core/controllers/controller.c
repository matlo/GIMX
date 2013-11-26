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

static s_controller controller[MAX_CONTROLLERS] =
{ };

void controller_init_type()
{
  unsigned char i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller[i].type = C_TYPE_DEFAULT;
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

static int min_refresh_period[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] =  1000,
    [C_TYPE_360_PAD]  =  1000,
    [C_TYPE_SIXAXIS]  =  1000,
    [C_TYPE_PS2_PAD]  = 16000,
    [C_TYPE_XBOX_PAD] =  4000,
    [C_TYPE_DS4]      =  1000,
    [C_TYPE_GPP]      =  1000,
    [C_TYPE_DEFAULT]  = 11250,
};

static int default_refresh_period[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] =  4000,
    [C_TYPE_360_PAD]  =  4000,
    [C_TYPE_SIXAXIS]  =  4000,
    [C_TYPE_PS2_PAD]  = 16000,
    [C_TYPE_XBOX_PAD] =  4000,
    [C_TYPE_DS4]      =  5000,
    [C_TYPE_GPP]      =  4000,
    [C_TYPE_DEFAULT]  = 11250,
};

int get_min_refresh_period(e_controller_type type)
{
  return min_refresh_period[type];
}

int get_default_refresh_period(e_controller_type type)
{
  return default_refresh_period[type];
}

extern int joystick_max_unsigned_axis_value[AXIS_MAX];
extern int ds2_max_unsigned_axis_value[AXIS_MAX];
extern int ds3_max_unsigned_axis_value[AXIS_MAX];
extern int ds4_max_unsigned_axis_value[AXIS_MAX];
extern int xbox_max_unsigned_axis_value[AXIS_MAX];
extern int x360_max_unsigned_axis_value[AXIS_MAX];

int* max_unsigned_axis_value[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] = joystick_max_unsigned_axis_value,
    [C_TYPE_PS2_PAD] = ds2_max_unsigned_axis_value,
    [C_TYPE_SIXAXIS] = ds3_max_unsigned_axis_value,
    [C_TYPE_DS4] = ds4_max_unsigned_axis_value,
    [C_TYPE_XBOX_PAD] = xbox_max_unsigned_axis_value,
    [C_TYPE_360_PAD] = x360_max_unsigned_axis_value,
    [C_TYPE_DEFAULT] = ds3_max_unsigned_axis_value,
};

inline int get_max_unsigned(e_controller_type type, int axis)
{
  return max_unsigned_axis_value[type][axis];
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
