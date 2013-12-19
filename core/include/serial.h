/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#ifndef WIN32
typedef int SERIALOBJECT;
#define SERIALOBJECT_UNDEF -1
#else
#include <windows.h>
typedef HANDLE SERIALOBJECT;
#define SERIALOBJECT_UNDEF NULL
#endif

SERIALOBJECT serial_connect(char* portname);
int serial_send(SERIALOBJECT so, void* pdata, unsigned int size);
int serial_recv(SERIALOBJECT so, void* pdata, unsigned int size);
void serial_close(SERIALOBJECT so);

#endif /* SERIAL_H_ */
