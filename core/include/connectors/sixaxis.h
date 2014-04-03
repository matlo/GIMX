/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SIXAXIS_H
#define SIXAXIS_H
 
#include <stdint.h>
#include <stdbool.h>
#include <ds3.h>

void sixaxis_set_bdaddr(int sixaxis_number, char* dst);
void sixaxis_set_dongle(int sixaxis_number, int dongle_index);
int sixaxis_connect(int sixaxis_number);
void sixaxis_close(int sixaxis_number);
int sixaxis_send_interrupt(int sixaxis_number, s_report_ds3* buf);

#endif
