/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ds4.h>
#include <report.h>
#include <controller.h>
#include <string.h>

static const char *ds4_axis_name[AXIS_MAX] =
{
  [ds4a_lstick_x] = "lstick x",
  [ds4a_lstick_y] = "lstick y",
  [ds4a_rstick_x] = "rstick x",
  [ds4a_rstick_y] = "rstick y",
  [ds4a_finger1_x] = "finger1 x",
  [ds4a_finger1_y] = "finger1 y",
  [ds4a_finger2_x] = "finger2 x",
  [ds4a_finger2_y] = "finger2 y",
  [ds4a_share] = "share",
  [ds4a_options] = "options",
  [ds4a_up] = "up",
  [ds4a_right] = "right",
  [ds4a_down] = "down",
  [ds4a_left] = "left",
  [ds4a_triangle] = "triangle",
  [ds4a_circle] = "circle",
  [ds4a_cross] = "cross",
  [ds4a_square] = "square",
  [ds4a_l1] = "l1",
  [ds4a_r1] = "r1",
  [ds4a_l2] = "l2",
  [ds4a_r2] = "r2",
  [ds4a_l3] = "l3",
  [ds4a_r3] = "r3",
  [ds4a_ps] = "PS",
  [ds4a_touchpad] = "touchpad",
  [ds4a_finger1] = "finger1",
  [ds4a_finger2] = "finger2",
};

#define TRACKPAD_MAX_X 1919
#define TRACKPAD_MAX_Y 919

static int ds4_max_unsigned_axis_value[AXIS_MAX] =
{
  [ds4a_lstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds4a_lstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds4a_rstick_x] = MAX_AXIS_VALUE_8BITS,
  [ds4a_rstick_y] = MAX_AXIS_VALUE_8BITS,
  [ds4a_finger1_x] = TRACKPAD_MAX_X,
  [ds4a_finger1_y] = TRACKPAD_MAX_Y,
  [ds4a_finger2_x] = TRACKPAD_MAX_X,
  [ds4a_finger2_y] = TRACKPAD_MAX_Y,
  [ds4a_up] = MAX_AXIS_VALUE_8BITS,
  [ds4a_right] = MAX_AXIS_VALUE_8BITS,
  [ds4a_down] = MAX_AXIS_VALUE_8BITS,
  [ds4a_left] = MAX_AXIS_VALUE_8BITS,
  [ds4a_square] = MAX_AXIS_VALUE_8BITS,
  [ds4a_cross] = MAX_AXIS_VALUE_8BITS,
  [ds4a_circle] = MAX_AXIS_VALUE_8BITS,
  [ds4a_triangle] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l1] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r1] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l2] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r2] = MAX_AXIS_VALUE_8BITS,
  [ds4a_share] = MAX_AXIS_VALUE_8BITS,
  [ds4a_options] = MAX_AXIS_VALUE_8BITS,
  [ds4a_l3] = MAX_AXIS_VALUE_8BITS,
  [ds4a_r3] = MAX_AXIS_VALUE_8BITS,
  [ds4a_ps] = MAX_AXIS_VALUE_8BITS,
  [ds4a_touchpad] = MAX_AXIS_VALUE_8BITS,
  [ds4a_finger1] = MAX_AXIS_VALUE_8BITS,
  [ds4a_finger2] = MAX_AXIS_VALUE_8BITS,
};

static s_controller_params ds4_params =
{
    .min_refresh_period = 1000,
    .default_refresh_period = 10000,
    .max_unsigned_axis_value = ds4_max_unsigned_axis_value
};

static s_report_ds4 init_report_ds4 =
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
  ._time = {0x99, 0x50},
  .battery_level = 0xff,
  .rel_gyro = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  .abs_gyro = {0xe7, 0xff, 0x6e, 0x20, 0xd9, 0x09},
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

void ds4_init_report(s_report_ds4* ds4)
{
  memcpy(ds4, &init_report_ds4, sizeof(s_report_ds4));
}

