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
#include "controllers/controller.h"

static volatile int done = 0;

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
  unsigned int running_macros;

  if(!get_controller(0)->bdaddr_dst)
  {
    GE_TimerStart(&period);
  }

  /*
   * Non-generated events are ignored if the --keygen argument is used.
   */
  if(emuclient_params.keygen)
  {
    GE_SetCallback(ignore_event);
  }
  else
  {
    GE_SetCallback(process_event);
  }

  while(!done)
  {
    /*
     * GE_PumpEvents should always be executed as it drives the period.
     */
    GE_PumpEvents();

    cfg_process_motion();

    cfg_config_activation();

    if(connector_send() < 0)
    {
      done = 1;
    }

    if(emuclient_params.curses)
    {
      display_run(get_controller(0)->axis);
    }

    /*
     * These two functions generate events.
     */
    calibration_test();

    running_macros = macro_process();

    /*
     * This part of the loop processes events generated
     * by macros and calibration tests, and the --keygen argument.
     */

    num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      printf("buffer too small!!!\n");
    }
    
    for (event = events; event < events + num_evt; ++event)
    {
      process_event(event);
    }

    /*
     * The --keygen argument is used
     * and there are no more event or macro to process => exit.
     */
    if(emuclient_params.keygen && !running_macros && !num_evt)
    {
      done = 1;
    }
  }
    
  GE_TimerClose();
}
