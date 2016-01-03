/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gtimer.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);
#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define MAX_TIMERS 32

static struct {
  int fd;
  int user;
  int (*fp_read)(int);
  int (*fp_close)(int);
} timers[MAX_TIMERS] = { };

#define CHECK_TIMER(TIMER,RETVALUE) \
  if (TIMER < 0 || TIMER >= MAX_TIMERS || timers[TIMER].fd < 0) { \
    PRINT_ERROR_OTHER("invalid timer") \
    return RETVALUE; \
  }

void gtimer_init(void) __attribute__((constructor (101)));
void gtimer_init(void) {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    timers[i].fd = -1;
  }
}

static int get_slot() {
  unsigned int i;
  for (i = 0; i < sizeof(timers) / sizeof(*timers); ++i) {
    if (timers[i].fd == -1) {
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
    PRINT_ERROR_OTHER("timer fired several times...")
  }

  return timers[timer].fp_read(timers[timer].user);
}

int gtimer_start(int user, int usec, GPOLL_READ_CALLBACK fp_read, GPOLL_CLOSE_CALLBACK fp_close,
    GPOLL_REGISTER_FD fp_register) {

  __time_t sec = usec / 1000000;
  __time_t nsec = (usec - sec * 1000000) * 1000;
  struct timespec period = { .tv_sec = sec, .tv_nsec = nsec };
  struct itimerspec new_value = { .it_interval = period, .it_value = period, };

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

  ret = fp_register(tfd, slot, read_callback, NULL, close_callback);
  if (ret < 0) {
    close(tfd);
    return -1;
  }

  timers[slot].fd = tfd;
  timers[slot].user = user;
  timers[slot].fp_read = fp_read;
  timers[slot].fp_close = fp_close;

  return slot;
}

int gtimer_close(int timer) {

  CHECK_TIMER(timer, -1)

  gpoll_remove_fd(timers[timer].fd);
  close(timers[timer].fd);
  memset(timers + timer, 0x00, sizeof(*timers));

  return 1;
}
