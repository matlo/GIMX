/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* controller_names[C_TYPE_MAX] =
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
  if(type < C_TYPE_MAX)
  {
    return controller_names[type];
  }
  return controller_names[C_TYPE_SIXAXIS];
}

e_controller_type controller_get_type(const char* name)
{
  int i;
  for(i=0; i<C_TYPE_MAX; ++i) {
    if(!strcmp(controller_names[i], name)) {
      return i;
    }
  }
  return C_TYPE_SIXAXIS;
}

static s_controller_params* controller_params[C_TYPE_MAX] = {};

void controller_register_params(e_controller_type type, s_controller_params* params)
{
  controller_params[type] = params;
}

typedef struct
{
  int nb;
  s_axis_name_dir* axis_names;
} s_entry;

static s_entry controller_axis_names[C_TYPE_MAX] = {};

void controller_register_axis_names(e_controller_type type, int nb, s_axis_name_dir* axis_names)
{
  controller_axis_names[type].nb = nb;
  controller_axis_names[type].axis_names = axis_names;
}

void controller_init(void) __attribute__((constructor (102)));
void controller_init(void)
{
  int type;
  for(type=0; type<C_TYPE_MAX; ++type)
  {
    if(!controller_params[type])
    {
      fprintf(stderr, "Controller '%s' is missing parameters!\n", controller_names[type]);
      exit(-1);
    }
    if(!controller_axis_names[type].axis_names)
    {
      fprintf(stderr, "Controller '%s' is missing axis names!\n", controller_names[type]);
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

static const s_axis_name_dir axis_names[] =
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
  int i;
  s_axis_props none = {-1, AXIS_PROP_NONE};
  for(i=0; i<sizeof(axis_names)/sizeof(*axis_names); ++i)
  {
    if(!strcmp(axis_names[i].name, name))
    {
      return axis_names[i].axis_props;
    }
  }
  return none;
}

const char* controller_get_generic_axis_name_from_index(s_axis_props axis_props)
{
  int i;
  for(i=0; i<sizeof(axis_names)/sizeof(*axis_names); ++i)
  {
    if(axis_names[i].axis_props.axis == axis_props.axis
        && axis_names[i].axis_props.props == axis_props.props)
    {
      return axis_names[i].name;
    }
  }
  return "";
}

const char* controller_get_specific_axis_name_from_index(e_controller_type type, s_axis_props axis_props)
{
  int i;
  for(i=0; i<controller_axis_names[type].nb; ++i)
  {
    if(controller_axis_names[type].axis_names[i].axis_props.axis == axis_props.axis
        && controller_axis_names[type].axis_names[i].axis_props.props == axis_props.props)
    {
      return controller_axis_names[type].axis_names[i].name;
    }
  }
  return "";
}

s_axis_props controller_get_axis_index_from_specific_name(e_controller_type type, const char* name)
{
  int i;
  for(i=0; i<controller_axis_names[type].nb; ++i)
  {
    if(!strcmp(controller_axis_names[type].axis_names[i].name, name))
    {
      return controller_axis_names[type].axis_names[i].axis_props;
    }
  }
  return controller_get_axis_index_from_name(name);
}
