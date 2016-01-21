/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gtimer.h>
#include <gerror.h>

#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_TIMERS 32

static struct {
  HANDLE handle;
  int user;
  int (*fp_read)(int);
  int (*fp_close)(int);
} timers[MAX_TIMERS] = { };

static unsigned int nb_timers = 0;

#define CHECK_TIMER(TIMER,RETVALUE) \
  if (TIMER < 0 || TIMER >= MAX_TIMERS || timers[TIMER].handle == INVALID_HANDLE_VALUE) { \
    PRINT_ERROR_OTHER("invalid timer") \
    return RETVALUE; \
  }

void gtimer_init(void) __attribute__((constructor (101)));
void gtimer_init(void) {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    timers[i].handle = INVALID_HANDLE_VALUE;
  }
}

static int get_slot() {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    if (timers[i].handle == INVALID_HANDLE_VALUE) {
      return i;
    }
  }
  return -1;
}

static int close_callback(int timer) {

  CHECK_TIMER(timer, -1)

  return timers[timer].fp_close(timers[timer].user);
}

static int read_callback(int timer) {

  CHECK_TIMER(timer, -1)

  return timers[timer].fp_read(timers[timer].user);
}

int gtimer_start(int user, int usec, GPOLL_READ_CALLBACK fp_read, GPOLL_CLOSE_CALLBACK fp_close,
    GPOLL_REGISTER_HANDLE fp_register) {

  int slot = get_slot();
  if (slot < 0) {
    PRINT_ERROR_OTHER("no slot available")
    return -1;
  }

  if (nb_timers == 0) {
    timeBeginPeriod(1);
  }

  HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  if (hTimer != INVALID_HANDLE_VALUE) {
    LONG period = usec / 1000;
    if (period * 1000 != usec) {
      PRINT_ERROR_OTHER("timer accuracy is only 1ms on Windows")
    }
    if (period == 0) {
      period = 1;
    }
    LARGE_INTEGER li = { .QuadPart = -(period * 10000) };
    if (!SetWaitableTimer(hTimer, &li, period, NULL, NULL, FALSE)) {
      PRINT_ERROR_GETLASTERROR("SetWaitableTimer")
      CloseHandle(hTimer);
      hTimer = INVALID_HANDLE_VALUE;
    }
  } else {
    PRINT_ERROR_GETLASTERROR("CreateWaitableTimer")
  }

  int ret = fp_register(hTimer, slot, read_callback, NULL, close_callback);
  if (ret < 0) {
    CloseHandle(hTimer);
    hTimer = INVALID_HANDLE_VALUE;
  }

  if (hTimer != INVALID_HANDLE_VALUE) {
    ++nb_timers;
    timers[slot].handle = hTimer;
    timers[slot].user = user;
    timers[slot].fp_read = fp_read;
    timers[slot].fp_close = fp_close;
  } else {
    slot = -1;
  }

  if(nb_timers == 0) {
    timeEndPeriod(0);
  }

  return slot;
}

int gtimer_close(int timer) {

  CHECK_TIMER(timer, -1)

  gpoll_remove_handle(timers[timer].handle);
  CloseHandle(timers[timer].handle);
  memset(timers + timer, 0x00, sizeof(*timers));
  timers[timer].handle = INVALID_HANDLE_VALUE;

  --nb_timers;
  if(nb_timers == 0) {
    timeEndPeriod(0);
  }

  return 1;
}
