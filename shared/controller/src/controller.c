/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* controller_name[C_TYPE_MAX] =
{
  [C_TYPE_JOYSTICK] = "joystick",
  [C_TYPE_360_PAD]  = "360pad",
  [C_TYPE_SIXAXIS]  = "Sixaxis",
  [C_TYPE_PS2_PAD]  = "PS2pad",
  [C_TYPE_XBOX_PAD] = "XboxPad",
  [C_TYPE_DS4]      = "DS4",
  [C_TYPE_XONE_PAD] = "XOnePad",
  [C_TYPE_GPP]      = "GPP",
  [C_TYPE_DEFAULT]  = "none",
};

const char* controller_get_name(e_controller_type type)
{
  return controller_name[type];
}

static s_controller_params* controller_params[C_TYPE_MAX] = {};

void controller_register_params(e_controller_type type, s_controller_params* params)
{
  controller_params[type] = params;
}

void controller_init(void) __attribute__((constructor (102)));
void controller_init(void)
{
  int type;
  for(type=0; type<C_TYPE_MAX; ++type)
  {
    if(!controller_params[type])
    {
      fprintf(stderr, "Controller type %d is missing parameters!\n", type);
      exit(-1);
    }
  }
}

void controller_gpp_set_params(e_controller_type type)
{
  controller_params[C_TYPE_GPP] = controller_params[type];
}

int controller_get_min_refresh_period(e_controller_type type)
{
  return controller_params[type]->min_refresh_period;
}

int controller_get_default_refresh_period(e_controller_type type)
{
  return controller_params[type]->default_refresh_period;
}

inline int controller_get_max_unsigned(e_controller_type type, int axis)
{
  return controller_params[type]->max_unsigned_axis_value[axis];
}

inline int controller_get_max_signed(e_controller_type type, int axis)
{
  if(axis < abs_axis_0)
  {
    /*
     * relative axis
     */
    return controller_get_max_unsigned(type, axis) / 2 + 1;
  }
  else
  {
    /*
     * absolute axis
     */
    return controller_get_max_unsigned(type, axis);
  }
}

inline int controller_get_mean_unsigned(e_controller_type type, int axis)
{
  return controller_get_max_unsigned(type, axis) / 2 + 1;
}

inline double controller_get_axis_scale(e_controller_type type, int axis)
{
  return (double) controller_get_max_unsigned(type, axis) / DEFAULT_MAX_AXIS_VALUE;
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

    //the above values are kept for compatibility with old configurations

    {.name="rel_axis_0",     {.value=-1,  .index=rel_axis_0}},
    {.name="rel_axis_1",     {.value=-1,  .index=rel_axis_1}},
    {.name="rel_axis_2",     {.value=-1,  .index=rel_axis_2}},
    {.name="rel_axis_3",     {.value=-1,  .index=rel_axis_3}},
    {.name="rel_axis_4",     {.value=-1,  .index=rel_axis_4}},
    {.name="rel_axis_5",     {.value=-1,  .index=rel_axis_5}},
    {.name="rel_axis_6",     {.value=-1,  .index=rel_axis_6}},
    {.name="rel_axis_7",     {.value=-1,  .index=rel_axis_7}},

    {.name="rel_axis_0-",    {.value=-1,  .index=rel_axis_0}},
    {.name="rel_axis_1-",    {.value=-1,  .index=rel_axis_1}},
    {.name="rel_axis_2-",    {.value=-1,  .index=rel_axis_2}},
    {.name="rel_axis_3-",    {.value=-1,  .index=rel_axis_3}},
    {.name="rel_axis_4-",    {.value=-1,  .index=rel_axis_4}},
    {.name="rel_axis_5-",    {.value=-1,  .index=rel_axis_5}},
    {.name="rel_axis_6-",    {.value=-1,  .index=rel_axis_6}},
    {.name="rel_axis_7-",    {.value=-1,  .index=rel_axis_7}},

    {.name="rel_axis_0+",    {.value=1,  .index=rel_axis_0}},
    {.name="rel_axis_1+",    {.value=1,  .index=rel_axis_1}},
    {.name="rel_axis_2+",    {.value=1,  .index=rel_axis_2}},
    {.name="rel_axis_3+",    {.value=1,  .index=rel_axis_3}},
    {.name="rel_axis_4+",    {.value=1,  .index=rel_axis_4}},
    {.name="rel_axis_5+",    {.value=1,  .index=rel_axis_5}},
    {.name="rel_axis_6+",    {.value=1,  .index=rel_axis_6}},
    {.name="rel_axis_7+",    {.value=1,  .index=rel_axis_7}},

    {.name="abs_axis_0",     {.value=0,  .index=abs_axis_0}},
    {.name="abs_axis_1",     {.value=0,  .index=abs_axis_1}},
    {.name="abs_axis_2",     {.value=0,  .index=abs_axis_2}},
    {.name="abs_axis_3",     {.value=0,  .index=abs_axis_3}},
    {.name="abs_axis_4",     {.value=0,  .index=abs_axis_4}},
    {.name="abs_axis_5",     {.value=0,  .index=abs_axis_5}},
    {.name="abs_axis_6",     {.value=0,  .index=abs_axis_6}},
    {.name="abs_axis_7",     {.value=0,  .index=abs_axis_7}},
    {.name="abs_axis_8",     {.value=0,  .index=abs_axis_8}},
    {.name="abs_axis_9",     {.value=0,  .index=abs_axis_9}},
    {.name="abs_axis_10",    {.value=0,  .index=abs_axis_10}},
    {.name="abs_axis_11",    {.value=0,  .index=abs_axis_11}},
    {.name="abs_axis_12",    {.value=0,  .index=abs_axis_12}},
    {.name="abs_axis_13",    {.value=0,  .index=abs_axis_13}},
    {.name="abs_axis_14",    {.value=0,  .index=abs_axis_14}},
    {.name="abs_axis_15",    {.value=0,  .index=abs_axis_15}},
    {.name="abs_axis_16",    {.value=0,  .index=abs_axis_16}},
    {.name="abs_axis_17",    {.value=0,  .index=abs_axis_17}},

};

s_axis_index controller_get_axis_index_from_name(const char* name)
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
