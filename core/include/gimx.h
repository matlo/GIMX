/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef GIMX_H_
#define GIMX_H_

#include <GE.h>

#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

/*
 * Controllers are listening from TCP_PORT to TCP_PORT+MAX_CONTROLLERS-1
 */
#define TCP_PORT 21313

#define DEFAULT_REFRESH_PERIOD 11250 //=11.25ms

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
  char* config_file;
  int postpone_count;
  int subpositions;
  int window_events;
} s_gimx_params;

extern s_gimx_params gimx_params;

#define gprintf(...) if(gimx_params.status) printf(__VA_ARGS__)

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#endif /* GIMX_H_ */
