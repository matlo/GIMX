/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef LIN_SERIAL_CON_H_
#define LIN_SERIAL_CON_H_

#include "serial_con.h"

int lin_serial_connect(char*);
int lin_serial_send(void*, unsigned int);
void lin_serial_close();

#endif /* LIN_SERIAL_CON_H_ */
