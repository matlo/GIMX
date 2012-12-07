/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <sys/timerfd.h>
#include <emuclient.h>
#include <unistd.h>

#define PERIOD_SEC      0
#define PERIOD_NSEC     emuclient_params.refresh_rate*1000 // 4ms

int tfd = -1;

int timer_getfd()
{
  return tfd;
}

int timer_start()
{
  struct itimerspec new_value =
  {
      .it_interval = {.tv_sec = PERIOD_SEC, .tv_nsec = PERIOD_NSEC },
      .it_value =    {.tv_sec = PERIOD_SEC, .tv_nsec = PERIOD_NSEC },
  };

  tfd = timerfd_create(CLOCK_REALTIME, 0);
  if(tfd < 0)
  {
    fprintf(stderr, "timerfd_create");
    return -1;
  }
  if(timerfd_settime(tfd, 0, &new_value, NULL))
  {
    fprintf(stderr, "timerfd_settime");
    close(tfd);
    return -1;
  }
  return 0;
}

void timer_close()
{
  close(tfd);
}
