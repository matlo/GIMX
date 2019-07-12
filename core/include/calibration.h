/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include "config.h"

#define GSTEPS_MAX 8
#define GWINDOWS_MAX 8
#define GMODES_MAX 8

typedef enum {
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
} e_current_cal;

typedef enum {
    STEP_1,
    STEP_2,
    STEP_3,
    STEP_4,
    STEP_5
} e_cal_steps;

typedef enum {
    MODE_BASIC,
    MODE_ADVANCED,
    MODE_EXPERT,
    MODE_STATUS
} e_cal_modes;

struct gcalibration {
    e_current_cal current;
    s_mouse_cal* mouse_data;
    int mouse;
    int profile;
    e_cal_steps step;
    e_cal_modes mode;
    int cal_modes_max_step[4];
};

void cal_status();
void cal_button(int);
void cal_key(int, int);
s_mouse_cal* cal_get_mouse(int mouse, int conf);
void cal_set_mouse(s_config_entry* entry);
int cal_skip_event(GE_Event*);
void cal_init();
int cal_get_controller(int);
void cal_set_controller(int, int);
void calibration_test();

int calibration_get_mouse();
void calibration_set_mouse(int mouse);
e_current_cal calibration_get_current();

#endif /* CALIBRATION_H_ */
