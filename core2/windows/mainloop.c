
#include <sys/time.h>
#include "sdl_tools.h"
#include "emuclient.h"
#include "calibration.h"
#include "serial_con.h"
#include "gpp_con.h"
#include "tcp_con.h"
#include "macros.h"
#include "display.h"
#include "events.h"
#include <windows.h>
#include <unistd.h>

static int done = 0;

void set_done()
{
  done = 1;
}

void mainloop()
{
  SDL_Event events[EVENT_BUFFER_SIZE];
  int num_evt;
  SDL_Event* event;
  LARGE_INTEGER t0, t1, freq;
  int time_to_sleep;
  int ptl;
  
  QueryPerformanceFrequency(&freq);
    
  if(emuclient_params.grab)
  {
    usleep(1000000);
    sdl_grab();
  }

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
      sdl_pump_events();
    } 

    num_evt = sdl_peep_events(events, sizeof(events) / sizeof(events[0]), SDL_GETEVENT, SDL_ALLEVENTS);

    num_evt = sdl_preprocess_events(events, num_evt);

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      printf("buffer too small!!!\n");
    }

    for (event = events; event < events + num_evt; ++event)
    {
      if (event->type != SDL_MOUSEMOTION)
      {
        if (!cal_skip_event(event))
        {
          cfg_process_event(event);
        }
      }
      else
      {
        cfg_process_motion_event(event);
      }

      cfg_trigger_lookup(event);
      cfg_intensity_lookup(event);

      switch (event->type)
      {
        case SDL_QUIT:
          done = 1;
          break;
        case SDL_MOUSEBUTTONDOWN:
          cal_button(event->button.which, event->button.button);
          break;
        case SDL_KEYDOWN:
          cal_key(event->key.which, event->key.keysym.sym, 1);
          break;
        case SDL_KEYUP:
          cal_key(event->key.which, event->key.keysym.sym, 0);
          break;
      }
      
      macro_lookup(event);
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

    time_to_sleep = emuclient_params.refresh_rate - (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart;

    ptl = emuclient_params.refresh_rate - time_to_sleep;
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
        printf(_("processing time higher than %dus: %dus!!\n"), emuclient_params.refresh_rate, emuclient_params.refresh_rate - time_to_sleep);
      }
    }
  }
}
