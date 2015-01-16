/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds3.h>
#include <report.h>
#include <controller2.h>
#include <string.h>

static const char *ds3_axis_name[AXIS_MAX] = {
    [sa_lstick_x] = "lstick x",
    [sa_lstick_y] = "lstick y",
    [sa_rstick_x] = "rstick x",
    [sa_rstick_y] = "rstick y",
    [sa_acc_x] = "acc x",
    [sa_acc_y] = "acc y",
    [sa_acc_z] = "acc z",
    [sa_gyro] = "gyro",
    [sa_select] = "select",
    [sa_start] = "start",
    [sa_ps] = "PS",
    [sa_up] = "up",
    [sa_right] = "right",
    [sa_down] = "down",
    [sa_left] = "left",
    [sa_triangle] = "triangle",
    [sa_circle] = "circle",
    [sa_cross] = "cross",
    [sa_square] = "square",
    [sa_l1] = "l1",
    [sa_r1] = "r1",
    [sa_l2] = "l2",
    [sa_r2] = "r2",
    [sa_l3] = "l3",
    [sa_r3] = "r3",
};

static s_axis_name_dir axis_names[] =
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

static int ds3_max_unsigned_axis_value[AXIS_MAX] =
{
  [sa_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [sa_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [sa_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [sa_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [sa_acc_x] = MAX_AXIS_VALUE_10BITS,
  [sa_acc_y] = MAX_AXIS_VALUE_10BITS,
  [sa_acc_z] = MAX_AXIS_VALUE_10BITS,
  [sa_gyro] = MAX_AXIS_VALUE_10BITS,
  [sa_select] = MAX_AXIS_VALUE_8BITS,
  [sa_start] = MAX_AXIS_VALUE_8BITS,
  [sa_ps] = MAX_AXIS_VALUE_8BITS,
  [sa_up] = MAX_AXIS_VALUE_8BITS,
  [sa_right] = MAX_AXIS_VALUE_8BITS,
  [sa_down] = MAX_AXIS_VALUE_8BITS,
  [sa_left] = MAX_AXIS_VALUE_8BITS,
  [sa_triangle] = MAX_AXIS_VALUE_8BITS,
  [sa_circle] = MAX_AXIS_VALUE_8BITS,
  [sa_cross] = MAX_AXIS_VALUE_8BITS,
  [sa_square] = MAX_AXIS_VALUE_8BITS,
  [sa_l1] = MAX_AXIS_VALUE_8BITS,
  [sa_r1] = MAX_AXIS_VALUE_8BITS,
  [sa_l2] = MAX_AXIS_VALUE_8BITS,
  [sa_r2] = MAX_AXIS_VALUE_8BITS,
  [sa_l3] = MAX_AXIS_VALUE_8BITS,
  [sa_r3] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params ds3_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 10000,
    .max_unsigned_axis_value = ds3_max_unsigned_axis_value
};

static unsigned int ds3_report_build(int axis[AXIS_MAX], s_report_packet* report)
{
  s_report_ds3* ds3 = &report->value.ds3;
  unsigned char buttons1 = 0x00;
  unsigned char buttons2 = 0x00;
  unsigned char buttons3 = 0x00;

  ds3->report_id = 0x01;

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

  unsigned short value;

  value = clamp(0, axis[sa_acc_x] + CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  ds3->acc_x[0] = value >> 8;
  ds3->acc_x[1] = value & 0xFF;
  value = clamp(0, axis[sa_acc_y] + CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  ds3->acc_y[0] = value >> 8;
  ds3->acc_y[1] = value & 0xFF;
  value = clamp(0, axis[sa_acc_z] + 400, MAX_AXIS_VALUE_10BITS);
  ds3->acc_z[0] = value >> 8;
  ds3->acc_z[1] = value & 0xFF;
  value = clamp(0, axis[sa_gyro] + CENTER_AXIS_VALUE_10BITS, MAX_AXIS_VALUE_10BITS);
  ds3->gyro[0] = value >> 8;
  ds3->gyro[1] = value & 0xFF;

  ds3->unknown2[3] = 0x03;//not charging
  ds3->unknown2[4] = 0x05;//fully charged
  ds3->unknown2[5] = 0x10;//cable plugged in, no rumble

  ds3->unknown2[10] = 0x02;
  ds3->unknown2[11] = 0xff;
  ds3->unknown2[12] = 0x77;
  ds3->unknown2[13] = 0x01;
  ds3->unknown2[14] = 0x80;//no rumble

  return sizeof(*ds3);
}

void ds3_init(void) __attribute__((constructor (101)));
void ds3_init(void)
{
  controller_register_axis_names(C_TYPE_SIXAXIS, sizeof(axis_names)/sizeof(*axis_names), axis_names);
  controller_register_axis_names(C_TYPE_DEFAULT, sizeof(axis_names)/sizeof(*axis_names), axis_names);
  controller_register_axis_names(C_TYPE_GPP, sizeof(axis_names)/sizeof(*axis_names), axis_names);

  controller_register_params(C_TYPE_SIXAXIS, &ds3_params);
  controller_register_params(C_TYPE_DEFAULT, &ds3_params);
  controller_register_params(C_TYPE_GPP, &ds3_params);

  control_register_names(C_TYPE_SIXAXIS, ds3_axis_name);
  control_register_names(C_TYPE_DEFAULT, ds3_axis_name);
  control_register_names(C_TYPE_GPP, ds3_axis_name);

  report_register_builder(C_TYPE_SIXAXIS, ds3_report_build);
  report_register_builder(C_TYPE_DEFAULT, ds3_report_build);
  report_register_builder(C_TYPE_GPP, ds3_report_build);
}
