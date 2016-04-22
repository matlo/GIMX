/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <connectors/bluetooth/bt_device_abs.h>
#include <connectors/bluetooth/bt_abs.h>
#include <stdlib.h>
#include <stdio.h>

static s_bt_device_abs * bt_device_abs[E_BT_ABS_MAX] = {};

s_bt_device_abs * bt_device_abs_get()
{
  return bt_device_abs[bt_abs_value];
}

void bt_device_abs_register(e_bt_abs index, s_bt_device_abs * value)
{
  bt_device_abs[index] = value;
}
