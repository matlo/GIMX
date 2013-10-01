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
#include <stdio.h>

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
  struct timespec period = {.tv_sec = 0, .tv_nsec = emuclient_params.refresh_period*1000};
    
  GE_TimerStart(&period);

  GE_SetCallback(process_event);

  while (!done)
  {
    if(!emuclient_params.keygen)
    {
      GE_PumpEvents();
    }

    cfg_process_motion();

    cfg_config_activation();

    if(connector_send() < 0)
    {
      done = 1;
    }

    if(emuclient_params.curses)
    {
      display_run(state[0].user.axis);
    }

    calibration_test();

    macro_process();

    num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      printf("buffer too small!!!\n");
    }
    
    for (event = events; event < events + num_evt; ++event)
    {
      process_event(event);
    }
  }
    
  GE_TimerClose();
}
