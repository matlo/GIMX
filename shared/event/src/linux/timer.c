/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <sys/timerfd.h>
#include <unistd.h>
#include <stdio.h>

int tfd = -1;

int timer_getfd()
{
  return tfd;
}

int timer_start(struct timespec* period)
{
  struct itimerspec new_value =
  {
      .it_interval = *period,
      .it_value = *period,
  };
  
  if(tfd != -1)
  {
    return -1;
  }

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
