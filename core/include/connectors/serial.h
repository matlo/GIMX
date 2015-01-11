/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_H_
#define SERIAL_H_

int serial_connect(int id, char* portname);
int serial_send(int id, void* pdata, unsigned int size);
int serial_sendv(int id, void* pdata1, unsigned int size1, void* pdata2, unsigned int size2);
int serial_recv(int id, void* pdata, unsigned int size);
int serial_close(int id);

void serial_add_source(int id);

#endif /* SERIAL_H_ */
