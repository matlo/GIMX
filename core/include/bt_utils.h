/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_UTILS_H_
#define BT_UTILS_H_

int get_device_bdaddr(int device_number, char bdaddr[18]);
int write_device_class(int dev_number, uint32_t devclass);

#endif /* BT_UTILS_H_ */
