
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
#include <timer.h>

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
    
  timer_start();

  while (!done)
  {
    if(!emuclient_params.keygen)
    {
      sdl_pump_events();
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

    num_evt = sdl_peep_events(events, sizeof(events) / sizeof(events[0]), SDL_GETEVENT, SDL_ALLEVENTS);

    if (num_evt == EVENT_BUFFER_SIZE)
    {
      printf("buffer too small!!!\n");
    }

    for (event = events; event < events + num_evt; ++event)
    {
      sdl_process_event(event);
    }
  }
    
  timer_close();
}
