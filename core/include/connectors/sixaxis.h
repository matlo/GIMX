/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SIXAXIS_H
#define SIXAXIS_H
 
#include <stdint.h>
#include <stdbool.h>
#include <controllers/ds3.h>

#define DS3_DEVICE_CLASS 0x508

#define PSM_HID_CONTROL   0x0011
#define PSM_HID_INTERRUPT 0x0013

#define HID_HANDSHAKE 0x0
#define HID_GET_REPORT 0x4
#define HID_SET_REPORT 0x5
#define HID_DATA 0xA

#define HID_TYPE_RESERVED 0
#define HID_TYPE_INPUT 1
#define HID_TYPE_OUTPUT 2
#define HID_TYPE_FEATURE 3

enum led_state_t { LED_OFF = 0, LED_FLASH, LED_ON };

struct sixaxis_state_sys {
    /*** Values provided by the system (PS3): */
    int reporting_enabled;
    int shutdown;
    int feature_ef_byte_6;

    /* led[0] is the spare */
    /* led[1]..led[4] correspond to printed text 1..4 */
    enum led_state_t led[5];
    uint8_t rumble[2];
};

struct sixaxis_state {
    char bdaddr_src[18];
    char bdaddr_dst[18];
    int dongle_index;
    int sixaxis_number;
    struct sixaxis_state_sys sys;
    s_report_ds3 user;
    int control;
    int interrupt;
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

void sixaxis_init(int sixaxis_number);
void sixaxis_set_bdaddr(int sixaxis_number, char* dst);
void sixaxis_set_dongle(int sixaxis_number, int dongle_index);
int sixaxis_connect(int sixaxis_number);
void sixaxis_close(int sixaxis_number);

int sixaxis_periodic_report(struct sixaxis_state *state);

int assemble_input_01(uint8_t*, int, struct sixaxis_state*);

int send_report(int fd, uint8_t type, uint8_t report,
        struct sixaxis_state *state, int blocking);

int process_report(uint8_t type, uint8_t report, const uint8_t *buf, int len, struct sixaxis_state *ps);

int process(int psm, const unsigned char *buf, int len, struct sixaxis_state *ps);

int send_interrupt(int sixaxis_number, s_report_ds3* buf);

#endif
