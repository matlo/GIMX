/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 Copyright (c) 2009 Jim Paris <jim@jtan.com>
 License: GPLv3
 */

#ifndef WIN32
#include <termio.h>
#include <sys/ioctl.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sched.h>
#else
#include <windows.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "sdl_tools.h"
#include "sixaxis.h"
#include "dump.h"
#include "macros.h"
#include "config.h"
#include "config_writter.h"
#include "config_reader.h"
#include <sys/time.h>
#include "calibration.h"
#include <libxml/parser.h>
#include "serial_con.h"
#include "gpp_con.h"
#include "tcp_con.h"
#include "display.h"
#include "emuclient.h"

#define DEFAULT_POSTPONE_COUNT 3
#define DEFAULT_MAX_AXIS_VALUE 255
#define DEFAULT_AXIS_SCALE 1

#ifndef WIN32
char* homedir;
#endif

#ifdef WIN32
static void err(int eval, const char *fmt)
{
  fprintf(stderr, fmt);
  exit(eval);
}
#endif

char* config_file = NULL;

char* portname = NULL;

char* keygen = NULL;

char* ip = NULL;

int refresh_rate = DEFAULT_REFRESH_PERIOD; //microseconds
int postpone_count = DEFAULT_POSTPONE_COUNT;
int max_axis_value = DEFAULT_MAX_AXIS_VALUE;
double frequency_scale;
int subpos = 0;

int done = 0;
int display = 0;
int curses = 0;
int force_updates = 0;
int check_config = 0;

struct sixaxis_state state[MAX_CONTROLLERS];
s_controller controller[MAX_CONTROLLERS] =
{ };

int merge_all_devices;

int proc_time = 0;
int proc_time_worst = 0;
int proc_time_total = 0;

static int max_unsigned_axis_value[SA_MAX] = {};

inline int get_max_unsigned(int axis)
{
  return max_unsigned_axis_value[axis];
}

inline int get_max_signed(int axis)
{
  if(axis < sa_select)
  {
    /*
     * relative axis
     */
    return get_max_unsigned(axis) / 2 + 1;
  }
  else
  {
    /*
     * absolute axis
     */
    return get_max_unsigned(axis);
  }
}

inline int get_mean_unsigned(int axis)
{
  return get_max_unsigned(axis) / 2 + 1;
}

inline double get_axis_scale(int axis)
{
  return (double) get_max_unsigned(axis) / DEFAULT_MAX_AXIS_VALUE;
}

