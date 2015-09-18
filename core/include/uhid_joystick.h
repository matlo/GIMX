/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef UHID_JOYSTICK_H_
#define UHID_JOYSTICK_H_

int uhid_joystick_open_all();
int uhid_joystick_close_unused();
int uhid_joystick_close_all();
int uhid_joystick_get_hid_id(int uhid_id);

#endif /* UHID_JOYSTICK_H_ */
