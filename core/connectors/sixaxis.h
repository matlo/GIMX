/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SIXAXIS_H
#define SIXAXIS_H
 
#include <stdint.h>
#include <stdbool.h>
#include <gimxcontroller/include/ds3.h>

int sixaxis_connect(int sixaxis_number, int dongle_index, const char * bdaddr_dst);
void sixaxis_close(int sixaxis_number);
int sixaxis_send_interrupt(int sixaxis_number, s_report_ds3* buf);

#endif
