/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include "config.h"

#ifdef WIN32
#define KEY_LEFTCTRL   SDLK_LCTRL
#define KEY_RIGHTCTRL  SDLK_RCTRL
#define KEY_LEFTSHIFT  SDLK_LSHIFT
#define KEY_RIGHTSHIFT SDLK_RSHIFT
#define KEY_LEFTALT    SDLK_LALT
#define KEY_RIGHTALT   SDLK_MODE
#define KEY_ESC        SDLK_ESCAPE
#define KEY_F1         SDLK_F1
#define KEY_F2         SDLK_F2
#define KEY_F3         SDLK_F3
#define KEY_F4         SDLK_F4
#define KEY_F5         SDLK_F5
#define KEY_F6         SDLK_F6
#define KEY_F7         SDLK_F7
#define KEY_F8         SDLK_F8
#define KEY_F9         SDLK_F9
#define KEY_F10        SDLK_F10
#define KEY_F11        SDLK_F11
#define KEY_F12        SDLK_F12
#endif

typedef enum
{
  NONE,
  MC,
  CC,
  MX,
  MY,
  DZX,
  DZY,
  DZS,
  RD,
  VEL,
  EX,
  EY,
  TEST
}e_current_cal;

extern int current_mouse;
extern int current_conf;
extern e_current_cal current_cal;

void cal_button(int, int);
void cal_key(int, int, int);
inline s_mouse_cal* cal_get_mouse(int, int);
int cal_skip_event(SDL_Event*);
void cal_init();
inline int cal_get_controller(int);
inline void cal_set_controller(int, int);
void calibration_test();

#endif /* CALIBRATION_H_ */
