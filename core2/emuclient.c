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
#include <sys/timerfd.h>
#include "linux_mainloop.h"
#include "connector.h"
#include "args.h"

#include <locale.h>

#define EVENT_BUFFER_SIZE 256
#define DEFAULT_POSTPONE_COUNT 3
#define DEFAULT_MAX_AXIS_VALUE 255
#define DEFAULT_AXIS_SCALE 1

#ifdef WIN32
static void err(int eval, const char *fmt)
{
  fprintf(stderr, fmt);
  exit(eval);
}
#endif

s_emuclient_params emuclient_params =
{
   .ctype = C_TYPE_DEFAULT,
#ifndef WIN32
  .homedir = NULL,
#endif
  .portname = NULL,
  .force_updates = 0,
  .keygen = NULL,
  .grab = 1,
  .refresh_rate = DEFAULT_REFRESH_PERIOD,
  .max_axis_value = DEFAULT_MAX_AXIS_VALUE,
  .frequency_scale = 1,
  .status = 0,
  .curses = 0,
  .config_file = NULL,
  .ip = NULL,
  .postpone_count = DEFAULT_POSTPONE_COUNT,
  .subpos = 0,
};

int done = 0;

inline int get_force_updates()
{
  return emuclient_params.force_updates;
}

inline const char* get_keygen()
{
  return emuclient_params.keygen;
}

inline e_controller_type get_controller_type()
{
  return emuclient_params.ctype;
}

struct sixaxis_state state[MAX_CONTROLLERS];
s_controller controller[MAX_CONTROLLERS] =
{ };

int merge_all_devices;

int proc_time = 0;
int proc_time_worst = 0;
int proc_time_total = 0;

/* TIMER */
#include <signal.h>
#include <time.h>
#include <stdio.h>

#define PERIOD_SIG      SIGRTMIN
#define PERIOD_SEC      0
#define PERIOD_NSEC     emuclient_params.refresh_rate*1000 // 4ms

/* TIMER */

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

int tfd = -1;

static int start_timer()
{
  struct itimerspec new_value =
  {
      .it_interval = {.tv_sec = PERIOD_SEC, .tv_nsec = PERIOD_NSEC },
      .it_value =    {.tv_sec = PERIOD_SEC, .tv_nsec = PERIOD_NSEC },
  };

  tfd = timerfd_create(CLOCK_REALTIME, 0);
  if(tfd < 0)
  {
    fprintf(stderr, "timerfd_create");
    return -1;
  }
  if(timerfd_settime(tfd, 0, &new_value, NULL))
  {
    fprintf(stderr, "timerfd_settime");
    close(tfd);
    return -1;
  }
  return 0;
}

static void close_timer()
{
  close(tfd);
}

int main(int argc, char *argv[])
{
  SDL_Event kgevent = {.type = SDL_KEYDOWN};
  int i;
#ifdef WIN32
  LARGE_INTEGER t0, t1, freq;
#endif

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
  emuclient_params.homedir = getpwuid(getuid())->pw_dir;
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

  args_read(argc, argv, &emuclient_params);

  if (emuclient_params.ctype == C_TYPE_JOYSTICK)
  {
    max_unsigned_axis_value[sa_lstick_x] = 65535;
    max_unsigned_axis_value[sa_lstick_y] = 65535;
    max_unsigned_axis_value[sa_rstick_x] = 65535;
    max_unsigned_axis_value[sa_rstick_y] = 65535;
  }

  if(emuclient_params.curses)
  {
    display_init();
  }

  emuclient_params.frequency_scale = (double) DEFAULT_REFRESH_PERIOD / emuclient_params.refresh_rate;

  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    sixaxis_init(state + i);
    memset(controller + i, 0x00, sizeof(s_controller));
  }

  if(emuclient_params.grab)
  {
    usleep(1000000);
    sdl_grab();
  }

  if (!sdl_initialize())
  {
    err(1, "can't init sdl");
  }

  macros_init();

  if(read_config_file(emuclient_params.config_file) < 0)
  {

  }

  if(merge_all_devices)
  {
    free_config();
    sdl_free_mouse_keyboard_names();
    read_config_file(emuclient_params.config_file);
  }

  sdl_release_unused();

  macros_read();

  if(connector_init(emuclient_params.ctype, emuclient_params.portname) < 0)
  {
    err(1, "connector_init");
  }

  if(emuclient_params.keygen)
  {
    kgevent.key.keysym.sym = get_key_from_buffer(emuclient_params.keygen);
    if(kgevent.key.keysym.sym != SDLK_UNKNOWN)
    {
      macro_lookup(&kgevent);
    }
    else
    {
      err(1, "Unknown key name for argument --keygen!");
    }
  }

  cfg_trigger_init();

  start_timer();

  mainloop();

  gprintf(_("Exiting\n"));

  close_timer();

  free_macros();
  free_config();
  sdl_quit();
  connector_clean();

  xmlCleanupParser();

  if(emuclient_params.curses)
  {
    display_end();
  }

  return 0;
}
