/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "../include/controller.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

s_controller * controllers[C_TYPE_MAX] = {};

int clamp(int min, int val, int max)
{
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

const char * controller_get_name(e_controller_type type)
{
  if(type < C_TYPE_MAX)
  {
    if (controllers[type] == NULL)
    {
      return "none";
    }
    return controllers[type]->name;
  }
  return "none";
}

e_controller_type controller_get_type(const char* name)
{
  int type;
  for(type = 0; type < C_TYPE_MAX; ++type) {
    if (controllers[type] == NULL)
    {
      continue;
    }
    if(!strcmp(controllers[type]->name, name)) {
      return type;
    }
  }
  return C_TYPE_NONE;
}

void controller_register(e_controller_type type, s_controller * controller)
{
  controllers[type] = controller;
}


int controller_get_min_refresh_period(e_controller_type type)
{
  if(type < C_TYPE_MAX)
  {
    return controllers[type]->refresh_period.min_value;
  }
  return DEFAULT_REFRESH_PERIOD;
}

int controller_get_default_refresh_period(e_controller_type type)
{
  if(type < C_TYPE_MAX)
  {
    return controllers[type]->refresh_period.default_value;
  }
  return DEFAULT_REFRESH_PERIOD;
}

int controller_get_max_unsigned(e_controller_type type, int axis)
{
  if(type < C_TYPE_MAX && axis < AXIS_MAX)
  {
    return controllers[type]->axes[axis].max_unsigned_value;
  }
  return DEFAULT_MAX_AXIS_VALUE;
}

int controller_get_max_signed(e_controller_type type, int axis)
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

int controller_get_mean_unsigned(e_controller_type type, int axis)
{
  return controller_get_max_unsigned(type, axis) / 2 + 1;
}

double controller_get_axis_scale(e_controller_type type, int axis)
{
  return (double) controller_get_max_unsigned(type, axis) / DEFAULT_MAX_AXIS_VALUE;
}

static const s_axis_name_dir name_dirs[] =
{
    {.name = "rel_axis_0",   {.axis = rel_axis_0,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_1",   {.axis = rel_axis_1,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_2",   {.axis = rel_axis_2,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_3",   {.axis = rel_axis_3,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_4",   {.axis = rel_axis_4,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_5",   {.axis = rel_axis_5,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_6",   {.axis = rel_axis_6,  .props = AXIS_PROP_CENTERED}},
    {.name = "rel_axis_7",   {.axis = rel_axis_7,  .props = AXIS_PROP_CENTERED}},

    {.name = "rel_axis_0-",  {.axis = rel_axis_0, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_1-",  {.axis = rel_axis_1, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_2-",  {.axis = rel_axis_2, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_3-",  {.axis = rel_axis_3, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_4-",  {.axis = rel_axis_4, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_5-",  {.axis = rel_axis_5, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_6-",  {.axis = rel_axis_6, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rel_axis_7-",  {.axis = rel_axis_7, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},

    {.name = "rel_axis_0+",  {.axis = rel_axis_0, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_1+",  {.axis = rel_axis_1, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_2+",  {.axis = rel_axis_2, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_3+",  {.axis = rel_axis_3, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_4+",  {.axis = rel_axis_4, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_5+",  {.axis = rel_axis_5, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_6+",  {.axis = rel_axis_6, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rel_axis_7+",  {.axis = rel_axis_7, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

    {.name = "abs_axis_0",   {.axis = abs_axis_0,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_1",   {.axis = abs_axis_1,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_2",   {.axis = abs_axis_2,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_3",   {.axis = abs_axis_3,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_4",   {.axis = abs_axis_4,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_5",   {.axis = abs_axis_5,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_6",   {.axis = abs_axis_6,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_7",   {.axis = abs_axis_7,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_8",   {.axis = abs_axis_8,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_9",   {.axis = abs_axis_9,  .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_10",  {.axis = abs_axis_10, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_11",  {.axis = abs_axis_11, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_12",  {.axis = abs_axis_12, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_13",  {.axis = abs_axis_13, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_14",  {.axis = abs_axis_14, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_15",  {.axis = abs_axis_15, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_16",  {.axis = abs_axis_16, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_17",  {.axis = abs_axis_17, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_18",  {.axis = abs_axis_18, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_19",  {.axis = abs_axis_19, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_20",  {.axis = abs_axis_20, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_21",  {.axis = abs_axis_21, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_22",  {.axis = abs_axis_22, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_23",  {.axis = abs_axis_23, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_24",  {.axis = abs_axis_24, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_25",  {.axis = abs_axis_25, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_26",  {.axis = abs_axis_26, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_27",  {.axis = abs_axis_27, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_28",  {.axis = abs_axis_28, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_29",  {.axis = abs_axis_29, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_30",  {.axis = abs_axis_30, .props = AXIS_PROP_POSITIVE}},
    {.name = "abs_axis_31",  {.axis = abs_axis_31, .props = AXIS_PROP_POSITIVE}},

    {.name = "abs_axis_0",   {.axis = abs_axis_0,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_1",   {.axis = abs_axis_1,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_2",   {.axis = abs_axis_2,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_3",   {.axis = abs_axis_3,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_4",   {.axis = abs_axis_4,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_5",   {.axis = abs_axis_5,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_6",   {.axis = abs_axis_6,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_7",   {.axis = abs_axis_7,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_8",   {.axis = abs_axis_8,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_9",   {.axis = abs_axis_9,  .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_10",  {.axis = abs_axis_10, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_11",  {.axis = abs_axis_11, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_12",  {.axis = abs_axis_12, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_13",  {.axis = abs_axis_13, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_14",  {.axis = abs_axis_14, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_15",  {.axis = abs_axis_15, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_16",  {.axis = abs_axis_16, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_17",  {.axis = abs_axis_17, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_18",  {.axis = abs_axis_18, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_19",  {.axis = abs_axis_19, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_20",  {.axis = abs_axis_20, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_21",  {.axis = abs_axis_21, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_22",  {.axis = abs_axis_22, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_23",  {.axis = abs_axis_23, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_24",  {.axis = abs_axis_24, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_25",  {.axis = abs_axis_25, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_26",  {.axis = abs_axis_26, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_27",  {.axis = abs_axis_27, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_28",  {.axis = abs_axis_28, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_29",  {.axis = abs_axis_29, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_30",  {.axis = abs_axis_30, .props = AXIS_PROP_TOGGLE}},
    {.name = "abs_axis_31",  {.axis = abs_axis_31, .props = AXIS_PROP_TOGGLE}},

    //the below values are kept for compatibility with old configurations

    {.name = "rstick x",     {.axis = sa_rstick_x, .props = AXIS_PROP_CENTERED}},
    {.name = "rstick y",     {.axis = sa_rstick_y, .props = AXIS_PROP_CENTERED}},
    {.name = "lstick x",     {.axis = sa_lstick_x, .props = AXIS_PROP_CENTERED}},
    {.name = "lstick y",     {.axis = sa_lstick_y, .props = AXIS_PROP_CENTERED}},

    {.name = "rstick left",  {.axis = sa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rstick right", {.axis = sa_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "rstick up",    {.axis = sa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "rstick down",  {.axis = sa_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

    {.name = "lstick left",  {.axis = sa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "lstick right", {.axis = sa_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "lstick up",    {.axis = sa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "lstick down",  {.axis = sa_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

    {.name = "acc x",        {.axis = sa_acc_x,    .props = AXIS_PROP_CENTERED | AXIS_PROP_CENTERED}},
    {.name = "acc y",        {.axis = sa_acc_y,    .props = AXIS_PROP_CENTERED | AXIS_PROP_CENTERED}},
    {.name = "acc z",        {.axis = sa_acc_z,    .props = AXIS_PROP_CENTERED | AXIS_PROP_CENTERED}},
    {.name = "gyro",         {.axis = sa_gyro,     .props = AXIS_PROP_CENTERED | AXIS_PROP_CENTERED}},

    {.name = "acc x -",      {.axis = sa_acc_x,    .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "acc y -",      {.axis = sa_acc_y,    .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "acc z -",      {.axis = sa_acc_z,    .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
    {.name = "gyro -",       {.axis = sa_gyro,     .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},

    {.name = "acc x +",      {.axis = sa_acc_x,    .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "acc y +",      {.axis = sa_acc_y,    .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "acc z +",      {.axis = sa_acc_z,    .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
    {.name = "gyro +",       {.axis = sa_gyro,     .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

    {.name = "up",           {.axis = sa_up,       .props = AXIS_PROP_POSITIVE}},
    {.name = "down",         {.axis = sa_down,     .props = AXIS_PROP_POSITIVE}},
    {.name = "right",        {.axis = sa_right,    .props = AXIS_PROP_POSITIVE}},
    {.name = "left",         {.axis = sa_left,     .props = AXIS_PROP_POSITIVE}},
    {.name = "r1",           {.axis = sa_r1,       .props = AXIS_PROP_POSITIVE}},
    {.name = "r2",           {.axis = sa_r2,       .props = AXIS_PROP_POSITIVE}},
    {.name = "l1",           {.axis = sa_l1,       .props = AXIS_PROP_POSITIVE}},
    {.name = "l2",           {.axis = sa_l2,       .props = AXIS_PROP_POSITIVE}},
    {.name = "circle",       {.axis = sa_circle,   .props = AXIS_PROP_POSITIVE}},
    {.name = "square",       {.axis = sa_square,   .props = AXIS_PROP_POSITIVE}},
    {.name = "cross",        {.axis = sa_cross,    .props = AXIS_PROP_POSITIVE}},
    {.name = "triangle",     {.axis = sa_triangle, .props = AXIS_PROP_POSITIVE}},

    {.name = "select",       {.axis = sa_select,   .props = AXIS_PROP_TOGGLE}},
    {.name = "start",        {.axis = sa_start,    .props = AXIS_PROP_TOGGLE}},
    {.name = "PS",           {.axis = sa_ps,       .props = AXIS_PROP_TOGGLE}},
    {.name = "r3",           {.axis = sa_r3,       .props = AXIS_PROP_TOGGLE}},
    {.name = "l3",           {.axis = sa_l3,       .props = AXIS_PROP_TOGGLE}},
};

s_axis_props controller_get_axis_index_from_name(const char* name)
{
  unsigned int i;
  s_axis_props none = {-1, AXIS_PROP_NONE};
  for(i=0; i<sizeof(name_dirs)/sizeof(*name_dirs); ++i)
  {
    if(!strcmp(name_dirs[i].name, name))
    {
      return name_dirs[i].axis_props;
    }
  }
  return none;
}

const char* controller_get_generic_axis_name_from_index(s_axis_props axis_props)
{
  unsigned int i;
  for(i=0; i<sizeof(name_dirs)/sizeof(*name_dirs); ++i)
  {
    if(name_dirs[i].axis_props.axis == axis_props.axis
        && name_dirs[i].axis_props.props == axis_props.props)
    {
      return name_dirs[i].name;
    }
  }
  return "";
}

const char* controller_get_specific_axis_name_from_index(e_controller_type type, s_axis_props axis_props)
{
  if(type < C_TYPE_MAX)
  {
    int i;
    for(i=0; i<controllers[type]->axis_name_dirs.nb; ++i)
    {
      if(controllers[type]->axis_name_dirs.values[i].axis_props.axis == axis_props.axis
          && controllers[type]->axis_name_dirs.values[i].axis_props.props == axis_props.props)
      {
        return controllers[type]->axis_name_dirs.values[i].name;
      }
    }
  }
  return "";
}

s_axis_props controller_get_axis_index_from_specific_name(e_controller_type type, const char* name)
{
  if(type < C_TYPE_MAX)
  {
    int i;
    for(i=0; i<controllers[type]->axis_name_dirs.nb; ++i)
    {
      if(!strcmp(controllers[type]->axis_name_dirs.values[i].name, name))
      {
        return controllers[type]->axis_name_dirs.values[i].axis_props;
      }
    }
  }
  return controller_get_axis_index_from_name(name);
}

unsigned int controller_build_report(e_controller_type type, int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  if(type < C_TYPE_MAX)
  {
    return controllers[type]->fp_build_report(axis, report);
  }
  return 0;
}

void controller_init_report(e_controller_type type, s_report * report)
{
  if(type < C_TYPE_MAX)
  {
    controllers[type]->fp_init_report(report);
  }
}

const char* controller_get_axis_name(e_controller_type type, e_controller_axis_index index)
{
  return controllers[type]->axes[index].name;
}

int controller_get_axis_index(const char* name)
{
  unsigned int axis = -1;

  if(sscanf(name, "rel_axis_%u", &axis) == 1)
  {
    if(axis > rel_axis_max)
    {
      return -1;
    }
  }
  else if(sscanf(name, "abs_axis_%u", &axis) == 1)
  {
    axis += abs_axis_0;

    if(axis > abs_axis_max)
    {
      return -1;
    }
  }
  else
  {
    //handle old configs
    int i;
    for(i=0; i<AXIS_MAX; ++i)
    {
      if(controllers[C_TYPE_SIXAXIS]->axes[i].name)
      {
        if(!strcmp(controllers[C_TYPE_SIXAXIS]->axes[i].name, name))
        {
          return i;
        }
      }
    }
  }

  return axis;
}

int controller_is_auth_required(e_controller_type type)
{
  return controllers[type]->auth_required;
}

void controller_get_ids(e_controller_type type, unsigned short * vid, unsigned short * pid)
{
  if(type < C_TYPE_MAX)
  {
    *vid = controllers[type]->vid;
    *pid = controllers[type]->pid;
  }
}
