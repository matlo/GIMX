
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
  e_controller_type ctype = get_controller_type();

  while (!done)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    //printf("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);

    if(!get_keygen())
    {
      sdl_pump_events();
    }

    cfg_process_motion();

    cfg_config_activation();

    switch(ctype)
    {
      case C_TYPE_DEFAULT:
        tcp_send(get_force_updates());
        break;
      case C_TYPE_GPP:
        gpp_send(get_force_updates());
        break;
      default:
        serial_send(ctype, get_force_updates());
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
      SDL_ProcessEvent(event);
    }
  }
}
