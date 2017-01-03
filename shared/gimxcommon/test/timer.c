/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <stddef.h>
#endif

static int timer_close(int user __attribute__((unused))) {
  done = 1;
  return 1;
}

static int timer_read(int user __attribute__((unused))) {
  /*
   * Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
   */
  return 1;
}

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
