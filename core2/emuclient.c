/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 Copyright (c) 2009 Jim Paris <jim@jtan.com>
 License: GPLv3
 */

#include <prio.h>
#include <signal.h>
#include <errno.h>

#ifndef WIN32
#include <termio.h>
#include <sys/ioctl.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pwd.h>
#include <sys/resource.h>
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
#include <GE.h>
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
#include "mainloop.h"
#include "connector.h"
#include "args.h"

#include <locale.h>

#define EVENT_BUFFER_SIZE 256
#define DEFAULT_POSTPONE_COUNT 3
#define DEFAULT_MAX_AXIS_VALUE 255
#define DEFAULT_AXIS_SCALE 1

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

void terminate(int sig)
{
  set_done();
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

  if (!GE_initialize())
  {
    fprintf(stderr, "GE_initialize: %s\n", strerror(errno));
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
    fprintf(stderr, "read_config_file error: %s\n", strerror(errno));
    goto QUIT;
  }

  if(merge_all_devices)
  {
    free_config();
    GE_free_mouse_keyboard_names();
    read_config_file(emuclient_params.config_file);
  }

  GE_release_unused();

  macros_read();

  if(connector_init(emuclient_params.ctype, emuclient_params.portname) < 0)
  {
    fprintf(stderr, "connector_init: %s\n", strerror(errno));
    goto QUIT;
  }

  if(emuclient_params.keygen)
  {
    kgevent.key.keysym = get_key_from_buffer(emuclient_params.keygen);
    if(kgevent.key.keysym)
    {
      macro_lookup(&kgevent);
    }
    else
    {
      fprintf(stderr, "Unknown key name for argument --keygen: '%s'\n", emuclient_params.keygen);
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
