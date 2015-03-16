/*
 * bt_abs.h
 *
 *  Created on: 14 mars 2015
 *      Author: matlo
 */

#ifndef BT_ABS_H_
#define BT_ABS_H_

typedef enum
{
  E_BT_ABS_BTSTACK,
  E_BT_ABS_BLUEZ,
  E_BT_ABS_MAX,
} e_bt_abs;

#ifdef WIN32
#define DEFAULT_BT_ABS E_BT_ABS_BTSTACK
#else
#define DEFAULT_BT_ABS E_BT_ABS_BLUEZ
#endif

extern e_bt_abs bt_abs_value;

#endif /* BT_ABS_H_ */
