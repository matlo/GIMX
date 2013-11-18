/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_H_
#define SERIAL_H_

int serial_connect(char* portname);
int serial_send(int fd, void* pdata, unsigned int size);
int serial_recv(int fd, void* pdata, unsigned int size);
void serial_close(int fd);

#endif /* SERIAL_H_ */
