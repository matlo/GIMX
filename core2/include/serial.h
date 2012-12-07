/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_H_
#define SERIAL_H_

int serial_connect(char*);
int serial_send(void*, unsigned int);
void serial_close();

#endif /* SERIAL_H_ */
