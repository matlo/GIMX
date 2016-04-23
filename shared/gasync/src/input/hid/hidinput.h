/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HIDINPUT_H_
#define HIDINPUT_H_

#include <ginput.h>
#include <ghid.h>

typedef struct {
    unsigned short vendor;
    unsigned short product;
    const char * name;
} s_hidinput_ids;

typedef struct {
    s_hidinput_ids * ids;
    int (* init)(int(*callback)(GE_Event*));
    int (* probe)(int hid);
    int (* process)(int joystick, const void * report, unsigned int size, const void * prev);
} s_hidinput_driver;

int hidinput_register(s_hidinput_driver * driver);

int hidinput_init(int(*callback)(GE_Event*));
void hidinput_quit();

#endif /* HIDINPUT_H_ */
