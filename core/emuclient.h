/*
 * emuclient.h
 *
 *  Created on: 10 mai 2012
 *      Author: matlo
 */

#ifndef EMUCLIENT_H_
#define EMUCLIENT_H_

#include "config.h"

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

#define gprintf(...) if(display) printf(__VA_ARGS__)

#endif /* EMUCLIENT_H_ */
