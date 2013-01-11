/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SIXAXIS_H
#define SIXAXIS_H
 
#include <stdint.h>
#include <stdbool.h>

#define TS_MAX 2
#define TS_AXIS_MAX 2
 
#define HID_HANDSHAKE 0x0
#define HID_GET_REPORT 0x4
#define HID_SET_REPORT 0x5
#define HID_DATA 0xA

#define HID_TYPE_RESERVED 0
#define HID_TYPE_INPUT 1
#define HID_TYPE_OUTPUT 2
#define HID_TYPE_FEATURE 3

typedef enum sixaxis_axis_index {
    sa_lstick_x = 0, sa_lstick_y,
    sa_rstick_x, sa_rstick_y,
    sa_acc_x,
    sa_acc_y,
    sa_acc_z,
    sa_gyro,
    sa_select, sa_start, sa_ps,
    sa_up, sa_right, sa_down, sa_left,
    sa_triangle, sa_circle, sa_cross, sa_square,
    sa_l1, sa_r1,
    sa_l2, sa_r2,
    sa_l3, sa_r3,
    SA_MAX
} e_sixaxis_axis_index;

enum led_state_t { LED_OFF = 0, LED_FLASH, LED_ON };

struct sixaxis_state_sys {
    /*** Values provided by the system (PS3): */
    int reporting_enabled;
    int feature_ef_byte_6;

    /* led[0] is the spare */
    /* led[1]..led[4] correspond to printed text 1..4 */
    enum led_state_t led[5];
    uint8_t rumble[2];
};

struct sixaxis_state_user {
    /*** Values provided by the user (controller): */
    int axis[SA_MAX];
};

struct sixaxis_state {
    struct sixaxis_state_sys sys;
    struct sixaxis_state_user user;
};

struct sixaxis_assemble_t {
    int type;
    uint8_t report;
    int (*func)(uint8_t *buf, int maxlen, struct sixaxis_state *state);
};

struct sixaxis_process_t {
    int type;
    uint8_t report;
    int (*func)(const uint8_t *buf, int len, struct sixaxis_state *state);
};

extern struct sixaxis_assemble_t sixaxis_assemble[];
extern struct sixaxis_process_t sixaxis_process[];
extern char bdaddr_src[18];
extern int sixaxis_number;
extern const int digital_order[17];
extern const int analog_order[12];

void sixaxis_init(struct sixaxis_state *state);
int sixaxis_periodic_report(struct sixaxis_state *state);

int assemble_input_01(uint8_t*, int, struct sixaxis_state*);

int clamp(int, int, int);

typedef struct {
    int value;
    int index;
} s_axis_index;

const char* get_axis_name(int);
int get_button_index_from_name(const char*);
s_axis_index get_axis_index_from_name(const char*);

#endif
