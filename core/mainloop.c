/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxinput/include/ginput.h>
#include <gimxpoll/include/gpoll.h>
#include <gimxtimer/include/gtimer.h>
#include <gimxusb/include/gusb.h>
#include <gimxcommon/include/gperf.h>
#include <gimxtime/include/gtime.h>
#include "gimx.h"
#include "calibration.h"
#include "macros.h"
#include <stdio.h>
#include <controller.h>
#include <connectors/usb_con.h>
#include <connectors/report2event/report2event.h>

#define SAMPLETYPE \
    struct { \
            gtime now; \
            gtime delta; \
        }

#define NBSAMPLES 2500 // 10s with a period of 4ms

#define SAMPLEPRINT(SAMPLE) \
    printf("now = "GTIME_FS" delta = "GTIME_FS"\n", SAMPLE.now, SAMPLE.delta)

static GPERF_INST(mainloop, SAMPLETYPE, NBSAMPLES);

static volatile int done = 0;

void set_done()
{
  done = 1;
}

int get_done()
{
  return done;
}

static int timer_read(void * user __attribute__((unused)))
{
  return 1;
}

static int timer_close(void * user __attribute__((unused)))
{
  set_done();
  return 1;
}

e_gimx_status mainloop()
{
  e_gimx_status status = E_GIMX_STATUS_SUCCESS;
  GE_Event events[EVENT_BUFFER_SIZE];
  int num_evt;
  GE_Event* event;
  unsigned int running_macros;
  struct gtimer * timer = NULL;

  unsigned int refresh_period = (unsigned int)gimx_params.refresh_period;

  GTIMER_CALLBACKS callbacks = {
          .fp_read = timer_read,
          .fp_close = timer_close,
          .fp_register = REGISTER_FUNCTION,
          .fp_remove = REMOVE_FUNCTION,
  };

  if(adapter_get(0)->atype != E_ADAPTER_TYPE_BLUETOOTH || adapter_get(0)->ctype == C_TYPE_DS4)
  {
    timer = gtimer_start(NULL, refresh_period, &callbacks);
    if (timer == NULL)
    {
      done = 1;
    }
  }

  report2event_set_callback(process_event);

  // flush all messages
  fflush(stdout);
  fflush(stderr);

  while(!done)
  {
    /*
     * gpoll should always be executed as it drives the period.
     */
    gpoll();

    ginput_periodic_task();

    cfg_process_motion();

    cfg_profile_activation();

    if(adapter_send() < 0)
    {
      done = 1;
    }

    if (timer != NULL && (unsigned int)gimx_params.refresh_period != refresh_period)
    {
      refresh_period = gimx_params.refresh_period;
      gimx_params.frequency_scale = (double) DEFAULT_REFRESH_PERIOD / gimx_params.refresh_period;
      gtimer_close(timer);
      gwarn(_("Lowering controller frequency to %dHz due to low mouse frequency.\n"), 1000000 / refresh_period);
      timer = gtimer_start(NULL, refresh_period, &callbacks);
      if (timer == NULL)
      {
        done = 1;
      }
    }

    if (gimx_params.debug.mainloop) {
        GPERF_TICK(mainloop, gtime_gettime());
        if (gperf_mainloop.count > 0) {
            GPERF_SAMPLE(mainloop).now = gperf_mainloop.end;
            GPERF_SAMPLE(mainloop).delta = gperf_mainloop.diff;
            GPERF_SAMPLE_INC(mainloop);
        }
    }

    cfg_process_rumble();
    
    if (usb_poll_interrupts() < 0)
    {
      done = 1;
      status = E_GIMX_STATUS_AUTH_CONTROLLER_ERROR;
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

    num_evt = ginput_queue_pop(events, sizeof(events) / sizeof(events[0]));

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      gwarn("buffer too small!!!\n");
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

  if (timer != NULL)
  {
    gtimer_close(timer);
  }

  if (gimx_params.debug.mainloop) {
      GPERF_SAMPLE_PRINT(mainloop, SAMPLEPRINT);
      GPERF_LOG(mainloop);
  }

  return status;
}
