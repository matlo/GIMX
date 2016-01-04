/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include "gimx.h"
#include "calibration.h"
#include "macros.h"
#include <stdio.h>
#include <adapter.h>
#include <connectors/usb_con.h>
#include <report2event/report2event.h>

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static volatile int done = 0;

void set_done()
{
  done = 1;
}

static int timer_read(int user)
{
  return 1;
}

static int timer_close(int timer)
{
  set_done();
  return 1;
}

void mainloop()
{
  GE_Event events[EVENT_BUFFER_SIZE];
  int num_evt;
  GE_Event* event;
  unsigned int running_macros;
  int timer = -1;

  if(!adapter_get(0)->bdaddr_dst || adapter_get(0)->ctype == C_TYPE_DS4)
  {
    timer = gtimer_start(0, gimx_params.refresh_period, timer_read, timer_close, REGISTER_FUNCTION);
    if (timer < 0)
    {
      done = 1;
    }
  }

  report2event_set_callback(process_event);

  while(!done)
  {
    /*
     * gpoll should always be executed as it drives the period.
     */
    gpoll();

    ginput_sync_process();

    cfg_process_motion();

    cfg_config_activation();

    if(adapter_send() < 0)
    {
      done = 1;
    }

    cfg_process_rumble();
    
    usb_poll_interrupts();

#ifndef WIN32
    adapter_hid_poll();
#endif

#ifdef WIN32
    if (gusb_handle_events(0) < 0)
    {
      done = 1;
    }
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

    num_evt = ginput_queue_pop(events, sizeof(events) / sizeof(events[0]));

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

  if (timer >= 0) {
    gtimer_close(timer);
  }
}
