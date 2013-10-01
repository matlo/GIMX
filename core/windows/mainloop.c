/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include "emuclient.h"
#include "calibration.h"
#include "connector.h"
#include "macros.h"
#include "display.h"
#include <windows.h>
#include <unistd.h>

static int done = 0;

void set_done()
{
  done = 1;
}

void mainloop()
{
  GE_Event events[EVENT_BUFFER_SIZE];
  int num_evt;
  GE_Event* event;
  LARGE_INTEGER t0, t1, freq;
  int time_to_sleep;
  int ptl;
  
  QueryPerformanceFrequency(&freq);

  while (!done)
  {
    QueryPerformanceCounter(&t0);

    /*
     * These two functions generate events.
     */
    macro_process();
	  calibration_test();
    
    if(!emuclient_params.keygen)
    {
      GE_PumpEvents();
    } 

    num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      printf("buffer too small!!!\n");
    }

    for (event = events; event < events + num_evt; ++event)
    {
      process_event(event);
    }

    cfg_process_motion();

    cfg_config_activation();

    if(connector_send() < 0)
    {
      done = 1;
    }

    /*
     * There is no setlinebuf(stdout) in windows.
     */
    if(emuclient_params.status)
    {
      fflush(stdout);
    }
    if(emuclient_params.curses)
    {
      display_run(state[0].user.axis);
    }

    QueryPerformanceCounter(&t1);

    time_to_sleep = emuclient_params.refresh_period - (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart;

    ptl = emuclient_params.refresh_period - time_to_sleep;
    proc_time += ptl;
    proc_time_total += ptl;
    if(ptl > proc_time_worst && proc_time_total > 50000)
    {
      proc_time_worst = ptl;
    }

    if (time_to_sleep > 0)
    {
      usleep(time_to_sleep);
    }
    else
    {
      if(!emuclient_params.curses)
      {
        printf(_("processing time higher than %dus: %dus!!\n"), emuclient_params.refresh_period, emuclient_params.refresh_period - time_to_sleep);
      }
    }
  }
}
