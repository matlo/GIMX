/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 Copyright (c) 2009 Jim Paris <jim@jtan.com>
 License: GPLv3
 */

#include <stdio.h> //fprintf
#include <locale.h> //internationalization
#include <prio.h> //to set the thread priority
#include <signal.h> //to catch SIGINT
#include <errno.h> //to print errors
#include <string.h> //to print errors

#ifndef WIN32
#include <pwd.h> //to get the homedir
#include <sys/types.h> //to get the homedir
#include <unistd.h> //to get the homedir
#else
#include <windows.h>
#include <shlobj.h> //to get the homedir
#include <unistd.h> //usleep
#endif

#include "emuclient.h"
#include "macros.h"
#include "config_reader.h"
#include "calibration.h"
#include "display.h"
#include "mainloop.h"
#include "connector.h"
#include "args.h"

#define DEFAULT_POSTPONE_COUNT 3 //unit = DEFAULT_REFRESH_PERIOD
#define DEFAULT_MAX_AXIS_VALUE 255

int min_refresh_period[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] =  1000,
    [C_TYPE_360_PAD]  =  1000,
    [C_TYPE_SIXAXIS]  =  1000,
    [C_TYPE_PS2_PAD]  = 16000,
    [C_TYPE_XBOX_PAD] =  4000,
    [C_TYPE_GPP]      =  1000,
    [C_TYPE_DEFAULT]  = 11250,
};

int default_refresh_period[C_TYPE_MAX] =
{
    [C_TYPE_JOYSTICK] =  4000,
    [C_TYPE_360_PAD]  =  4000,
    [C_TYPE_SIXAXIS]  =  4000,
    [C_TYPE_PS2_PAD]  = 16000,
    [C_TYPE_XBOX_PAD] =  4000,
    [C_TYPE_GPP]      =  4000,
    [C_TYPE_DEFAULT]  = 11250,
};

s_emuclient_params emuclient_params =
{
   .ctype = C_TYPE_DEFAULT,
  .homedir = NULL,
  .portname = NULL,
  .force_updates = 0,
  .keygen = NULL,
  .grab = 1,
  .refresh_period = -1,
  .max_axis_value = DEFAULT_MAX_AXIS_VALUE,
  .frequency_scale = 1,
  .status = 0,
  .curses = 0,
  .config_file = NULL,
  .event = 0,
  .ip = NULL,
  .postpone_count = DEFAULT_POSTPONE_COUNT,
  .subpos = 0,
};

struct sixaxis_state state[MAX_CONTROLLERS];
s_controller controller[MAX_CONTROLLERS] =
{ };

void set_axis_value(int axis, int value)
{
  if(axis >= 0 && axis < SA_MAX)
  {
    state[0].user.axis[axis] = value;
  }
}

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

void terminate(int sig)
{
  set_done();
}

int ignore_event(GE_Event* event)
{
  return 0;
}

int process_event(GE_Event* event)
{
  if (event->type != GE_MOUSEMOTION)
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
    case GE_MOUSEBUTTONDOWN:
      cal_button(event->button.which, event->button.button);
      break;
    case GE_KEYDOWN:
      cal_key(event->key.which, event->key.keysym, 1);
      break;
    case GE_KEYUP:
      cal_key(event->key.which, event->key.keysym, 0);
      break;
  }

  macro_lookup(event);

  return 0;
}

int main(int argc, char *argv[])
{
  GE_Event kgevent = {.type = GE_KEYDOWN};
  int i;

  (void) signal(SIGINT, terminate);

  setlocale( LC_ALL, "" );
#ifndef WIN32
  bindtextdomain( "gimx", "/usr/share/locale" );
#else
  bindtextdomain( "gimx", "share/locale" );
#endif
  textdomain( "gimx" );

  setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */
  
#ifndef WIN32
  setlinebuf(stdout);

  emuclient_params.homedir = getpwuid(getuid())->pw_dir;
#else
  static char path[MAX_PATH];
  if(SHGetFolderPath( NULL, CSIDL_APPDATA , NULL, 0, path ))
  {
    fprintf(stderr, "Can't get the user directory.\n");
    goto QUIT;
  }
  emuclient_params.homedir = path;
#endif

  set_prio();

  for(i = 0; i < SA_MAX; ++i)
  {
    max_unsigned_axis_value[i] = DEFAULT_MAX_AXIS_VALUE;
  }
  max_unsigned_axis_value[sa_acc_x] = 1023;
  max_unsigned_axis_value[sa_acc_y] = 1023;
  max_unsigned_axis_value[sa_acc_z] = 1023;
  max_unsigned_axis_value[sa_gyro]  = 1023;

  /*
   * This is initialized before reading the arguments
   * as the --event argument can modify the controller state.
   */
  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    sixaxis_init(state + i);
    memset(controller + i, 0x00, sizeof(s_controller));
  }

  if(args_read(argc, argv, &emuclient_params) < 0)
  {
    fprintf(stderr, _("Wrong argument.\n"));
    goto QUIT;
  }

  if(connector_init() < 0)
  {
    fprintf(stderr, _("connector_init failed\n"));
    goto QUIT;
  }

  if(emuclient_params.refresh_period == -1)
  {
    emuclient_params.refresh_period = default_refresh_period[emuclient_params.ctype];
    emuclient_params.postpone_count = 3 * DEFAULT_REFRESH_PERIOD / emuclient_params.refresh_period;
    printf(_("using default refresh period: %.02fms\n"), (double)emuclient_params.refresh_period/1000);
  }
  else if(emuclient_params.refresh_period < min_refresh_period[emuclient_params.ctype])
  {
    fprintf(stderr, "Refresh period should be at least %.02fms\n", (double)min_refresh_period[emuclient_params.ctype]/1000);
    goto QUIT;
  }

  if(emuclient_params.ctype == C_TYPE_JOYSTICK)
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

  emuclient_params.frequency_scale = (double) DEFAULT_REFRESH_PERIOD / emuclient_params.refresh_period;

  /*
   * The --event argument makes emuclient send a packet and exit.
   */
  if(emuclient_params.event)
  {
    controller[0].send_command = 1;
    connector_send();
    goto QUIT;
  }

  if (!GE_initialize())
  {
    fprintf(stderr, _("GE_initialize failed\n"));
    goto QUIT;
  }

  if(emuclient_params.grab)
  {
#ifdef WIN32
    usleep(1000000);
#endif
    GE_grab();
  }

  macros_init();

  if(read_config_file(emuclient_params.config_file) < 0)
  {
    fprintf(stderr, _("read_config_file failed\n"));
    goto QUIT;
  }

  if(GE_GetMKMode() == GE_MK_MODE_SINGLE_INPUT)
  {
    free_config();
    GE_FreeMKames();
    read_config_file(emuclient_params.config_file);
  }

  GE_release_unused();

  macros_read();

  if(emuclient_params.keygen)
  {
    kgevent.key.keysym = GE_KeyId(emuclient_params.keygen);
    if(kgevent.key.keysym)
    {
      macro_lookup(&kgevent);
    }
    else
    {
      fprintf(stderr, _("Unknown key name for argument --keygen: '%s'\n"), emuclient_params.keygen);
      goto QUIT;
    }
  }

  cfg_trigger_init();

  mainloop();

  gprintf(_("Exiting\n"));

  QUIT:

  free_macros();
  free_config();
  GE_quit();
  connector_clean();

  xmlCleanupParser();

  if(emuclient_params.curses)
  {
    display_end();
  }

  return 0;
}
