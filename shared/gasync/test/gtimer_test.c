/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include <gprio.h>
#include "common.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

#ifdef WIN32
LARGE_INTEGER get_time() {

  FILETIME ftime;
  GetSystemTimeAsFileTime(&ftime);
  LARGE_INTEGER li = { .HighPart = ftime.dwHighDateTime, .LowPart = ftime.dwLowDateTime };
  return li;
}
#endif

static struct {
  unsigned int usec;
  int timer;
#ifndef WIN32
  struct timeval t0;
  struct timeval t1;
#else
  LARGE_INTEGER t0;
  LARGE_INTEGER t1;
#endif
  unsigned char tolerance;
  unsigned int sum;
  unsigned int count;
} timers[] = {
#ifndef WIN32
    { 1000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 2000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 3000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 4000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 5000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 6000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 7000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 8000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 9000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
    { 10000, -1, {0, 0}, {0, 0}, 10, 0, 0 },
#else
    { 1000, -1, {}, {}, 10, 0, 0 },
    { 2000, -1, {}, {}, 10, 0, 0 },
    { 3000, -1, {}, {}, 10, 0, 0 },
    { 4000, -1, {}, {}, 10, 0, 0 },
    { 5000, -1, {}, {}, 10, 0, 0 },
    { 6000, -1, {}, {}, 10, 0, 0 },
    { 7000, -1, {}, {}, 10, 0, 0 },
    { 8000, -1, {}, {}, 10, 0, 0 },
    { 9000, -1, {}, {}, 10, 0, 0 },
    { 10000, -1, {}, {}, 10, 0, 0 },
#endif
};

static int timer_close_callback(int user) {
  set_done();
  return 1;
}

#ifndef WIN32
static int timer_read_callback(int user) {

  gettimeofday(&timers[user].t1, NULL);

  struct timeval res;
  if(timercmp(&timers[user].t1, &timers[user].t0, <)) {
    fprintf(stderr, "error: timer fired too early\n");
    set_done();
    return -1;
  }

  timersub(&timers[user].t1, &timers[user].t0, &res);
  unsigned int diff = res.tv_sec * 1000000 + res.tv_usec;
  unsigned int percent = diff * 100 / timers[user].usec;
  if ((int)percent >= timers[user].tolerance) {
    printf("timer is off by more than %u percent: period=%uµs, error=%u%%\n", timers[user].tolerance, timers[user].usec, percent);
    fflush(stdout);
  }

  timers[user].sum += diff;
  ++timers[user].count;

  struct timeval add = { .tv_sec = 0, .tv_usec = timers[user].usec };
  timeradd(&timers[user].t0, &add, &res);
  timers[user].t0 = res;

  return 1; // Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
}
#else
/*
 * Timers on Windows are not so accurate and may drift.
 * => calculate the next timeout from now.
 */
static int timer_read_callback(int user) {

  timers[user].t1 = get_time();

  LONGLONG diff = (timers[user].t1.QuadPart - timers[user].t0.QuadPart) / 10;

  // Tolerate early firing:
  // on Windows the timer period is rounded to the highest multiple of the timer resolution not higher than the timer period.

  unsigned int percent = abs(diff) * 100 / timers[user].usec;
  if ((int)percent >= abs(100 - timers[user].tolerance)) {
    printf("timer is off by more than %u percent: period=%uus, error=%u%%\n", timers[user].tolerance, timers[user].usec, percent);
    fflush(stdout);
  }

  timers[user].sum += abs(diff);
  ++timers[user].count;

  timers[user].t1.QuadPart += (timers[user].usec * 10);
  timers[user].t0 = timers[user].t1;

  return 1; // Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
}
#endif

int main(int argc, char* argv[]) {

  setup_handlers();

  gprio();

  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {

#ifndef WIN32
    gettimeofday(&timers[i].t0, NULL);
    struct timeval res;
    struct timeval add = { .tv_sec = 0, .tv_usec = timers[i].usec };
    timeradd(&timers[i].t0, &add, &res);
    timers[i].t0 = res;
#else
    timers[i].t0 = get_time();
    timers[i].t0.QuadPart += (timers[i].usec * 10);
#endif

    timers[i].timer = gtimer_start(i, timers[i].usec, timer_read_callback, timer_close_callback, REGISTER_FUNCTION);
    if (timers[i].timer < 0) {
      set_done();
      break;
    }
  }

  while(!is_done()) {
    gpoll();
  }

  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    gtimer_close(timers[i].timer);
  }

  fprintf(stderr, "Exiting\n");

  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    if (timers[i].timer >= 0 && timers[i].usec > 0 && timers[i].count > 0) {
      printf("timer: %d, period: %uus, count=%u, error average: %u%%\n", timers[i].timer, timers[i].usec, timers[i].count, timers[i].sum / timers[i].count * 100 / timers[i].usec);
    }
  }

  return 0;
}
