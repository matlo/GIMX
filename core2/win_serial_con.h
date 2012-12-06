/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef WIN_SERIAL_CON_H_
#define WIN_SERIAL_CON_H_

#include "serial_con.h"

int win_serial_connect(char*);
void win_serial_send(void*, unsigned int);
void win_serial_close();

#endif /* WIN_SERIAL_H_ */
