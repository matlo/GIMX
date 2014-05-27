/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef BTDS4_H
#define BTDS4_H
 
#include <stdint.h>
#include <stdbool.h>
#include <ds4.h>

void btds4_set_bdaddr(int btds4_number, char* dst);
void btds4_set_dongle(int btds4_number, int dongle_index);
int btds4_init(int btds4_number);
void btds4_close(int btds4_number);
int btds4_send_interrupt(int btds4_number, s_report_ds4* report);

#endif
