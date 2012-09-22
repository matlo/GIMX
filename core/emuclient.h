/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef EMUCLIENT_H_
#define EMUCLIENT_H_

#include "config.h"

#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

#ifndef WIN32
extern char* homedir;
#endif

extern int refresh_rate;
extern int mean_axis_value;
extern int max_axis_value;
extern double axis_scale;
extern double frequency_scale;
extern int display;
extern int curses;
extern char* config_file;
extern int check_config;
extern struct sixaxis_state state[MAX_CONTROLLERS];
extern s_controller controller[MAX_CONTROLLERS];
extern int postpone_count;
extern int subpos;
extern int proc_time;
extern int proc_time_worst;
extern int proc_time_total;

int get_max_axis_value(int);

#define gprintf(...) if(display) printf(__VA_ARGS__)

#endif /* EMUCLIENT_H_ */
