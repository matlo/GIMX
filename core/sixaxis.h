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

#define DEFAULT_REFRESH_PERIOD 10000 //=10ms

struct sixaxis_button {
    /* It's possible for a button to report as being pressed
          but still have a zero value */
    bool pressed;
    /* 0 to 255 */
    uint8_t value;
};

struct sixaxis_axis {
    /* -max_axis_value to max_axis_value */
    int x;
    int y;
};

struct sixaxis_accelerometer {
    /* -512 to 511 */
    int x;
    int y;
    int z;
    int gyro;
};

typedef enum sixaxis_button_index {
    sb_select = 0, sb_start, sb_ps,
    sb_up, sb_right, sb_down, sb_left,
    sb_triangle, sb_circle, sb_cross, sb_square,
    sb_l1, sb_r1,
    sb_l2, sb_r2,
    sb_l3, sb_r3,
    SB_MAX
} e_sixaxis_button_index;

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
    struct sixaxis_button button[SB_MAX];
    int axis[TS_MAX][TS_AXIS_MAX];
    struct sixaxis_accelerometer accel;
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

void sixaxis_init(struct sixaxis_state *state);
int sixaxis_periodic_report(struct sixaxis_state *state);

int assemble_input_01(uint8_t*, int, struct sixaxis_state*);

int get_button_index_from_name(const char*);

int clamp(int, int, int);

#endif
