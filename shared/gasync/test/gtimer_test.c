/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include <gprio.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

#ifdef WIN32
# define timeradd(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                          \
    if ((result)->tv_usec >= 1000000)                                         \
      {                                                                       \
        ++(result)->tv_sec;                                                   \
        (result)->tv_usec -= 1000000;                                         \
      }                                                                       \
  } while (0)
# define timersub(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#endif

static volatile int done = 0;

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
} timers[] = {
#ifndef WIN32
    { 1000, -1, {0, 0}, {0, 0}, 10 },
    { 2000, -1, {0, 0}, {0, 0}, 10 },
    { 3000, -1, {0, 0}, {0, 0}, 10 },
    { 4000, -1, {0, 0}, {0, 0}, 10 },
    { 5000, -1, {0, 0}, {0, 0}, 10 },
    { 6000, -1, {0, 0}, {0, 0}, 10 },
    { 7000, -1, {0, 0}, {0, 0}, 10 },
    { 8000, -1, {0, 0}, {0, 0}, 10 },
    { 9000, -1, {0, 0}, {0, 0}, 10 },
    { 10000, -1, {0, 0}, {0, 0}, 10 },
#else
    { 1000, -1, {}, {}, 50 },
    { 4000, -1, {}, {}, 25 },
    { 10000, -1, {}, {}, 10 },
#endif
};

static void terminate(int sig) {

  done = 1;
}

static int timer_close_callback(int user) {
  done = 1;
  return 1;
}

#ifndef WIN32
static int timer_read_callback(int user) {

  gettimeofday(&timers[user].t1, NULL);

  struct timeval res;
  if(timercmp(&timers[user].t1, &timers[user].t0, >)) {
    timersub(&timers[user].t1, &timers[user].t0, &res);
  } else {
    timersub(&timers[user].t0, &timers[user].t1, &res);
  }
  unsigned int percent = (res.tv_sec * 1000000 + res.tv_usec) * 100 / timers[user].usec;
  if (percent <= (100UL - timers[user].tolerance) || percent >= (100UL + timers[user].tolerance)) {
    printf("timer is off by more than %u percent: period=%uµs, value=%u%%\n", timers[user].tolerance, timers[user].usec, percent);
    fflush(stdout);
  }

  struct timeval add = { .tv_sec = 0, .tv_usec = timers[user].usec };
  timeradd(&timers[user].t0, &add, &res);
  timers[user].t0 = res;

  return 1; // Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
}
#else
/*
 * Timers on Windows are very inacurrate and drift.
 * => calculate the next timeout from now.
 */
static int timer_read_callback(int user) {

  FILETIME ftime;
  GetSystemTimeAsFileTime(&ftime);
  timers[user].t1.HighPart = ftime.dwHighDateTime;
  timers[user].t1.LowPart = ftime.dwLowDateTime;

  LONGLONG tdiff;
  if (timers[user].t1.QuadPart > timers[user].t0.QuadPart) {
    tdiff = timers[user].t1.QuadPart - timers[user].t0.QuadPart;
  } else {
    tdiff = timers[user].t0.QuadPart - timers[user].t1.QuadPart;
  }

  unsigned int percent = (timers[user].usec + tdiff / 10) * 100 / timers[user].usec;
  if (percent <= (100UL - timers[user].tolerance) || percent >= (100UL + timers[user].tolerance)) {
    printf("timer is off by more than %u percent: period=%uµs, value=%u%%\n", timers[user].tolerance, timers[user].usec, percent);
    fflush(stdout);
  }

  timers[user].t1.QuadPart += timers[user].usec * 10;
  timers[user].t0 = timers[user].t1;

  return 1; // Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
}
#endif

int main(int argc, char* argv[]) {

  (void) signal(SIGINT, terminate);

  gprio();

  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    timers[i].timer = gtimer_start(i, timers[i].usec, timer_read_callback, timer_close_callback, REGISTER_FUNCTION);
    if (timers[i].timer < 0) {
      done = 1;
      break;
    }
#ifndef WIN32
    gettimeofday(&timers[i].t0, NULL);
#else
    FILETIME ftime;
    GetSystemTimeAsFileTime(&ftime);
    timers[i].t0.HighPart = ftime.dwHighDateTime;
    timers[i].t0.LowPart = ftime.dwLowDateTime;
#endif
  }

  while(!done) {
    gpoll();
  }

  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    gtimer_close(timers[i].timer);
  }

  printf("Exiting\n");

  return 0;
}
