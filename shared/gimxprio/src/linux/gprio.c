/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <sched.h>
#include <stdio.h>

int gprio()
{
  /*
   * Set highest priority & scheduler policy.
   */
  struct sched_param p =
  { .sched_priority = sched_get_priority_max(SCHED_FIFO) };

  if( sched_setscheduler(0, SCHED_FIFO, &p) < 0 )
  {
    perror("sched_setscheduler");
    return -1;
  }
  return 0;
}
