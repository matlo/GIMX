/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TIMERRES_H_
#define TIMERRES_H_

#include <windows.h>

typedef int (*HANDLE_CALLBACK)(int);
typedef int (*TIMERRES_REGISTER_HANDLE)(HANDLE handle, int id, HANDLE_CALLBACK fp_read, HANDLE_CALLBACK fp_write,
        HANDLE_CALLBACK fp_close);
typedef void (*TIMERRES_REMOVE_HANDLE)(HANDLE handle);
typedef int (*TIMERRES_CALLBACK)(unsigned int);

int timerres_begin(TIMERRES_REGISTER_HANDLE fp_register, TIMERRES_REMOVE_HANDLE fp_remove, TIMERRES_CALLBACK timer_cb);
void timerres_end();

#endif /* TIMERRES_H_ */
