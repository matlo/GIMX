/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"
#include "gimx.h"
#include "../info.h"
#include <getopt.h>
#include <adapter.h>
#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <connectors/protocol.h>
#include "../directories.h"
#include <unistd.h>
#include <limits.h>
#include "log.h"

#define DEV_HIDRAW "/dev/hidraw"
#ifndef WIN32
#define DEV_SERIAL "/dev/tty"
#else
#define DEV_SERIAL "COM"
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static void usage()
{
#ifndef WIN32
  printf("PS3+bluetooth: gimx --type Sixaxis --config filename --hci bt_device_index --bdaddr ps3_bt_address\n");
  printf("  filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");
  printf("  bt_device_index: The bluetooth device index (ex: 0 for hci0, 1 for hci1, etc). Optional, default value is 0.\n");
  printf("  ps3_bt_address: The bluetooth device address of the PS3.\n");

  printf("PS4+bluetooth: gimx --type DS4 --config filename --hci bt_device_index --bdaddr ps4_bt_address\n");
  printf("  filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");
  printf("  bt_device_index: The bluetooth device index (ex: 0 for hci0, 1 for hci1, etc). Optional, default value is 0.\n");
  printf("  ps4_bt_address: The bluetooth device address of the PS4.\n");
#endif
  printf("DIY USB adapter: gimx --config filename --port portname\n");
  printf("  filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");
  printf("  portname: The serial port. Ex: /dev/ttyUSB0 in Linux, COM4 in Windows.\n");

  printf("GPP/Cronus/Titan: gimx --config filename --type GPP\n");
  printf("  filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");

  printf("Remote GIMX: gimx --config filename --dst IP:port\n");
  printf("  filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");
  printf("  IP:port: The destination IP+port. Ex: 127.0.0.1:51914.\n");

  printf("Multiple controllers:\n");
  printf("  A --bdaddr, --port or --dst argument finishes the current controller options.\n");
  printf("  Further non-global options apply to further controller instances.\n");

  printf("Controller options:\n");
  printf("  --event \"control(value)\": send controls to the console and exit.\n");
  printf("    Names and value ranges: \"lstick x\", \"lstick y\", \"rstick x\", \"rstick y\": [-128,127]\n");
  printf("    \"acc x\", \"acc y\", \"acc z\", \"gyro\": [-512,511]\n");
  printf("    \"select\", \"start\", \"PS\", \"l3\", \"r3\": {0, 255}\n");
  printf("    \"up\", \"right\", \"down\", \"left\", \"triangle\", \"circle\", \"cross\", \"square\", \"l1\", \"r1\", \"l2\", \"r2\": [0,255]\n");
  printf("  --src IP:port: Specifies a source IP+port to listen on. Ex: 127.0.0.1:51914.\n");

  printf("Global options:\n");
  printf("  These options apply to all controller instances.\n");
  printf("  --config filename: The name of the config file, in the ~/.gimx/config directory (ex: \"File name.xml\").\n");
  printf("  --curses: Curses terminal display. Mouse calibration is available through this interface.\n");
  printf("  --status: Display controls in the terminal.\n");
  printf("  --nograb: Do not grab the mouse cursor.\n");
  printf("  --force-updates: Send button+axis status even if there is no change.\n");
  printf("  --subpos: Improve stick precision.\n");
  printf("  --window-events : Read window events instead of hardware events.\n");
  printf("  --keygen key: Generate a key press at gimx startup.\n");
  printf("  --refresh n: The refresh period, in ms. Forcing the refresh period is not recommended.\n");
  printf("  --btstack: use btstack for the bluetooth connection.\n");
  printf("    Btstack is the only available connection method on Windows, and an alternative connection method on Linux.\n");
  printf("  --log filename: write messages into a log file instead of the standard output.\n");
  printf("    filename: The name of the log file, in the ~/.gimx/log directory (make sure this folder exists).\n");
  printf("  --skip_leds: Filter out set led commands from FFB command stream (performance tweak for G27/G29 wheels on small targets).\n");
}

/*
 * Try to parse an argument with the following expected format: a.b.c.d:e
 * where a.b.c.d is an IPv4 address and e is a port.
 */
static int read_ip(char* optarg, in_addr_t* ip, unsigned short* port)
{
  int ret = 0;
  int pos;
  size_t len = strlen(optarg);
  //check the length
  if(len + 1 > sizeof("111.111.111.111:65535"))
  {
    return -1;
  }
  //check the absence of spaces
  if(strchr(optarg, ' '))
  {
    return -1;
  }
  //get the position of the ':'
  char* sep = strchr(optarg, ':');
  if (sep)
  {
    *sep = ' ';//Temporarily separate the address and the port
    *ip = inet_addr(optarg);//parse the IP
    //parse the port
    if(sscanf(sep + 1, "%hu%n", port, &pos) != 1 || (unsigned int)pos != (len - (sep + 1 - optarg)))
    {
      ret = -1;
    }
    *sep = ':';//Revert.
  }
  if (!sep || *ip == INADDR_NONE || *port == 0)
  {
    ret = -1;
  }
  return ret;
}

