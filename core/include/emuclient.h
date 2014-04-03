/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef EMUCLIENT_H_
#define EMUCLIENT_H_

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
  int subpos;
} s_emuclient_params;

extern s_emuclient_params emuclient_params;

extern int proc_time;
extern int proc_time_worst;
extern int proc_time_total;

#define gprintf(...) if(emuclient_params.status) printf(__VA_ARGS__)

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#endif /* EMUCLIENT_H_ */
