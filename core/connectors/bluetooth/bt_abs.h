/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_ABS_H_
#define BT_ABS_H_

typedef enum
{
  E_BT_ABS_BTSTACK,
#ifndef WIN32
  E_BT_ABS_BLUEZ,
#endif
  E_BT_ABS_MAX,
} e_bt_abs;

#ifdef WIN32
#define DEFAULT_BT_ABS E_BT_ABS_BTSTACK
#else
#define DEFAULT_BT_ABS E_BT_ABS_BLUEZ
#endif

extern e_bt_abs bt_abs_value;

#endif /* BT_ABS_H_ */
