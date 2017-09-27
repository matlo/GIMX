/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds4.h>
#include <report.h>
#include <string.h>
#include "../include/controller.h"

static s_axis axes[AXIS_MAX] =
{
  [ds4a_lstick_x] =  { .name = "lstick x",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_lstick_y] =  { .name = "lstick y",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_rstick_x] =  { .name = "rstick x",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_rstick_y] =  { .name = "rstick y",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  
  [ds4a_finger1_x] = { .name = "finger1 x", .max_unsigned_value = DS4_TRACKPAD_MAX_X },
  [ds4a_finger1_y] = { .name = "finger1 y", .max_unsigned_value = DS4_TRACKPAD_MAX_Y },
  [ds4a_finger2_x] = { .name = "finger2 x", .max_unsigned_value = DS4_TRACKPAD_MAX_X },
  [ds4a_finger2_y] = { .name = "finger2 y", .max_unsigned_value = DS4_TRACKPAD_MAX_Y },
  
  [ds4a_share] =     { .name = "share",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_options] =   { .name = "options",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_up] =        { .name = "up",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_right] =     { .name = "right",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_down] =      { .name = "down",      .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_left] =      { .name = "left",      .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_triangle] =  { .name = "triangle",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_circle] =    { .name = "circle",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_cross] =     { .name = "cross",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_square] =    { .name = "square",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_l1] =        { .name = "l1",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_r1] =        { .name = "r1",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_l2] =        { .name = "l2",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_r2] =        { .name = "r2",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_l3] =        { .name = "l3",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_r3] =        { .name = "r3",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_ps] =        { .name = "PS",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_touchpad] =  { .name = "touchpad",  .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_finger1] =   { .name = "finger1",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
  [ds4a_finger2] =   { .name = "finger2",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] =
{
  {.name = "rstick x",     {.axis = ds4a_rstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "rstick y",     {.axis = ds4a_rstick_y, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick x",     {.axis = ds4a_lstick_x, .props = AXIS_PROP_CENTERED}},
  {.name = "lstick y",     {.axis = ds4a_lstick_y, .props = AXIS_PROP_CENTERED}},

  {.name = "rstick left",  {.axis = ds4a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick right", {.axis = ds4a_rstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "rstick up",    {.axis = ds4a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "rstick down",  {.axis = ds4a_rstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "lstick left",  {.axis = ds4a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick right", {.axis = ds4a_lstick_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "lstick up",    {.axis = ds4a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "lstick down",  {.axis = ds4a_lstick_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "finger1 x",    {.axis = ds4a_finger1_x, .props = AXIS_PROP_CENTERED,}},
  {.name = "finger1 y",    {.axis = ds4a_finger1_y, .props = AXIS_PROP_CENTERED,}},
  {.name = "finger2 x",    {.axis = ds4a_finger2_x, .props = AXIS_PROP_CENTERED,}},
  {.name = "finger2 y",    {.axis = ds4a_finger2_y, .props = AXIS_PROP_CENTERED,}},

  {.name = "finger1 x -",  {.axis = ds4a_finger1_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "finger1 y -",  {.axis = ds4a_finger1_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "finger2 x -",  {.axis = ds4a_finger2_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},
  {.name = "finger2 y -",  {.axis = ds4a_finger2_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_NEGATIVE}},

  {.name = "finger1 x +",  {.axis = ds4a_finger1_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "finger1 y +",  {.axis = ds4a_finger1_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "finger2 x +",  {.axis = ds4a_finger2_x, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},
  {.name = "finger2 y +",  {.axis = ds4a_finger2_y, .props = AXIS_PROP_CENTERED | AXIS_PROP_POSITIVE}},

  {.name = "r2",           {.axis = ds4a_r2,       .props = AXIS_PROP_POSITIVE}},
  {.name = "l2",           {.axis = ds4a_l2,       .props = AXIS_PROP_POSITIVE}},

  {.name = "share",        {.axis = ds4a_share,    .props = AXIS_PROP_TOGGLE}},
  {.name = "options",      {.axis = ds4a_options,  .props = AXIS_PROP_TOGGLE}},
  {.name = "PS",           {.axis = ds4a_ps,       .props = AXIS_PROP_TOGGLE}},
  {.name = "up",           {.axis = ds4a_up,       .props = AXIS_PROP_TOGGLE}},
  {.name = "down",         {.axis = ds4a_down,     .props = AXIS_PROP_TOGGLE}},
  {.name = "right",        {.axis = ds4a_right,    .props = AXIS_PROP_TOGGLE}},
  {.name = "left",         {.axis = ds4a_left,     .props = AXIS_PROP_TOGGLE}},
  {.name = "r1",           {.axis = ds4a_r1,       .props = AXIS_PROP_TOGGLE}},
  {.name = "r3",           {.axis = ds4a_r3,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l1",           {.axis = ds4a_l1,       .props = AXIS_PROP_TOGGLE}},
  {.name = "l3",           {.axis = ds4a_l3,       .props = AXIS_PROP_TOGGLE}},
  {.name = "circle",       {.axis = ds4a_circle,   .props = AXIS_PROP_TOGGLE}},
  {.name = "square",       {.axis = ds4a_square,   .props = AXIS_PROP_TOGGLE}},
  {.name = "cross",        {.axis = ds4a_cross,    .props = AXIS_PROP_TOGGLE}},
  {.name = "triangle",     {.axis = ds4a_triangle, .props = AXIS_PROP_TOGGLE}},
  {.name = "touchpad",     {.axis = ds4a_touchpad, .props = AXIS_PROP_TOGGLE}},
  {.name = "finger1",      {.axis = ds4a_finger1,  .props = AXIS_PROP_TOGGLE}},
  {.name = "finger2",      {.axis = ds4a_finger2,  .props = AXIS_PROP_TOGGLE}},
};

static s_report_ds4 default_report =
{
  .report_id = 0x00,
  .X = 0x80,
  .Y = 0x80,
  .Z = 0x80,
  .Rz = 0x80,
  .HatAndButtons = 0x08,
  .ButtonsAndCounter = 0xfc00,
  .Rx = 0x00,
  .Ry = 0x00,
  ._time = 0x5099,
  .battery_level = 0xff,
  .motion_acc = {0x0006, 0x0001, 0x350f},
  .motion_gyro = {0x1903,  0x0001, 0x0008},
  ._unknown1 = {0x00, 0x00, 0x00, 0x00, 0x00},
  .ext = 0x08,
  ._unknown2 = {0x00, 0x00},
  .packets = 0x00,
  .packet1 =
  {
    .counter = 0x00,
    .finger1 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
    .finger2 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
  },
  .packet2 =
  {
    .counter = 0x00,
    .finger1 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
    .finger2 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
  },
  .packet3 =
  {
    .counter = 0x00,
    .finger1 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
    .finger2 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
  },
  .packet4 =
  {
    .counter = 0x00,
    .finger1 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
    .finger2 =
    {
      .id = 0x80,
      .coords = {0x00, 0x00, 0x00}
    },
  },
  ._unknown5 = {0x00, 0x00},
};

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

/*
 * Update touchpad finger.
 * The axes are relative to the last position.
 */
static inline void update_finger(s_trackpad_finger* finger, int presence, int* axis_x, int* axis_y)
{
  unsigned char* coords = finger->coords;

  unsigned short finger_x;
  unsigned short finger_y;

  if(presence)
  {
    if(finger->id & 0x80)
    {
      //increase the finger id in case of new touch
      finger->id++;

      //start movement from center
      finger_x = DS4_TRACKPAD_MAX_X/2;
      finger_y = DS4_TRACKPAD_MAX_Y/2;
    }
    else
    {
      //continue movement from last position
      finger_x = ((coords[1] & 0x0F) << 8) | coords[0];
      finger_y = coords[2] << 4 | (coords[1]>>4);
    }

    //finger present
    finger->id &= 0x7F;

    int x = finger_x + *axis_x;
    int y = finger_y + *axis_y;

    finger_x = clamp(0, x, DS4_TRACKPAD_MAX_X);
    finger_y = clamp(0, y, DS4_TRACKPAD_MAX_Y);

    coords[0] = finger_x & 0xFF;
    coords[1] =  ((finger_x >> 8) & 0x0F) | ((finger_y & 0x0F) << 4);
    coords[2] =  finger_y >> 4;
  }
  else
  {
    //finger absent
    finger->id |= 0x80;
  }
}

static unsigned int build_report(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS])
{
  unsigned int index = 0;
  report[index].length = sizeof(s_report_ds4);
  s_report_ds4* ds4 = &report[index].value.ds4;

  unsigned char counter;
  unsigned short buttons = 0x0000;

  ds4->X = clamp(0, axis[ds4a_lstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds4->Y = clamp(0, axis[ds4a_lstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds4->Z = clamp(0, axis[ds4a_rstick_x] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);
  ds4->Rz = clamp(0, axis[ds4a_rstick_y] + CENTER_AXIS_VALUE_8BITS, MAX_AXIS_VALUE_8BITS);

  if (axis[ds4a_right])
  {
    if (axis[ds4a_down])
    {
      ds4->HatAndButtons = 0x03;
    }
    else if (axis[ds4a_up])
    {
      ds4->HatAndButtons = 0x01;
    }
    else
    {
      ds4->HatAndButtons = 0x02;
    }
  }
  else if (axis[ds4a_left])
  {
    if (axis[ds4a_down])
    {
      ds4->HatAndButtons = 0x05;
    }
    else if (axis[ds4a_up])
    {
      ds4->HatAndButtons = 0x07;
    }
    else
    {
      ds4->HatAndButtons = 0x06;
    }
  }
  else if (axis[ds4a_down])
  {
    ds4->HatAndButtons = 0x04;
  }
  else if (axis[ds4a_up])
  {
    ds4->HatAndButtons = 0x00;
  }
  else
  {
    ds4->HatAndButtons = 0x08;
  }

  if (axis[ds4a_square])
  {
    ds4->HatAndButtons |= DS4_SQUARE_MASK;
  }
  if (axis[ds4a_cross])
  {
    ds4->HatAndButtons |= DS4_CROSS_MASK;
  }
  if (axis[ds4a_circle])
  {
    ds4->HatAndButtons |= DS4_CIRCLE_MASK;
  }
  if (axis[ds4a_triangle])
  {
    ds4->HatAndButtons |= DS4_TRIANGLE_MASK;
  }

  if (axis[ds4a_l1])
  {
    buttons |= DS4_L1_MASK;
  }
  if (axis[ds4a_r1])
  {
    buttons |= DS4_R1_MASK;
  }
  if (axis[ds4a_l2])
  {
    buttons |= DS4_L2_MASK;
  }
  if (axis[ds4a_r2])
  {
    buttons |= DS4_R2_MASK;
  }
  if (axis[ds4a_share])
  {
    buttons |= DS4_SHARE_MASK;
  }
  if (axis[ds4a_options])
  {
    buttons |= DS4_OPTIONS_MASK;
  }
  if (axis[ds4a_l3])
  {
    buttons |= DS4_L3_MASK;
  }
  if (axis[ds4a_r3])
  {
    buttons |= DS4_R3_MASK;
  }
  if (axis[ds4a_ps])
  {
    buttons |= DS4_PS_MASK;
  }
  if (axis[ds4a_touchpad])
  {
    buttons |= DS4_TOUCHPAD_MASK;
  }

  counter = (ds4->ButtonsAndCounter >> 8) & 0xFC;
  counter += 4;

  ds4->ButtonsAndCounter = (counter << 8) | buttons;

  ds4->Rx = clamp(0, axis[ds4a_l2], MAX_AXIS_VALUE_8BITS);
  ds4->Ry = clamp(0, axis[ds4a_r2], MAX_AXIS_VALUE_8BITS);

  ds4->packets = 1;

  if(axis[ds4a_finger1] || axis[ds4a_finger2])
  {
    ds4->packet1.counter += 4;
  }

  update_finger(&ds4->packet1.finger1, axis[ds4a_finger1], &axis[ds4a_finger1_x], &axis[ds4a_finger1_y]);

  update_finger(&ds4->packet1.finger2, axis[ds4a_finger2], &axis[ds4a_finger2_x], &axis[ds4a_finger2_y]);

  return index;
}

static s_controller controller =
{
  .name = "DS4",
  .vid = DS4_VENDOR,
  .pid = DS4_PRODUCT,
  .refresh_period = { .min_value = 1000, .default_value = 10000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = build_report,
  .fp_init_report = init_report,
  .activation_button = ds4a_ps,
};

void ds4_init(void) __attribute__((constructor));
void ds4_init(void)
{
  controller_register(C_TYPE_DS4, &controller);
}

