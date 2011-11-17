/* Sixaxis emulator

 Copyright (c) 2010 Mathieu Laurendeau

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
#include "tcp_con.h"
#include <sys/resource.h>
#include <sched.h>
#else
#include <windows.h>
#define sleep Sleep
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

#define EVENT_BUFFER_SIZE 256
#define DEFAULT_POSTPONE_COUNT 3
#define DEFAULT_MAX_AXIS_VALUE 255
#define DEFAULT_AXIS_SCALE 1

#ifndef WIN32
char* homedir = "";
//#else
//char* ip = "";
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

int refresh = DEFAULT_REFRESH_PERIOD; //µs
int postpone_count = DEFAULT_POSTPONE_COUNT;
int max_axis_value = DEFAULT_MAX_AXIS_VALUE;
int mean_axis_value = DEFAULT_MAX_AXIS_VALUE / 2;
double axis_scale = DEFAULT_AXIS_SCALE;
double frequency_scale;
int subpos = 0;

int serial = 0;
int done = 0;
int display = 0;
int force_updates = 0;
int check_config = 0;

struct sixaxis_state state[MAX_CONTROLLERS];
s_controller controller[MAX_CONTROLLERS] =
{ };

extern int merge_all_devices;

int main(int argc, char *argv[])
{
  int grab = 1;
  SDL_Event events[EVENT_BUFFER_SIZE];
  SDL_Event* event;
  int i;
  int num_evt;
  int read = 0;
#ifndef WIN32
  struct timeval t0, t1;
#else
  LARGE_INTEGER t0, t1, freq;
#endif
  int time_to_sleep;

#ifndef WIN32
  /*
   * Set highest priority & scheduler policy.
   */
  struct sched_param p = {.sched_priority = 99};

  sched_setscheduler(0, SCHED_FIFO, &p);
  //setpriority(PRIO_PROCESS, getpid(), -20); only useful with SCHED_OTHER

  setlinebuf(stdout);
  homedir = getpwuid(getuid())->pw_dir;

  system("test -d ~/.emuclient || cp -r /etc/emuclient ~/.emuclient");
#else
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

  QueryPerformanceFrequency(&freq);
#endif

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
      display = 1;
    }
    else if (!strcmp(argv[i], "--refresh"))
    {
      refresh = atoi(argv[++i]) * 1000;
      postpone_count = 3 * DEFAULT_REFRESH_PERIOD / refresh;
    }
    else if (!strcmp(argv[i], "--precision"))
    {
      max_axis_value = (1 << atoi(argv[++i])) - 1;
      mean_axis_value = max_axis_value / 2;
    }
    else if (!strcmp(argv[i], "--serial"))
    {
      serial = 1;
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
    }
//#ifdef WIN32
//    else if (!strcmp(argv[i], "--ip") && i < argc)
//    {
//      ip = argv[++i];
//    }
//#endif
  }

#ifdef WIN32
  if (!portname && !check_config)
  {
    err(1, "no serial port specified!\n");
  }
  /*
   * Force precision to 16bits
   */
  max_axis_value = (1 << 16) - 1;
  mean_axis_value = max_axis_value / 2;
#endif

  if (display == 1)
  {
    printf("max_axis_value: %d\n", max_axis_value);//needed by sixstatus...
  }

  axis_scale = (double) max_axis_value / DEFAULT_MAX_AXIS_VALUE;
  frequency_scale = (double) DEFAULT_REFRESH_PERIOD / refresh;

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
    sleep(1);
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

#ifndef WIN32
  if(serial)
  {
    if (serial_connect(portname) < 0)
    {
      err(1, "serial_connect");
    }
  }
  else if(tcp_connect() < 0)
  {
    err(1, "tcp_connect");
  }
#else
  if (serial_connect(portname) < 0)
  {
    err(1, "serial_connect");
  }
#endif

  done = 0;
  while (!done)
  {
#ifndef WIN32
    gettimeofday(&t0, NULL);
#else
    QueryPerformanceCounter(&t0);
#endif

    SDL_PumpEvents();
    num_evt = SDL_PeepEvents(events, sizeof(events) / sizeof(events[0]),
        SDL_GETEVENT, SDL_ALLEVENTS);

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
          macro_lookup(event->key.which, event->key.keysym.sym);
          if(event->key.keysym.sym == SDLK_ESCAPE)
          {
            done = 1;
          }
          break;
        case SDL_KEYUP:
          cal_key(event->key.which, event->key.keysym.sym, 0);
          break;
        case SDL_MOUSEBUTTONDOWN:
          cal_button(event->button.which, event->button.button);
          break;
      }
    }

    cfg_process_motion();

    cfg_config_activation();

#ifndef WIN32
    if(serial)
    {
      serial_send(force_updates);
    }
    else
    {
      tcp_send(force_updates);
    }
#else
    serial_send(force_updates);
#endif

#ifdef WIN32
    /*
     * There is no setlinebuf(stdout) in windows.
     */
    if(display)
    {
      fflush(stdout);
    }
#endif

#ifndef WIN32
    gettimeofday(&t1, NULL);

    time_to_sleep = refresh - ((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec));
#else
    QueryPerformanceCounter(&t1);

    time_to_sleep = refresh - (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart;
#endif

    if (time_to_sleep > 0)
    {
      usleep(time_to_sleep);
    }
    else

    {
      printf("processing time higher than %dus: %dus!!\n", refresh, refresh - time_to_sleep);
    }
  }

  printf("Exiting\n");

EXIT:
  free_macros();
  free_config();
  sdl_quit();
#ifdef WIN32
  serial_close();
#endif

  xmlCleanupParser();

  return 0;
}
