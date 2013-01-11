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
  struct sched_param p = {.sched_priority = 99};

  sched_setscheduler(0, SCHED_FIFO, &p);
  //setpriority(PRIO_PROCESS, getpid(), -20); only useful with SCHED_OTHER
}
