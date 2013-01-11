/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_CON_H_
#define SERIAL_CON_H_

#include <stdint.h>
#include "emuclient.h"

int serial_con_connect(char*);
void serial_con_send(e_controller_type, int);
void serial_con_close();

#endif /* SERIAL_CON_H_ */
