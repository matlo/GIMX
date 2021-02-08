/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <locale.h> //internationalization
#include <signal.h> //to catch SIGINT
#include <errno.h> //to print errors
#include <string.h> //to print errors
#include <limits.h> //PATH_MAX

#ifndef WIN32
#include <termios.h> //to disable/enable echo
#include <unistd.h>
#else
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_VERSION_FROM_WIN32_WINNT(NTDDI_VISTA)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
#endif

#include "gimx.h"
#include "macros.h"
#include "config_reader.h"
#include "calibration.h"
#include "control.h"
#include "display.h"
#include "mainloop.h"
#include "connectors/bluetooth/bt_abs.h"
#include "connectors/usb_con.h"
#include "args.h"
#include <controller.h>
#include <stats.h>
#include <gimxgpp/pcprog.h>
#include "../directories.h"
#include <gimxprio/include/gprio.h>
#include <gimxusb/include/gusb.h>
#include <gimxlog/include/glog.h>
#include <gimxfile/include/gfile.h>

#define DEFAULT_POSTPONE_COUNT 3 //unit = DEFAULT_REFRESH_PERIOD

s_gimx_params gimx_params =
{
  .homedir = NULL,
  .force_updates = 0,
  .keygen = NULL,
  .grab = 1,
  .refresh_period = -1,
  .frequency_scale = 1,
  .status = 0,
  .curses = 0,
  .curses_status = 0,
  .debug = { 0 },
  .config_file = NULL,
  .postpone_count = DEFAULT_POSTPONE_COUNT,
  .subpositions = 0,
  .window_events = 0,
  .btstack = 0,
  .logfilename = NULL,
  .logfile = NULL,
  .skip_leds = 0,
  .ff_conv = 0,
  .inactivity_timeout = 0,
  .clock_source = CLOCK_TIMER,
};

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwType)
{
    switch(dwType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:

      set_done();//signal the main thread to terminate

      //Returning would make the process exit!
      //We just make the handler sleep until the main thread exits,
      //or until the maximum execution time for this handler is reached.
      Sleep(10000);

      return TRUE;
    default:
      break;
    }
    return FALSE;
}
#endif

void terminate(int sig __attribute__((unused)))
{
  set_done();
}

void grab()
{
  if(gimx_params.grab)
  {
    ginput_grab();
  }
}

int ignore_event(GE_Event* event __attribute__((unused)))
{
  return 0;
}

int process_event(GE_Event* event)
{
  if (!gimx_params.config_file || get_done())
  {
    return 0;
  }
  switch (event->type)
  {
    case GE_MOUSEMOTION:
    {
      cfg_process_motion_event(event);
      unsigned int device = ginput_get_device_id(event);
      int controller = adapter_get_controller(E_DEVICE_TYPE_MOUSE, device);
      if (controller != -1) {
          s_adapter * adapter = adapter_get(controller);
          if (adapter->mstats != NULL) {
            stats_update(adapter->mstats);
          }
      }
      break;
    }
    case GE_JOYRUMBLE:
      cfg_process_rumble_event(event);
      break;
    default:
      if (!cal_skip_event(event))
      {
        cfg_process_event(event);
      }
      break;
  }

  //make sure to process the event before these two lines
  cfg_trigger_lookup(event);
  cfg_intensity_lookup(event);

  switch (event->type)
  {
    case GE_MOUSEBUTTONDOWN:
      cal_button(event->button.button);
      break;
    case GE_KEYDOWN:
      cal_key(event->key.keysym, 1);
      control_key(event->key.keysym, 1);
      break;
    case GE_KEYUP:
      cal_key(event->key.keysym, 0);
      control_key(event->key.keysym, 0);
      break;
  }

  if(event->type != GE_MOUSEMOTION)
  {
    macro_lookup(event);
  }

  return 0;
}

void show_devices()
{
  const char * name;
  int i;
  for (i = 0; i < MAX_DEVICES && (name = ginput_mouse_name(i)) != NULL; ++i)
  {
    printf("mouse %d has name '%s' and virtual id %d\n", i, name, ginput_mouse_virtual_id(i));
  }
  for (i = 0; i < MAX_DEVICES && (name = ginput_keyboard_name(i)) != NULL; ++i)
  {
    printf("keyboard %d has name '%s' and virtual id %d\n", i, name, ginput_keyboard_virtual_id(i));
  }
  for (i = 0; i < MAX_DEVICES && (name = ginput_joystick_name(i)) != NULL; ++i)
  {
    printf("joystick %d has name '%s' and virtual id %d\n", i, name, ginput_joystick_virtual_id(i));
  }
}

void show_config()
{
  if (gimx_params.config_file == NULL)
  {
    return;
  }

  char file_path[PATH_MAX];

  snprintf(file_path, sizeof(file_path), "%s%s%s%s", gimx_params.homedir, GIMX_DIR, CONFIG_DIR, gimx_params.config_file);

  FILE * fp = gfile_fopen(file_path, "r");
  if (fp == NULL)
  {
    gwarn("failed to dump %s\n", file_path);
  }
  else
  {
    printf("Dump of %s:\n", file_path);
    char line[LINE_MAX];
    while (fgets(line, sizeof(line), fp))
    {
      printf("%s", line);
    }
    fclose(fp);
  }
}

