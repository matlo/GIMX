/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef EMUCLIENT_H_
#define EMUCLIENT_H_

#include <config.h>

#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

#define DEFAULT_REFRESH_PERIOD 11250 //=11.25ms

typedef enum
{
  C_TYPE_JOYSTICK = 0,
  C_TYPE_360_PAD = 1,
  C_TYPE_SIXAXIS = 2,
  C_TYPE_PS2_PAD = 3,
  C_TYPE_XBOX_PAD = 4,
  C_TYPE_GPP = 5,
  C_TYPE_DEFAULT = 6,
  C_TYPE_MAX
} e_controller_type;

typedef struct
{
  e_controller_type ctype;
  char* homedir;
  char* portname;
  int force_updates;
  char* keygen;
  int grab;
  int refresh_period;
  int max_axis_value;
  double frequency_scale;
  int status;
  int curses;
  char* config_file;
  char* ip;
  int postpone_count;
  int subpos;
} s_emuclient_params;

extern s_emuclient_params emuclient_params;

extern struct sixaxis_state state[MAX_CONTROLLERS];
extern s_controller controller[MAX_CONTROLLERS];
extern int proc_time;
extern int proc_time_worst;
extern int proc_time_total;

inline int get_max_signed(int);
inline int get_max_unsigned(int);
inline int get_mean_unsigned(int);
inline double get_axis_scale(int);

#define gprintf(...) if(emuclient_params.status) printf(__VA_ARGS__)

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#endif /* EMUCLIENT_H_ */
