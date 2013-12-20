/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"
#include "emuclient.h"
#include <getopt.h>
#include <controllers/controller.h>
#include <controllers/ds3.h>
#include <netinet/in.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif

/*
 * Try to parse an argument with the following expected format: a.b.c.d:e
 * where a.b.c.d is an IPv4 address and e is a port.
 */
static int read_ip(char* optarg, unsigned int* ip, unsigned short* port)
{
  int ret = 0;
  char* sep = strchr(optarg, ':');
  if (sep)
  {
    *sep = ' ';//Temporarily separate the address and the port
    *ip = inet_addr(optarg);
    *port = atoi(sep + 1);
    *sep = ':';//Revert.
  }
  if (!sep || *ip == INADDR_NONE || *port == 0)
  {
    ret = -1;
  }
  return ret;
}

int args_read(int argc, char *argv[], s_emuclient_params* params)
{
  int ret = 0;
  int c;
  unsigned char controller = 0;

  struct option long_options[] =
  {
    /* These options set a flag. */
    {"nograb",         no_argument, &params->grab,           0},
    {"status",         no_argument, &params->status,         1},
    {"subpos",         no_argument, &params->subpos,         1},
    {"force-updates",  no_argument, &params->force_updates,  1},
    {"curses",         no_argument, &params->curses,         1},
    /* These options don't set a flag. We distinguish them by their indices. */
    {"config",  required_argument, 0, 'c'},
    {"dst",     required_argument, 0, 'd'},
    {"event",   required_argument, 0, 'e'},
    {"keygen",  required_argument, 0, 'k'},
    {"port",    required_argument, 0, 'p'},
    {"refresh", required_argument, 0, 'r'},
    {"src",     required_argument, 0, 's'},
    {"type",    required_argument, 0, 't'},
    {0, 0, 0, 0}
  };

  while (1)
  {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "c:d:e:i:k:p:r:s:t:", long_options, &option_index);

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

      case 'c':
        params->config_file = optarg;
        printf(_("option -c with value `%s'\n"), optarg);
        break;

      case 'e':
        {
          char axis_label[9] = {};
          int axis;
          int value;
          if(sscanf(optarg, "%8[^(](%d)", axis_label, &value) != 2)
          {
            fprintf(stderr, _("Bad event format: %s\n"), optarg);
            ret = -1;
          }
          else
          {
            if((axis = ds3_get_button_index_from_name(axis_label)) != -1)
            {
              printf(_("option -e with value `%s(%d)'\n"), axis_label, value);
              set_axis_value(controller, axis, value);
              get_controller(controller)->event = 1;
            }
            else
            {
              fprintf(stderr, _("Bad axis name for event: %s\n"), optarg);
              ret = -1;
            }
          }
        }
        break;

      case 'd':
        if(read_ip(optarg, &get_controller(controller)->dst_ip,
            &get_controller(controller)->dst_port) < 0)
        {
          printf(_("Bad format for argument -d: '%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          printf(_("option -d with value `%s'\n"), optarg);
        }
        break;

      case 's':
        if(read_ip(optarg, &get_controller(controller)->src_ip,
            &get_controller(controller)->src_port) < 0)
        {
          printf(_("Bad format for argument -s: '%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          printf(_("option -s with value `%s'\n"), optarg);
        }
        break;

      case 'k':
        params->keygen = optarg;
        printf(_("option -k with value `%s'\n"), optarg);
        break;

      case 'p':
        if(controller_set_port(controller, optarg) < 0)
        {
          printf(_("port already used: `%s'\n"), optarg);
          ret = -1;
        }
        else
        {
          ++controller;
          printf(_("option -p with value `%s'\n"), optarg);
        }
        break;

      case 'r':
        params->refresh_period = atof(optarg) * 1000;
        if(params->refresh_period)
        {
          params->postpone_count = 3 * DEFAULT_REFRESH_PERIOD / params->refresh_period;
          printf(_("option -r with value `%s'\n"), optarg);
        }
        else
        {
          fprintf(stderr, "Bad refresh period: %s\n", optarg);
          ret = -1;
        }
        break;

      case 't':
        printf(_("option -t with value `%s'\n"), optarg);
        if (!strcmp(optarg, "joystick"))
        {
          get_controller(controller)->type = C_TYPE_JOYSTICK;
        }
        else if (!strcmp(optarg, "360pad"))
        {
          get_controller(controller)->type = C_TYPE_360_PAD;
        }
        else if (!strcmp(optarg, "Sixaxis"))
        {
          get_controller(controller)->type = C_TYPE_SIXAXIS;
        }
        else if (!strcmp(optarg, "PS2pad"))
        {
          get_controller(controller)->type = C_TYPE_PS2_PAD;
        }
        else if (!strcmp(optarg, "GPP"))
        {
          get_controller(controller)->type = C_TYPE_GPP;
        }
        else if (!strcmp(optarg, "XboxPad"))
        {
          get_controller(controller)->type = C_TYPE_XBOX_PAD;
        }
        else if (!strcmp(optarg, "DS4"))
        {
          get_controller(controller)->type = C_TYPE_DS4;
        }
        else
        {
          fprintf(stderr, "Bad controller type: %s\n", optarg);
          ret = -1;
        }
        break;

      case '?':
        /* getopt_long already printed an error message. */
        break;

      default:
        printf(_("unrecognized option: %c\n"), c);
        ret = -1;
        break;
    }
  }

  if(params->status)
    params->curses = 0;

  if(!params->grab)
    printf(_("grab flag is unset\n"));
  if(params->status)
    printf(_("status flag is set\n"));
  if(params->subpos)
    printf(_("subpos flag is set\n"));
  if(params->force_updates)
    printf(_("force_updates flag is set\n"));
  if(params->curses)
    printf(_("curses flag is set\n"));

  return ret;
}
