/*
 * windows_test.c
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
#include <unistd.h>

#include "common.h"

static void terminate(int sig)
{
  done = 1;
}

int main(int argc, char* argv[])
{
  int num_evt;
  GE_Event events[EVENT_BUFFER_SIZE];
  GE_Event* event;
  
  if (!GE_initialize())
  {
    fprintf(stderr, "GE_initialize: %s\n", strerror(errno));
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

  display_devices();

  while(!done)
  {
    GE_PumpEvents();

    num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));
 
     if (num_evt > 0)
     {
       for (event = events; event < events + num_evt; ++event)
       {
         process_event(event);
       }
     }
     
     usleep(1000);
  }

  GE_quit();

  printf("Exiting\n");

  return 0;
}
