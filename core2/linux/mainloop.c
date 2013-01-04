/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <sys/time.h>
#include <GE.h>
#include "emuclient.h"
#include "calibration.h"
#include "serial_con.h"
#include "gpp_con.h"
#include "tcp_con.h"
#include "macros.h"
#include "display.h"
//#include <events.h>
#include <timer.h>
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
  struct timespec period = {.tv_sec = 0, .tv_nsec = emuclient_params.refresh_rate*1000};
    
  timer_start(&period);

  while (!done)
  {
    if(!emuclient_params.keygen)
    {
      GE_PumpEvents();
    }

    cfg_process_motion();

    cfg_config_activation();

    switch(emuclient_params.ctype)
    {
      case C_TYPE_DEFAULT:
        tcp_send(emuclient_params.force_updates);
        break;
      case C_TYPE_GPP:
        gpp_send(emuclient_params.force_updates);
        break;
      default:
        serial_con_send(emuclient_params.ctype, emuclient_params.force_updates);
        break;
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
    
  timer_close();
}
