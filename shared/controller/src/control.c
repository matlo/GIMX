/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <controller2.h>
#include <control.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

inline int clamp(int min, int val, int max)
{
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

static const char** control_names[C_TYPE_MAX] = {};

void control_init(void) __attribute__((constructor (102)));
void control_init(void)
{
  int type;
  for(type=0; type<C_TYPE_MAX; ++type)
  {
    if(!control_names[type])
    {
      fprintf(stderr, "Controller type %d is missing axis names!\n", type);
      exit(-1);
    }
  }
}

void control_register_names(e_controller_type type, const char** names)
{
  control_names[type] = names;
}

const char* control_get_name(e_controller_type type, e_controller_axis_index index)
{
  return control_names[type][index];
}

int control_get_index(const char* name)
{
  unsigned int axis;

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
      if(control_names[C_TYPE_SIXAXIS][i])
      {
        if(!strcmp(control_names[C_TYPE_SIXAXIS][i], name))
        {
          return i;
        }
      }
    }
  }

  return axis;
}

