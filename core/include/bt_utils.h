/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_UTILS_H_
#define BT_UTILS_H_

int bt_get_device_bdaddr(int device_number, char bdaddr[18]);
int bt_write_device_class(int device_number, uint32_t devclass);
int bt_disconnect(char bdaddr[18]);

#endif /* BT_UTILS_H_ */
