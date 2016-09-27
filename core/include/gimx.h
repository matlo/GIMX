/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GIMX_H_
#define GIMX_H_

#include <ginput.h>
#include <stdio.h>

#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

#define MAX_CONTROLLERS 7
#define MAX_CONFIGURATIONS 8
#define MAX_DEVICES 256
#define MAX_CONTROLS 256

/*
 * Controllers are listening from TCP_PORT to TCP_PORT+MAX_CONTROLLERS-1
 */
#define TCP_PORT 21313

typedef struct
{
  char* homedir;
  int force_updates;
  char* keygen;
  int grab;
  int refresh_period;
  double frequency_scale;
  int status;
  int curses;
  int debug;
  char* config_file;
  int postpone_count;
  int subpositions;
  int window_events;
  int network_input;
  int btstack;
  char * logfilename;
  FILE * logfile;
} s_gimx_params;

extern s_gimx_params gimx_params;

#define gprintf(...) if(gimx_params.status) printf(__VA_ARGS__)
#define dprintf(...) if(gimx_params.debug) printf(__VA_ARGS__)

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#endif /* GIMX_H_ */
