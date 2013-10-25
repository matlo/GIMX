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

int args_read(int argc, char *argv[], s_emuclient_params* params)
{
  int ret = 0;
  int c;

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
    {"event",   required_argument, 0, 'e'},
    {"ip",      required_argument, 0, 'i'},
    {"keygen",  required_argument, 0, 'k'},
    {"port",    required_argument, 0, 'p'},
    {"refresh", required_argument, 0, 'r'},
    {"type",    required_argument, 0, 't'},
    {0, 0, 0, 0}
  };

  while (1)
  {
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "c:e:i:k:p:r:t:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
    break;

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
            if((axis = get_button_index_from_name(axis_label)) != -1)
            {
              printf(_("option -e with value `%s(%d)'\n"), axis_label, value);
              set_axis_value(axis, value);
              params->event = 1;
            }
            else
            {
              fprintf(stderr, _("Bad axis name for event: %s\n"), optarg);
              ret = -1;
            }
          }
        }
        break;

      case 'i':
        params->ip = optarg;
        printf(_("option -i with value `%s'\n"), optarg);
        break;

      case 'k':
        params->keygen = optarg;
        printf(_("option -k with value `%s'\n"), optarg);
        break;

      case 'p':
        params->portname = optarg;
        printf(_("option -p with value `%s'\n"), optarg);
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
          params->ctype = C_TYPE_JOYSTICK;
        }
        else if (!strcmp(optarg, "360pad"))
        {
          params->ctype = C_TYPE_360_PAD;
        }
        else if (!strcmp(optarg, "Sixaxis"))
        {
          params->ctype = C_TYPE_SIXAXIS;
        }
        else if (!strcmp(optarg, "PS2pad"))
        {
          params->ctype = C_TYPE_PS2_PAD;
        }
        else if (!strcmp(optarg, "GPP"))
        {
          params->ctype = C_TYPE_GPP;
        }
        else if (!strcmp(optarg, "XboxPad"))
        {
          params->ctype = C_TYPE_XBOX_PAD;
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
