/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <ginput.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <stddef.h>
#endif

int is_done();
void set_done();

void setup_handlers();

void display_devices();
int process_event(GE_Event*);
int ignore_event(GE_Event*);

int timer_close(int user);
int timer_read(int user) ;

char * hid_select();

#ifndef WIN32
static inline unsigned long long int get_time() {

  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000000 + now.tv_usec;
}
#else
static inline unsigned long long int get_time() {

  FILETIME ftime;
  GetSystemTimeAsFileTime(&ftime);
  LARGE_INTEGER li = { .HighPart = ftime.dwHighDateTime, .LowPart = ftime.dwLowDateTime };
  return li.QuadPart / 10;
}
#endif

#endif /* COMMON_H_ */
