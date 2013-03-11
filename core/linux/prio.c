/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <sched.h>

void set_prio()
{
  /*
   * Set highest priority & scheduler policy.
   */
  struct sched_param p =
  { .sched_priority = sched_get_priority_max(SCHED_FIFO) };

  sched_setscheduler(0, SCHED_FIFO, &p);
}