int main(int argc, char *argv[])
{
  int grab = 1;
  SDL_Event events[EVENT_BUFFER_SIZE];
  SDL_Event* event;
  SDL_Event kgevent = {.type = SDL_KEYDOWN};
  int i;
  int num_evt;
  int read = 0;
#ifndef WIN32
  struct timeval t0, t1;
#else
  LARGE_INTEGER t0, t1, freq;
#endif
  int time_to_sleep;
  e_controller_type ctype = C_TYPE_DEFAULT;
  int ptl;

  setlocale( LC_ALL, "" );
#ifndef WIN32
  bindtextdomain( "gimx", "/usr/share/locale" );
#else
  bindtextdomain( "gimx", "share/locale" );
#endif
  textdomain( "gimx" );

  setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */
  
#ifndef WIN32
  /*
   * Set highest priority & scheduler policy.
   */
  struct sched_param p = {.sched_priority = 99};

  sched_setscheduler(0, SCHED_FIFO, &p);
  //setpriority(PRIO_PROCESS, getpid(), -20); only useful with SCHED_OTHER

  setlinebuf(stdout);
  homedir = getpwuid(getuid())->pw_dir;
#else
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

  QueryPerformanceFrequency(&freq);
#endif

  for(i = 0; i < SA_MAX; ++i)
  {
    max_unsigned_axis_value[i] = DEFAULT_MAX_AXIS_VALUE;
  }
  max_unsigned_axis_value[sa_acc_x] = 1023;
  max_unsigned_axis_value[sa_acc_y] = 1023;
  max_unsigned_axis_value[sa_acc_z] = 1023;
  max_unsigned_axis_value[sa_gyro]  = 1023;

  for (i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "--nograb"))
    {
      grab = 0;
    }
    else if (!strcmp(argv[i], "--config") && i < argc)
    {
      config_file = argv[++i];
      read = 1;
    }
    else if (!strcmp(argv[i], "--port") && i < argc)
    {
      portname = argv[++i];
    }
    else if (!strcmp(argv[i], "--status"))
    {
      if(!curses)
      {
        display = 1;
      }
    }
    else if (!strcmp(argv[i], "--refresh"))
    {
      refresh_rate = atof(argv[++i]) * 1000;
      postpone_count = 3 * DEFAULT_REFRESH_PERIOD / refresh_rate;
    }
    else if (!strcmp(argv[i], "--subpos"))
    {
      subpos = 1;
    }
    else if (!strcmp(argv[i], "--force-updates"))
    {
      force_updates = 1;
    }
    else if (!strcmp(argv[i], "--check"))
    {
      check_config = 1;
      display = 1;
    }
    else if (!strcmp(argv[i], "--joystick"))
    {
      ctype = C_TYPE_JOYSTICK;
      max_unsigned_axis_value[sa_lstick_x] = 65535;
      max_unsigned_axis_value[sa_lstick_y] = 65535;
      max_unsigned_axis_value[sa_rstick_x] = 65535;
      max_unsigned_axis_value[sa_rstick_y] = 65535;
    }
    else if (!strcmp(argv[i], "--360pad"))
    {
      ctype = C_TYPE_360_PAD;
    }
    else if (!strcmp(argv[i], "--Sixaxis"))
    {
      ctype = C_TYPE_SIXAXIS;
    }
    else if (!strcmp(argv[i], "--PS2pad"))
    {
      ctype = C_TYPE_PS2_PAD;
    }
    else if (!strcmp(argv[i], "--GPP"))
    {
      ctype = C_TYPE_GPP;
    }
    else if (!strcmp(argv[i], "--keygen") && i < argc)
    {
      keygen = argv[++i];
    }
    else if (!strcmp(argv[i], "--curses"))
    {
      if(!display)
      {
        curses = 1;
      }
    }
    else if (!strcmp(argv[i], "--ip") && i < argc)
    {
      ip = argv[++i];
    }
  }

  if(curses)
  {
    display_init();
  }

  frequency_scale = (double) DEFAULT_REFRESH_PERIOD / refresh_rate;

  initialize_macros();

  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    sixaxis_init(state + i);
    memset(controller + i, 0x00, sizeof(s_controller));
  }

  if (!sdl_initialize())
  {
    err(1, "can't init sdl");
  }

  if(grab)
  {
    usleep(1000000);
    sdl_grab();
  }

  if (read == 1)
  {
    read_config_file(config_file);

    if(check_config)
    {
      goto EXIT;
    }

    if(merge_all_devices)
    {
      free_config();
      sdl_free_mk();
      read_config_file(config_file);
    }

    sdl_release_unused();
  }

  switch(ctype)
  {
    case C_TYPE_DEFAULT:
      if(tcp_connect() < 0)
      {
        err(1, "tcp_connect");
      }
      break;
    case C_TYPE_GPP:
      if (gpp_connect() < 0)
      {
        err(1, "gpp_connect");
      }
      break;
    default:
      if(!strstr(portname, "none") && serial_connect(portname) < 0)
      {
        err(1, "serial_connect");
      }
      break;
  }

  if(keygen)
  {
    kgevent.key.keysym.sym = get_key_from_buffer(keygen);
    if(kgevent.key.keysym.sym != SDLK_UNKNOWN)
    {
      SDL_PushEvent(&kgevent);
    }
    else
    {
      err(1, "Unknown key name for argument --keygen!");
    }
  }

  cfg_trigger_init();

  done = 0;
  while (!done)
  {
#ifndef WIN32
    gettimeofday(&t0, NULL);
#else
    QueryPerformanceCounter(&t0);
#endif

    /*
     * These two functions generate events.
     */
    macro_process();
	  calibration_test();
    
    if(!keygen)
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
        case SDL_KEYDOWN:
          cal_key(event->key.which, event->key.keysym.sym, 1);
          macro_lookup(0, event->key.which, event->key.keysym.sym, 1);
          break;
        case SDL_KEYUP:
          cal_key(event->key.which, event->key.keysym.sym, 0);
          macro_lookup(0, event->key.which, event->key.keysym.sym, 0);
          break;
        case SDL_MOUSEBUTTONDOWN:
          cal_button(event->button.which, event->button.button);
          macro_lookup(1, event->button.which, event->button.button, 1);
          break;
        case SDL_MOUSEBUTTONUP:
          macro_lookup(1, event->button.which, event->button.button, 0);
          break;
        case SDL_JOYBUTTONDOWN:
          macro_lookup(2, event->jbutton.which, event->jbutton.button, 1);
          break;
        case SDL_JOYBUTTONUP:
          macro_lookup(2, event->jbutton.which, event->jbutton.button, 0);
          break;
      }

    }

    cfg_process_motion();

    cfg_config_activation();

    switch(ctype)
    {
      case C_TYPE_DEFAULT:
        tcp_send(force_updates);
        break;
      case C_TYPE_GPP:
        gpp_send(force_updates);
        break;
      default:
        serial_send(ctype, force_updates);
        break;
    }

#ifdef WIN32
    /*
     * There is no setlinebuf(stdout) in windows.
     */
    if(display)
    {
      fflush(stdout);
    }
#endif
    if(curses)
    {
      display_run(state[0].user.axis);
    }

#ifndef WIN32
    gettimeofday(&t1, NULL);

    time_to_sleep = refresh_rate - ((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec));
#else
    QueryPerformanceCounter(&t1);

    time_to_sleep = refresh_rate - (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart;
#endif

    ptl = refresh_rate - time_to_sleep;
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
      if(!curses)
      {
        printf(_("processing time higher than %dus: %dus!!\n"), refresh_rate, refresh_rate - time_to_sleep);
      }
    }
  }

  gprintf(_("Exiting\n"));

EXIT:
  free_macros();
  free_config();
  sdl_quit();
  switch(ctype)
  {
    case C_TYPE_DEFAULT:
      tcp_close();
      break;
    case C_TYPE_GPP:
      gpp_disconnect();
      break;
    default:
      serial_close();
      break;
  }

  xmlCleanupParser();

  if(curses)
  {
    display_end();
  }

  return 0;
}
