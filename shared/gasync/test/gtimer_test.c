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
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static volatile int done = 0;

static struct {
  unsigned int usec;
  int timer;
  struct timeval t0;
  struct timeval t1;
} timers[] = {
    { 1000, -1, {0, 0}, {0, 0} },
    { 2000, -1, {0, 0}, {0, 0} },
    { 3000, -1, {0, 0}, {0, 0} },
    { 4000, -1, {0, 0}, {0, 0} },
    { 5000, -1, {0, 0}, {0, 0} },
    { 6000, -1, {0, 0}, {0, 0} },
    { 7000, -1, {0, 0}, {0, 0} },
    { 8000, -1, {0, 0}, {0, 0} },
    { 9000, -1, {0, 0}, {0, 0} },
    { 10000, -1, {0, 0}, {0, 0} },
};

static void terminate(int sig) {

  done = 1;
}

static int timer_close_callback(int user) {
  done = 1;
  return 1;
}

static int timer_read_callback(int user) {

  gettimeofday(&timers[user].t1, NULL);

  struct timeval res;
  if(timercmp(&timers[user].t1, &timers[user].t0, >)) {
    timersub(&timers[user].t1, &timers[user].t0, &res);
  } else {
    timersub(&timers[user].t0, &timers[user].t1, &res);
  }
  unsigned int percent = (res.tv_sec * 1000000 + res.tv_usec) * 100 / timers[user].usec;
  if (percent <= 90 || percent >= 110) {
    printf("timer is off by more than 10 percent: period=%uµs, value=%u%%\n", timers[user].usec, percent);
  }

  struct timeval add = { .tv_sec = 0, .tv_usec = timers[user].usec };
  timeradd(&timers[user].t0, &add, &res);
  timers[user].t0 = res;

  return 1; // Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
}

int main(int argc, char* argv[]) {

  (void) signal(SIGINT, terminate);

#ifndef WIN32
  setlinebuf(stdout);
#endif

  gprio();

  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    timers[i].timer = gtimer_start(i, timers[i].usec, timer_read_callback, timer_close_callback, REGISTER_FUNCTION);
    if (timers[i].timer < 0) {
      done = 1;
      break;
    }
    gettimeofday(&timers[i].t0, NULL);
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
