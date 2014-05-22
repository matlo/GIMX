/*
 * linux_test.c
 *
 *  Created on: 13 janv. 2013
 *      Author: matlo
 */

#include <GE.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "common.h"

#define PERIOD 10000//microseconds

static void terminate(int sig)
{
  done = 1;
}

int main(int argc, char* argv[])
{
  if (!GE_initialize())
  {
    fprintf(stderr, "GE_initialize: %s\n", strerror(errno));
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

#ifndef WIN32
  setlinebuf(stdout);
#endif

  display_devices();

  GE_TimerStart(PERIOD);

  GE_SetCallback(process_event);
  
  GE_grab();

  while(!done)
  {
    GE_PumpEvents();

    //do something periodically
  }

  GE_quit();

  printf("Exiting\n");

  return 0;
}
