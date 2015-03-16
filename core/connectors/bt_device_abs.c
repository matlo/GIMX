/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <connectors/bluetooth/bt_device_abs.h>
#include <connectors/bluetooth/bt_abs.h>
#include <stdlib.h>

e_bt_abs bt_device_abs_value = DEFAULT_BT_ABS;

static s_bt_device_abs * bt_device_abs[E_BT_ABS_MAX] = {};

s_bt_device_abs * bt_device_abs_get()
{
  return bt_device_abs[bt_abs_value];
}

void bt_device_abs_register(e_bt_abs index, s_bt_device_abs * value)
{
  bt_device_abs[index] = value;
}

void bt_device_abs_init(void) __attribute__((constructor (102)));
void bt_device_abs_init(void)
{
  int index;
  for(index = 0; index < sizeof(bt_device_abs) / sizeof(*bt_device_abs); ++index)
  {
    if(!bt_device_abs[index])
    {
      fprintf(stderr, "bt_device_abs %d is missing!\n", index);
      exit(-1);
    }
  }
}