int main(int argc, char *argv[])
{
  e_gimx_status status = E_GIMX_STATUS_SUCCESS;

  GE_Event kgevent = { .key = { .type = GE_KEYDOWN } };

  glog_set_all_levels(E_GLOG_LEVEL_INFO);

  (void) signal(SIGINT, terminate);
  (void) signal(SIGTERM, terminate);
#ifndef WIN32
  (void) signal(SIGHUP, terminate);
#else
  if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == 0)
  {
    gerror("SetConsoleCtrlHandler failed\n");
    exit(-1);
  }
#endif

  setlocale( LC_ALL, "" );
#ifndef WIN32
  bindtextdomain( "gimx", "/usr/share/locale" );
#else
  bindtextdomain( "gimx", "share/locale" );
#endif
  textdomain( "gimx" );

  setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

#ifdef WIN32
  /*
   * Log file has to be in utf-8:
   * - force console output code point to utf-8
   * - use binary mode to avoid any text output processing
   * - make gettext return utf-8 strings
   */

  UINT cp = GetConsoleOutputCP();

  if (SetConsoleOutputCP(CP_UTF8))
  {
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    bind_textdomain_codeset("gimx", "utf-8");
  }
  else
  {
    gerror("SetConsoleOutputCP(CP_UTF8) failed\n");
  }
#endif

  gimx_params.homedir = gfile_homedir();

  if (gimx_params.homedir == NULL) {
    status = E_GIMX_STATUS_GENERIC_ERROR;
    goto QUIT;
  }

  gpppcprog_read_user_ids(gimx_params.homedir, GIMX_DIR);

  if(args_read(argc, argv, &gimx_params) < 0)
  {
    status = E_GIMX_STATUS_GENERIC_ERROR;
    goto QUIT;
  }

  if(gimx_params.btstack)
  {
    bt_abs_value = E_BT_ABS_BTSTACK;
  }

  status = adapter_detect();
  if(status != E_GIMX_STATUS_SUCCESS)
  {
    goto QUIT;
  }

  if(gimx_params.refresh_period == -1)
  {
    /*
     * TODO MLA: per controller refresh period?
     */
    gimx_params.refresh_period = controller_get_default_refresh_period(adapter_get(0)->ctype);
    gimx_params.postpone_count = 3 * DEFAULT_REFRESH_PERIOD / gimx_params.refresh_period;
    ginfo(_("using default refresh period: %.02fms\n"), (double)gimx_params.refresh_period/1000);
  }
  else if(gimx_params.refresh_period < controller_get_min_refresh_period(adapter_get(0)->ctype))
  {
    gerror("Refresh period should be at least %.02fms\n", (double)controller_get_min_refresh_period(adapter_get(0)->ctype)/1000);
    status = E_GIMX_STATUS_GENERIC_ERROR;
    goto QUIT;
  }

  gimx_params.frequency_scale = (double) DEFAULT_REFRESH_PERIOD / gimx_params.refresh_period;

  /*
   * The --event argument makes gimx send a packet and exit.
   */
  int event = 0;
  unsigned char controller;
  for(controller=0; controller<MAX_CONTROLLERS; ++controller)
  {
    s_adapter * adapter = adapter_get(controller);
    if(adapter->event)
    {
      adapter->send_command = 1;
      event = 1;
      if(adapter->remote.socket == NULL)
      {
        ginfo("The --event argument may require running two gimx instances.\n");
      }
    }
  }
  if(event)
  {
    if(adapter_start() < 0)
    {
      status = E_GIMX_STATUS_GENERIC_ERROR;
      goto QUIT;
    }
    adapter_send();
    goto QUIT;
  }

  if (gimx_params.config_file)
  {
    unsigned char src = GE_MKB_SOURCE_PHYSICAL;

    if(gimx_params.window_events)
    {
      src = GE_MKB_SOURCE_WINDOW_SYSTEM;
    }

    int(*fp)(GE_Event*) = NULL;

    /*
     * Non-generated events are ignored if the --keygen argument is used.
     */
    if(gimx_params.keygen)
    {
      fp = ignore_event;
    }
    else
    {
      fp = process_event;
    }
    GPOLL_INTERFACE poll_interace = {
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if (ginput_init(&poll_interace, src, fp) < 0)
    {
      status = E_GIMX_STATUS_GENERIC_ERROR;
      goto QUIT;
    }

    if (gimx_params.logfile != NULL)
    {
      show_devices();
      show_config();
    }

    cal_init();

    cfg_intensity_init();
    cfg_init_ffb_tweaks();

    if(read_config_file(gimx_params.config_file) < 0)
    {
      status = E_GIMX_STATUS_GENERIC_ERROR;
      goto QUIT;
    }

    if(ginput_get_mk_mode() == GE_MK_MODE_SINGLE_INPUT)
    {
      cfg_clean();
      ginput_free_mk_names();

      cal_init();

      cfg_intensity_init();
      cfg_init_ffb_tweaks();

      read_config_file(gimx_params.config_file);
    }

    cfg_read_calibration();

    cfg_pair_mouse_mappers();

    if(gimx_params.autograb)
    {
      gimx_params.grab = 0;
      int i;
      for (i = 0; i < MAX_CONTROLLERS; ++i)
      {
        // check if config has a keyboard binding or a mouse binding
        // in most cases window focus is required for getting keyboard/mouse events
        // if config only has joystick bindings, window focus is not required, and grabbing mouse is not needed
        if(adapter_get_device(E_DEVICE_TYPE_MOUSE, i) != -1 || adapter_get_device(E_DEVICE_TYPE_KEYBOARD, i) != -1)
        {
            gimx_params.grab = 1;
        }
      }
    }

    grab();

    ginput_release_unused();

    macros_init();

    if(gimx_params.keygen)
    {
      kgevent.key.keysym = ginput_key_id(gimx_params.keygen);
      if(kgevent.key.keysym)
      {
        macro_lookup(&kgevent);
      }
      else
      {
        gerror(_("Unknown key name for argument --keygen: '%s'\n"), gimx_params.keygen);
        status = E_GIMX_STATUS_GENERIC_ERROR;
        goto QUIT;
      }
    }

    cfg_trigger_init();
  }

  if(gimx_params.curses)
  {
    glog_set_all_levels(E_GLOG_LEVEL_NONE);
    gimx_params.curses_status = 1;

#ifdef WIN32
    /*
     * ncurses does not accomodate with utf-8 strings,
     * so make gettext to use default encoding.
     */
    bind_textdomain_codeset("gimx", "");
#endif

    display_init();
  }
#ifndef WIN32
  else if (gimx_params.logfile == NULL)
  {
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term);
  }
