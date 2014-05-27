/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include "emuclient.h"
#include "calibration.h"
#include "connectors/connector.h"
#include "macros.h"
#include "display.h"
#include <windows.h>
#include <unistd.h>
#include <adapter.h>

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
  unsigned int running_macros;

  HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  LARGE_INTEGER li = { .QuadPart = 0 };
  SetWaitableTimer(hTimer, &li, emuclient_params.refresh_period / 1000, NULL, NULL, FALSE);
  timeBeginPeriod(1);

  while (!done)
  {
    /*
     * These two functions generate events.
     */
	  calibration_test();

	  running_macros = macro_process();

    /*
     * Non-generated events are ignored if the --keygen argument is used.
     */
    if(!emuclient_params.keygen)
    {
      GE_PumpEvents();
    }
    
    /*
     * This part of the loop processes all events.
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
      continue;//no need to send anything...
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
      display_run(adapter_get(0)->type, adapter_get(0)->axis);
    }

    MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE, 0);
  }

  timeEndPeriod(0);
}
