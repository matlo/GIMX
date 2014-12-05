/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include "gimx.h"
#include "calibration.h"
#include "connectors/connector.h"
#include "macros.h"
#include <stdio.h>
#include <adapter.h>
#include <connectors/usb_con.h>

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

  if(!adapter_get(0)->bdaddr_dst || adapter_get(0)->type == C_TYPE_DS4)
  {
    GE_TimerStart(gimx_params.refresh_period);
  }

  /*
   * Non-generated events are ignored if the --keygen argument is used.
   */
  if(gimx_params.keygen)
  {
    GE_SetCallback(ignore_event);
  }
  else
  {
    GE_SetCallback(process_event);
  }

  usb_set_event_callback(process_event);

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
    
    usb_poll_interrupts();

#ifdef WIN32
    usb_handle_events(0);
#endif

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
    if(!gimx_params.network_input && gimx_params.keygen && !running_macros && !num_evt)
    {
      done = 1;
    }
  }
    
  GE_TimerClose();
}
