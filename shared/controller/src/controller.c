/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* names[C_TYPE_MAX] =
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
    return names[type];
  }
  return names[C_TYPE_SIXAXIS];
}

e_controller_type controller_get_type(const char* name)
{
  int i;
  for(i=0; i<C_TYPE_MAX; ++i) {
    if(!strcmp(names[i], name)) {
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
    {.name = "rstick x",     {.index = sa_rstick_x, .dir = 0}},
    {.name = "rstick y",     {.index = sa_rstick_y, .dir = 0}},
    {.name = "lstick x",     {.index = sa_lstick_x, .dir = 0}},
    {.name = "lstick y",     {.index = sa_lstick_y, .dir = 0}},

    {.name = "rstick left",  {.index = sa_rstick_x, .dir = -1}},
    {.name = "rstick right", {.index = sa_rstick_x, .dir =  1}},
    {.name = "rstick up",    {.index = sa_rstick_y, .dir = -1}},
    {.name = "rstick down",  {.index = sa_rstick_y, .dir =  1}},

    {.name = "lstick left",  {.index = sa_lstick_x, .dir = -1}},
    {.name = "lstick right", {.index = sa_lstick_x, .dir =  1}},
    {.name = "lstick up",    {.index = sa_lstick_y, .dir = -1}},
    {.name = "lstick down",  {.index = sa_lstick_y, .dir =  1}},

    {.name = "acc x",        {.index = sa_acc_x,    .dir = 0}},
    {.name = "acc y",        {.index = sa_acc_y,    .dir = 0}},
    {.name = "acc z",        {.index = sa_acc_z,    .dir = 0}},
    {.name = "gyro",         {.index = sa_gyro,     .dir = 0}},

    {.name = "acc x -",      {.index = sa_acc_x,    .dir = -1}},
    {.name = "acc y -",      {.index = sa_acc_y,    .dir = -1}},
    {.name = "acc z -",      {.index = sa_acc_z,    .dir = -1}},
    {.name = "gyro -",       {.index = sa_gyro,     .dir = -1}},

    {.name = "acc x +",      {.index = sa_acc_x,    .dir = 1}},
    {.name = "acc y +",      {.index = sa_acc_y,    .dir = 1}},
    {.name = "acc z +",      {.index = sa_acc_z,    .dir = 1}},
    {.name = "gyro +",       {.index = sa_gyro,     .dir = 1}},

    {.name = "up",           {.index = sa_up,       .dir = 0}},
    {.name = "down",         {.index = sa_down,     .dir = 0}},
    {.name = "right",        {.index = sa_right,    .dir = 0}},
    {.name = "left",         {.index = sa_left,     .dir = 0}},
    {.name = "r1",           {.index = sa_r1,       .dir = 0}},
    {.name = "r2",           {.index = sa_r2,       .dir = 0}},
    {.name = "l1",           {.index = sa_l1,       .dir = 0}},
    {.name = "l2",           {.index = sa_l2,       .dir = 0}},
    {.name = "circle",       {.index = sa_circle,   .dir = 0}},
    {.name = "square",       {.index = sa_square,   .dir = 0}},
    {.name = "cross",        {.index = sa_cross,    .dir = 0}},
    {.name = "triangle",     {.index = sa_triangle, .dir = 0}},

    //the above values are kept for compatibility with old configurations

    {.name = "rel_axis_0",   {.index = rel_axis_0,  .dir = 0,}},
    {.name = "rel_axis_1",   {.index = rel_axis_1,  .dir = 0,}},
    {.name = "rel_axis_2",   {.index = rel_axis_2,  .dir = 0,}},
    {.name = "rel_axis_3",   {.index = rel_axis_3,  .dir = 0,}},
    {.name = "rel_axis_4",   {.index = rel_axis_4,  .dir = 0,}},
    {.name = "rel_axis_5",   {.index = rel_axis_5,  .dir = 0,}},
    {.name = "rel_axis_6",   {.index = rel_axis_6,  .dir = 0,}},
    {.name = "rel_axis_7",   {.index = rel_axis_7,  .dir = 0,}},

    {.name = "rel_axis_0-",  {.index = rel_axis_0, .dir = -1}},
    {.name = "rel_axis_1-",  {.index = rel_axis_1, .dir = -1}},
    {.name = "rel_axis_2-",  {.index = rel_axis_2, .dir = -1}},
    {.name = "rel_axis_3-",  {.index = rel_axis_3, .dir = -1}},
    {.name = "rel_axis_4-",  {.index = rel_axis_4, .dir = -1}},
    {.name = "rel_axis_5-",  {.index = rel_axis_5, .dir = -1}},
    {.name = "rel_axis_6-",  {.index = rel_axis_6, .dir = -1}},
    {.name = "rel_axis_7-",  {.index = rel_axis_7, .dir = -1}},

    {.name = "rel_axis_0+",  {.index = rel_axis_0, .dir = 1}},
    {.name = "rel_axis_1+",  {.index = rel_axis_1, .dir = 1}},
    {.name = "rel_axis_2+",  {.index = rel_axis_2, .dir = 1}},
    {.name = "rel_axis_3+",  {.index = rel_axis_3, .dir = 1}},
    {.name = "rel_axis_4+",  {.index = rel_axis_4, .dir = 1}},
    {.name = "rel_axis_5+",  {.index = rel_axis_5, .dir = 1}},
    {.name = "rel_axis_6+",  {.index = rel_axis_6, .dir = 1}},
    {.name = "rel_axis_7+",  {.index = rel_axis_7, .dir = 1}},

    {.name = "abs_axis_0",   {.index = abs_axis_0,  .dir = 0}},
    {.name = "abs_axis_1",   {.index = abs_axis_1,  .dir = 0}},
    {.name = "abs_axis_2",   {.index = abs_axis_2,  .dir = 0}},
    {.name = "abs_axis_3",   {.index = abs_axis_3,  .dir = 0}},
    {.name = "abs_axis_4",   {.index = abs_axis_4,  .dir = 0}},
    {.name = "abs_axis_5",   {.index = abs_axis_5,  .dir = 0}},
    {.name = "abs_axis_6",   {.index = abs_axis_6,  .dir = 0}},
    {.name = "abs_axis_7",   {.index = abs_axis_7,  .dir = 0}},
    {.name = "abs_axis_8",   {.index = abs_axis_8,  .dir = 0}},
    {.name = "abs_axis_9",   {.index = abs_axis_9,  .dir = 0}},
    {.name = "abs_axis_10",  {.index = abs_axis_10, .dir = 0}},
    {.name = "abs_axis_11",  {.index = abs_axis_11, .dir = 0}},
    {.name = "abs_axis_12",  {.index = abs_axis_12, .dir = 0}},
    {.name = "abs_axis_13",  {.index = abs_axis_13, .dir = 0}},
    {.name = "abs_axis_14",  {.index = abs_axis_14, .dir = 0}},
    {.name = "abs_axis_15",  {.index = abs_axis_15, .dir = 0}},
    {.name = "abs_axis_16",  {.index = abs_axis_16, .dir = 0}},
    {.name = "abs_axis_17",  {.index = abs_axis_17, .dir = 0}},

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