int args_read(int argc, char *argv[], s_gimx_params* params)
{
  int ret = 0;
  int c;
  unsigned char controller = 0;
  unsigned char input = 0;

  struct option long_options[] =
  {
    /* These options set a flag. */
    {"nograb",         no_argument, &params->grab,           0},
    {"status",         no_argument, &params->status,         1},
    {"subpos",         no_argument, &params->subpositions,   1},
    {"force-updates",  no_argument, &params->force_updates,  1},
    {"curses",         no_argument, &params->curses,         1},
    {"window-events",  no_argument, &params->window_events,  1},
    {"btstack",        no_argument, &params->btstack,        1},
    {"debug.ff_lg",    no_argument, &params->debug.ff_lg,    1},
    {"debug.ff_conv",  no_argument, &params->debug.ff_conv,  1},
    {"debug.adapter",  no_argument, &params->debug.adapter,  1},
    {"debug.macros",   no_argument, &params->debug.macros,   1},
    {"debug.sixaxis",  no_argument, &params->debug.sixaxis,  1},
    {"skip_leds",      no_argument, &params->skip_leds,      1},
    /* These options don't set a flag. We distinguish them by their indices. */
    {"bdaddr",  required_argument, 0, 'b'},
    {"config",  required_argument, 0, 'c'},
    {"dst",     required_argument, 0, 'd'},
    {"event",   required_argument, 0, 'e'},
    {"hci",     required_argument, 0, 'h'},
    {"help",    no_argument,       0, 'm'},
    {"keygen",  required_argument, 0, 'k'},
    {"log",     required_argument, 0, 'l'},
    {"port",    required_argument, 0, 'p'},
    {"refresh", required_argument, 0, 'r'},
    {"src",     required_argument, 0, 's'},
    {"type",    required_argument, 0, 't'},
    {"version", no_argument,       0, 'v'},
    {0, 0, 0, 0}
  };
  
  while (1)
  {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "b:c:d:e:h:k:l:p:r:s:t:vm", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
    break;

    if(controller == MAX_CONTROLLERS)
    {
      printf(_("ignoring: -%c %s (max controllers reached)\n"), c, optarg);
      continue;
    }

    switch (c)
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf(_("option %s"), long_options[option_index].name);
        if (optarg)
          printf(_(" with arg %s"), optarg);
        printf("\n");
        break;

      case 'b':
        adapter_get(controller)->atype = E_ADAPTER_TYPE_BLUETOOTH;
        adapter_get(controller)->bdaddr_dst = optarg;
        if(adapter_get(controller)->ctype == C_TYPE_NONE)
        {
          adapter_get(controller)->ctype = C_TYPE_SIXAXIS;
        }
        printf(_("controller #%d: option -b with value `%s'\n"), controller + 1, optarg);
        ++controller;
        printf(_("now reading arguments for controller #%d\n"), controller + 1);
        break;

      case 'c':
        params->config_file = optarg;
        printf(_("global option -c with value `%s'\n"), optarg);
        input = 1;
        break;

      case 'e':
        {
          char label[AXIS_NAME_MAX_SIZE];
          int axis;
          int value;
          if(sscanf(optarg, "%"STR(AXIS_NAME_MAX_SIZE)"[^(](%d)", label, &value) != 2)
          {
            gerror(_("Bad event format: %s\n"), optarg);
            ret = -1;
          }
          else
          {
            // strip trailing spaces
            char * end;
            for (end = label + strlen(label) - 1; end > label && *end == ' '; --end) {}
            *(end + 1) = '\0';
            if((axis = controller_get_axis_index(label)) != -1)
            {
              printf(_("controller #%d: option -e with value `%s(%d)'\n"), controller + 1, label, value);
              adapter_set_axis(controller, axis, value);
              adapter_get(controller)->event = 1;
              input = 1;
            }
            else
            {
              gerror(_("Bad axis name for event: %s\n"), optarg);
              ret = -1;
            }
          }
        }
        break;

      case 'h':
        adapter_get(controller)->dongle_index = atoi(optarg);
        printf(_("controller #%d: option -h with value `%d'\n"), controller + 1, adapter_get(controller)->dongle_index);
        break;

      case 'd':
        if(read_ip(optarg, &adapter_get(controller)->dst_ip,
            &adapter_get(controller)->dst_port) < 0)
        {
          gerror(_("Bad format for argument -d: '%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          adapter_get(controller)->atype = E_ADAPTER_TYPE_REMOTE_GIMX;
          printf(_("controller #%d: option -d with value `%s'\n"), controller + 1, optarg);
          ++controller;
          printf(_("now reading arguments for controller #%d\n"), controller + 1);
        }
        break;

      case 'm':
        usage();
        exit(0);
        break;

      case 's':
        if(read_ip(optarg, &adapter_get(controller)->src_ip,
            &adapter_get(controller)->src_port) < 0)
        {
          gerror(_("Bad format for argument -s: '%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          printf(_("controller #%d: option -s with value `%s'\n"), controller + 1, optarg);
          input = 1;
          params->network_input = 1;
        }
        break;

      case 'k':
        params->keygen = optarg;
        printf(_("controller #%d: option -k with value `%s'\n"), controller + 1, optarg);
        break;

      case 'l':
        if(params->logfilename == NULL)
        {
          if(!params->curses)
          {
            params->logfilename = optarg;
            if(params->logfilename && strlen(params->logfilename))
            {
              char file_path[PATH_MAX];
              snprintf(file_path, sizeof(file_path), "%s%s%s%s", params->homedir, GIMX_DIR, LOG_DIR, params->logfilename);
              params->logfile = fopen(file_path, "w");
              if(params->logfile != NULL)
              {
                dup2(fileno(params->logfile), fileno(stdout));
                dup2(fileno(params->logfile), fileno(stderr));
              }
              else
              {
                gerror(_("can't open log file (%s)\n"), file_path);
                ret = -1;
              }
            }
            printf(_("global option -l with value `%s'\n"), optarg);
          }
          else
          {
            gerror(_("log file can't be used with curses\n"));
            ret = -1;
          }
        }
        else
        {
          gerror(_("only one log file can be specified\n"));
          ret = -1;
        }
        break;

      case 'p':
        if(adapter_get(controller)->atype == E_ADAPTER_TYPE_NONE)
        {
          // no adapter type specified => try to guess it from the port
          if(strstr(optarg, DEV_HIDRAW) || !strstr(optarg, DEV_SERIAL))
          {
            adapter_get(controller)->atype = E_ADAPTER_TYPE_GPP;
          }
          else
          {
            adapter_get(controller)->atype = E_ADAPTER_TYPE_DIY_USB;
          }
        }
        if(adapter_set_port(controller, optarg) < 0)
        {
          gerror(_("port already used: `%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          printf(_("controller #%d: option -p with value `%s'\n"), controller + 1, optarg);
          ++controller;
          printf(_("now reading arguments for controller #%d\n"), controller + 1);
        }
        break;

      case 'r':
        params->refresh_period = atof(optarg) * 1000;
        if(params->refresh_period)
        {
          params->postpone_count = 3 * DEFAULT_REFRESH_PERIOD / params->refresh_period;
          printf(_("global option -r with value `%s'\n"), optarg);
        }
        else
        {
          gerror("Bad refresh period: %s\n", optarg);
          ret = -1;
        }
        break;

      case 't':
        printf(_("controller #%d: option -t with value `%s'\n"), controller + 1, optarg);
        if (!strcmp(optarg, "GPP"))
        {
          adapter_get(controller)->atype = E_ADAPTER_TYPE_GPP;
        }
        else
        {
          adapter_get(controller)->ctype = controller_get_type(optarg);
          if (adapter_get(controller)->ctype == C_TYPE_NONE)
          {
            gerror("Bad controller type: %s\n", optarg);
            ret = -1;
          }
        }
        break;

      case 'v':
        printf("GIMX %s %s\n", INFO_VERSION, INFO_ARCH);
        exit(0);
        break;

      case '?':
        usage();
        exit(-1);
        break;

      default:
        gerror(_("unrecognized option: %c\n"), c);
        ret = -1;
        break;
    }
  }

  if(params->status || params->logfilename)
    params->curses = 0;

  if(!params->grab)
    printf(_("grab flag is unset\n"));
  if(params->status)
    printf(_("status flag is set\n"));
  if(params->subpositions)
    printf(_("subpos flag is set\n"));
  if(params->force_updates)
    printf(_("force_updates flag is set\n"));
  if(params->curses)
    printf(_("curses flag is set\n"));
  if(params->window_events)
    printf(_("window_events flag is set\n"));
  if(params->btstack)
    printf(_("btstack flag is set\n"));
  if(params->skip_leds)
    printf(_("skip_leds flag is set\n"));

  if(!input)
  {
    gerror("At least a config file, an event, or a source IP:port should be specified as argument.\n")
    ret = -1;
  }

  if(params->logfile)
  {
    log_info();
  }

  if (params->debug.ff_conv != 0)
  {
    params->debug.ff_lg = 1;
  }

  return ret;
}
