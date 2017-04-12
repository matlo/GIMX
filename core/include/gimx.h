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
  struct {
      int ff_lg;
      int ff_conv;
      int ff_common;
      int adapter;
  } debug;
  char* config_file;
  int postpone_count;
  int subpositions;
  int window_events;
  int network_input;
  int btstack;
  char * logfilename;
  FILE * logfile;
  int skip_leds;
  int record;
  int play;
  char* events_file;
} s_gimx_params;

extern s_gimx_params gimx_params;
extern uint64_t gimx_timer_start;

#define gprintf(...) if(gimx_params.status) printf(__VA_ARGS__)
#define ncprintf(...) if(!gimx_params.curses) printf(__VA_ARGS__)
#define eprintf(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg)

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif

void gimx_start_timer();
uint64_t timeval_to_usec(struct timeval tv);

#endif /* GIMX_H_ */