#endif

  if(adapter_start() < 0)
  {
    status = E_GIMX_STATUS_GENERIC_ERROR;
    goto QUIT;
  }

  usb_poll_interrupts();

  /*
   * Call gprio_init just before mainloop,
   * so that all libraries spawned the threads they need.
   */
  if (gprio_init() < 0)
  {
    gwarn("failed to set process priority\n");
  }

  e_gimx_status mstatus = mainloop();
  if (mstatus != E_GIMX_STATUS_SUCCESS)
  {
    status = mstatus;
  }

  gprio_clean();

  ginfo(_("Exiting\n"));

  QUIT: ;

  e_gimx_status clean_status = adapter_clean();
  if (status == E_GIMX_STATUS_SUCCESS && clean_status != E_GIMX_STATUS_SUCCESS)
  {
    status = clean_status;
  }

  if (gimx_params.config_file)
  {
    macros_clean();
    cfg_clean();
    ginput_quit();

    xmlCleanupParser();
  }

  if (status != E_GIMX_STATUS_SUCCESS)
  {
    /*
     * Write the status in the gimx.status file, in the system temp directory.
     *
     * In most cases gimx runs in a terminal window (such as xterm) that may not
     * provide the return code to the parent process (in most cases gimx-launcher).
     *
     * The absence of the gimx.status file means the execution was successful,
     * or that the program crashed.
     */

    char file[PATH_MAX + 1] = {};
    char * tmp = gfile_tempdir();
    if (strlen(tmp) + sizeof("/gimx.status") <= sizeof(file))
    {
      sprintf(file, "%s/gimx.status", tmp);
    }
    free(tmp);
    if (file != NULL && file[0] != '\0')
    {
      FILE * fp = gfile_fopen(file, "w");
      if (fp != NULL)
      {
        fprintf(fp, "%d\n", status);
        fclose(fp);
#ifndef WIN32
        if (gfile_makeown(file) < 0)
        {
          gerror("failed to set ownership of the gimx status file\n");
        }
#endif
      }
    }
  }

  if(gimx_params.logfile)
  {
    fclose(gimx_params.logfile);
#ifndef WIN32
    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s%s%s%s", gimx_params.homedir, GIMX_DIR, LOG_DIR, gimx_params.logfilename);
    if (gfile_makeown(file_path) < 0)
    {
      gerror("failed to set ownership of the gimx log file\n");
    }
#endif
  }

  if(gimx_params.curses)
  {
    display_end();

#ifdef WIN32
    bind_textdomain_codeset("gimx", "utf-8");
#endif
  }
#ifndef WIN32
  else if (gimx_params.logfile == NULL)
  {
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag |= ECHO;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term);
  }
#endif

#ifdef WIN32
  if (cp && !SetConsoleOutputCP(cp))
  {
    gerror("SetConsoleOutputCP(cp) failed\n");
  }
#endif

  free(gimx_params.homedir);

  return status;
}
