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

    c = getopt_long (argc, argv, "c:i:k:p:r:t:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
    break;

    switch (c)
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf("option %s", long_options[option_index].name);
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 'c':
        params->config_file = optarg;
        printf("option -c with value `%s'\n", optarg);
        break;

      case 'i':
        params->ip = optarg;
        printf("option -i with value `%s'\n", optarg);
        break;

      case 'k':
        params->keygen = optarg;
        printf("option -k with value `%s'\n", optarg);
        break;

      case 'p':
        params->portname = optarg;
        printf("option -p with value `%s'\n", optarg);
        break;

      case 'r':
        params->refresh_period = atof(optarg) * 1000;
        if(params->refresh_period)
        {
          params->postpone_count = 3 * DEFAULT_REFRESH_PERIOD / params->refresh_period;
          printf("option -r with value `%s'\n", optarg);
        }
        else
        {
          fprintf(stderr, "Bad refresh period: %s\n", optarg);
          ret = -1;
        }
        break;

      case 't':
        printf("option -t with value `%s'\n", optarg);
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
        printf("unrecognized option: -c\n");
        ret = -1;
        break;
    }
  }

  if(params->status)
    params->curses = 0;

  if(!params->grab)
    printf("grab flag is unset\n");
  if(params->status)
    printf("status flag is set\n");
  if(params->subpos)
    printf("subpos flag is set\n");
  if(params->force_updates)
    printf("force_updates flag is set\n");
  if(params->curses)
    printf("curses flag is set\n");

  return ret;
}
