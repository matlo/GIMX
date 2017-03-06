/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds3.h>
#include <report.h>
#include <string.h>
#include "../include/controller.h"

static s_axis axes[AXIS_MAX] = {
    [sa_lstick_x] = { .name = "lstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_lstick_y] = { .name = "lstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_rstick_x] = { .name = "rstick x", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_rstick_y] = { .name = "rstick y", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    
    [sa_acc_x] =    { .name = "acc x",    .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
    [sa_acc_y] =    { .name = "acc y",    .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
    [sa_acc_z] =    { .name = "acc z",    .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
    [sa_gyro] =     { .name = "gyro",     .max_unsigned_value = MAX_AXIS_VALUE_10BITS },
    
    [sa_select] =   { .name = "select",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_start] =    { .name = "start",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_ps] =       { .name = "PS",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_up] =       { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_right] =    { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_down] =     { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_left] =     { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_triangle] = { .name = "triangle", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_circle] =   { .name = "circle",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_cross] =    { .name = "cross",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_square] =   { .name = "square",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_l1] =       { .name = "l1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_r1] =       { .name = "r1",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_l2] =       { .name = "l2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_r2] =       { .name = "r2",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_l3] =       { .name = "l3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
    [sa_r3] =       { .name = "r3",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
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

  {.name = "acc x",        {.axis = sa_acc_x,    .props = AXIS_PROP_CENTERED}},
  {.name = "acc y",        {.axis = sa_acc_y,    .props = AXIS_PROP_CENTERED}},
  {.name = "acc z",        {.axis = sa_acc_z,    .props = AXIS_PROP_CENTERED}},
  {.name = "gyro",         {.axis = sa_gyro,     .props = AXIS_PROP_CENTERED}},

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

#define STORE_MOTION(value)

static s_report_ds3 default_report =
{
  .report_id = 0x01,
  .unused1 = 0x00,
  .buttons1 = 0x00,
  .buttons2 = 0x00,
  .buttons3 = 0x00,
  .unused2 = 0x00,
  .X = CENTER_AXIS_VALUE_8BITS,
  .Y = CENTER_AXIS_VALUE_8BITS,
  .Z = CENTER_AXIS_VALUE_8BITS,
  .Rz = CENTER_AXIS_VALUE_8BITS,
  .unknown1 = {},
  .up = 0x00,
  .right = 0x00,
  .down = 0x00,
  .left = 0x00,
  .l2 = 0x00,
  .r2 = 0x00,
  .l1 = 0x00,
  .r1 = 0x00,
  .triangle = 0x00,
  .circle = 0x00,
  .cross = 0x00,
  .square = 0x00,
  .unknown2 = {
    [3] = 0x03,//not charging
    [4] = 0x05,//fully charged
    [5] = 0x10,//cable plugged in, no rumble
    [10] = 0x02,
    [11] = 0xff,
    [12] = 0x77,
    [13] = 0x01,
    [14] = 0x80,//no rumble
  },
  .acc_x = { CENTER_AXIS_VALUE_10BITS >> 8, CENTER_AXIS_VALUE_10BITS & 0xff },
  .acc_y = { CENTER_AXIS_VALUE_10BITS >> 8, CENTER_AXIS_VALUE_10BITS & 0xff },
  .acc_z = { 400 >> 8, 400 & 0xff },
  .gyro = { CENTER_AXIS_VALUE_10BITS >> 8, CENTER_AXIS_VALUE_10BITS & 0xff },
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static inline void axis2axis(int from, unsigned char to[2], int center, int max)
{
  int value = clamp(0, from + center, max);
  to[0] = value >> 8;
  to[1] = value & 0xFF;
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_ds3);
  s_report_ds3* ds3 = &report[index].value.ds3;

  unsigned char buttons1 = 0x00;
  unsigned char buttons2 = 0x00;
  unsigned char buttons3 = 0x00;

  ds3->X = clamp(0, axis[sa_lstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds3->Y = clamp(0, axis[sa_lstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds3->Z = clamp(0, axis[sa_rstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds3->Rz = clamp(0, axis[sa_rstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);

  if(axis[sa_select])
  {
    buttons1 |= 0x01;
  }
  if(axis[sa_l3])
  {
    buttons1 |= 0x02;
  }
  if(axis[sa_r3])
  {
    buttons1 |= 0x04;
  }
  if(axis[sa_start])
  {
    buttons1 |= 0x08;
  }
  if(axis[sa_up])
  {
    buttons1 |= 0x10;
  }
  if(axis[sa_right])
  {
    buttons1 |= 0x20;
  }
  if(axis[sa_down])
  {
    buttons1 |= 0x40;
  }
  if(axis[sa_left])
  {
    buttons1 |= 0x80;
  }
  if(axis[sa_l2])
  {
    buttons2 |= 0x01;
  }
  if(axis[sa_r2])
  {
    buttons2 |= 0x02;
  }
  if(axis[sa_l1])
  {
    buttons2 |= 0x04;
  }
  if(axis[sa_r1])
  {
    buttons2 |= 0x08;
  }
  if(axis[sa_triangle])
  {
    buttons2 |= 0x10;
  }
  if(axis[sa_circle])
  {
    buttons2 |= 0x20;
  }
  if(axis[sa_cross])
  {
    buttons2 |= 0x40;
  }
  if(axis[sa_square])
  {
    buttons2 |= 0x80;
  }
  if(axis[sa_ps])
  {
    buttons3 |= 0x01;
  }

  ds3->buttons1 = buttons1;
  ds3->buttons2 = buttons2;
  ds3->buttons3 = buttons3;

  ds3->up = axis[sa_up];
  ds3->right = axis[sa_right];
  ds3->down = axis[sa_down];
  ds3->left = axis[sa_left];
  ds3->l2 = axis[sa_l2];
  ds3->r2 = axis[sa_r2];
  ds3->l1 = axis[sa_l1];
  ds3->r1 = axis[sa_r1];
  ds3->triangle = axis[sa_triangle];
  ds3->circle = axis[sa_circle];
  ds3->cross = axis[sa_cross];
  ds3->square = axis[sa_square];

  axis2axis(axis[sa_acc_x], ds3->acc_x, CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  axis2axis(axis[sa_acc_y], ds3->acc_y, CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  axis2axis(axis[sa_acc_z], ds3->acc_z, 400, MAX_AXIS_VALUE_10BITS);
  axis2axis(axis[sa_gyro], ds3->gyro, CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);

  return index;
}

static s_controller controller =
{
  .name = "Sixaxis",
  .vid = 0x054c,
  .pid = 0x0268,
  .refresh_period = { .min_value = 1000, .default_value = 10000 },
  .auth_required = 0,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
  .activation_button = "PS",
};

void ds3_init(void) __attribute__((constructor));
void ds3_init(void)
{
  controller_register(C_TYPE_SIXAXIS, &controller);
}
