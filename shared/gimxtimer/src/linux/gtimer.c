/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gtimer.h>
#include <gimxcommon/include/gerror.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <inttypes.h>

#define MAX_TIMERS 32

static struct {
  int fd;
  int user;
  GPOLL_READ_CALLBACK fp_read;
  GPOLL_CLOSE_CALLBACK fp_close;
  GTIMER_REMOVE_SOURCE fp_remove;
} timers[MAX_TIMERS] = { };

#define CHECK_TIMER(TIMER,RETVALUE) \
  if (TIMER < 0 || TIMER >= MAX_TIMERS || timers[TIMER].fd < 0) { \
    PRINT_ERROR_OTHER("invalid timer") \
    return RETVALUE; \
  }

void gtimer_init(void) __attribute__((constructor));
void gtimer_init(void) {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    timers[i].fd = -1;
  }
}

void gtimer_clean(void) __attribute__((destructor));
void gtimer_clean(void) {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    if (timers[i].fd >= 0) {
      gtimer_close(i);
    }
  }
}

static int get_slot() {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    if (timers[i].fd < 0) {
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

  uint64_t nexp;
  ssize_t res;

  res = read(timers[timer].fd, &nexp, sizeof(nexp));
  if (res != sizeof(nexp)) {
    PRINT_ERROR_ERRNO("read")
    return -1;
  }
  if (nexp > 1) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      fprintf (stderr, "%ld.%06ld timer fired %" PRIu64 " times...\n", tv.tv_sec, tv.tv_usec, nexp);
  }

  return timers[timer].fp_read(timers[timer].user);
}

int gtimer_start(int user, unsigned int usec, const GTIMER_CALLBACKS * callbacks) {

  __time_t sec = usec / 1000000;
  __time_t nsec = (usec - sec * 1000000) * 1000;
  struct timespec period = { .tv_sec = sec, .tv_nsec = nsec };
  struct itimerspec new_value = { .it_interval = period, .it_value = period, };

  if (callbacks->fp_register == NULL)
  {
    PRINT_ERROR_OTHER("fp_register is NULL")
    return -1;
  }

  if (callbacks->fp_remove == NULL)
  {
    PRINT_ERROR_OTHER("fp_remove is NULL")
    return -1;
  }

  int slot = get_slot();
  if (slot < 0) {
    PRINT_ERROR_OTHER("no slot available")
    return -1;
  }

  int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (tfd < 0) {
    PRINT_ERROR_ERRNO("timerfd_create")
    return -1;
  }

  int ret = timerfd_settime(tfd, 0, &new_value, NULL);
  if (ret) {
    PRINT_ERROR_ERRNO("timerfd_settime")
    close(tfd);
    return -1;
  }

  GPOLL_CALLBACKS gpoll_callbacks = {
          .fp_read = read_callback,
          .fp_write = NULL,
          .fp_close = close_callback,
  };
  ret = callbacks->fp_register(tfd, slot, &gpoll_callbacks);
  if (ret < 0) {
    close(tfd);
    return -1;
  }

  timers[slot].fd = tfd;
  timers[slot].user = user;
  timers[slot].fp_read = callbacks->fp_read;
  timers[slot].fp_close = callbacks->fp_close;
  timers[slot].fp_remove = callbacks->fp_remove;

  return slot;
}

int gtimer_close(int timer) {

  CHECK_TIMER(timer, -1)

  timers[timer].fp_remove(timers[timer].fd);
  close(timers[timer].fd);
  memset(timers + timer, 0x00, sizeof(*timers));
  timers[timer].fd = -1;

  return 1;
}