static inline void update_finger(s_trackpad_finger* finger, int presence, int axis_x, int axis_y)
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
      finger_x = TRACKPAD_MAX_X/2;
      finger_y = TRACKPAD_MAX_Y/2;
    }
    else
    {
      //continue movement from last position
      finger_x = ((coords[1] & 0x0F) << 8) | coords[0];
      finger_y = coords[2] << 4 | (coords[1]>>4);
    }

    //finger present
    finger->id &= 0x7F;

    int x = finger_x + axis_x;
    int y = finger_y + axis_y;

    finger_x = clamp(0, x, TRACKPAD_MAX_X);
    finger_y = clamp(0, y, TRACKPAD_MAX_Y);

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

/*
 * Work in progress...
 * Do not assume the code in the following function is right!
 */
static unsigned int ds4_report_build(int axis[AXIS_MAX], s_report* report)
{
  s_report_ds4* ds4 = &report->value.ds4;
  unsigned char counter;
  unsigned short buttons = 0x0000;

  ds4->X = clamp(0, axis[ds4a_lstick_x] + 128, MAX_AXIS_VALUE_8BITS);
  ds4->Y = clamp(0, axis[ds4a_lstick_y] + 128, MAX_AXIS_VALUE_8BITS);
  ds4->Z = clamp(0, axis[ds4a_rstick_x] + 128, MAX_AXIS_VALUE_8BITS);
  ds4->Rz = clamp(0, axis[ds4a_rstick_y] + 128, MAX_AXIS_VALUE_8BITS);

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
    ds4->HatAndButtons |= 0x10;
  }
  if (axis[ds4a_cross])
  {
    ds4->HatAndButtons |= 0x20;
  }
  if (axis[ds4a_circle])
  {
    ds4->HatAndButtons |= 0x40;
  }
  if (axis[ds4a_triangle])
  {
    ds4->HatAndButtons |= 0x80;
  }

  if (axis[ds4a_l1])
  {
    buttons |= 0x0001;
  }
  if (axis[ds4a_r1])
  {
    buttons |= 0x0002;
  }
  if (axis[ds4a_l2])
  {
    buttons |= 0x0004;
  }
  if (axis[ds4a_r2])
  {
    buttons |= 0x0008;
  }
  if (axis[ds4a_share])
  {
    buttons |= 0x0010;
  }
  if (axis[ds4a_options])
  {
    buttons |= 0x0020;
  }
  if (axis[ds4a_l3])
  {
    buttons |= 0x0040;
  }
  if (axis[ds4a_r3])
  {
    buttons |= 0x0080;
  }
  if (axis[ds4a_ps])
  {
    buttons |= 0x0100;
  }
  if (axis[ds4a_touchpad])
  {
    buttons |= 0x0200;
  }

  counter = (ds4->ButtonsAndCounter >> 8) & 0xFC;
  counter += 4;

  ds4->ButtonsAndCounter = (counter << 8) | buttons;

  ds4->Rx = clamp(0, axis[ds4a_l2], 255);
  ds4->Ry = clamp(0, axis[ds4a_r2], 255);

  ds4->packets = 1;

  if(axis[ds4a_finger1] || axis[ds4a_finger2])
  {
    ds4->packet1.counter += 4;
  }

  update_finger(&ds4->packet1.finger1, axis[ds4a_finger1], axis[ds4a_finger1_x], axis[ds4a_finger1_y]);

  update_finger(&ds4->packet1.finger2, axis[ds4a_finger2], axis[ds4a_finger2_x], axis[ds4a_finger2_y]);

  return sizeof(*ds4);
}

void ds4_init(void) __attribute__((constructor (101)));
void ds4_init(void)
{
  controller_register_params(C_TYPE_DS4, &ds4_params);

  control_register_names(C_TYPE_DS4, ds4_axis_name);

  report_register_builder(C_TYPE_DS4, ds4_report_build);
}
