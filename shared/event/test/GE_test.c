/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "common.h"

#define PERIOD 10000//microseconds

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static void terminate(int sig)
{
  done = 1;
}

int main(int argc, char* argv[])
{
  if (!GE_initialize(GE_MKB_SOURCE_PHYSICAL, process_event))
  {
    fprintf(stderr, "GE_initialize failed\n");
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

#ifndef WIN32
  setlinebuf(stdout);
#endif

  display_devices();

  int timer = gtimer_start(42, PERIOD, timer_read, timer_close, REGISTER_FUNCTION);
  if (timer < 0) {
    done = 1;
  }
  
  GE_grab();

  while(!done)
  {
    gpoll();

    //do something periodically
  }

  if (timer >= 0) {
    gtimer_close(timer);
  }

  GE_quit();

  printf("Exiting\n");

  return 0;
}
