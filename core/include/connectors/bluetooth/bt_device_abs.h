/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_DEVICE_ABS_H_
#define BT_DEVICE_ABS_H_

#ifndef WIN32
#include <bluetooth/bluetooth.h>
#else
#include <connectors/windows/bt_bdaddr.h>
#endif

#include <connectors/bluetooth/bt_abs.h>

typedef struct
{
  int (* init)();
  int (* get_bdaddr)(int device_number, bdaddr_t * bdaddr);
  int (* write_device_class)(int device_number, uint32_t devclass);
} s_bt_device_abs;

void bt_device_abs_register(e_bt_abs index, s_bt_device_abs * value);
s_bt_device_abs * bt_device_abs_get();

#endif /* BT_DEVICE_ABS_H_ */
