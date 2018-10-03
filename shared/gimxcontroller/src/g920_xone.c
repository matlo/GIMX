/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <g920_xone.h>
#include <report.h>
#include <string.h>
#include <stdlib.h>
#include "../include/controller.h"
#include <gimxcommon/include/gerror.h>

#define G920_VENDOR 0x045e
#define G920_PRODUCT 0x0261

#define G920_INTERRUPT_PACKET_SIZE 64

#define G920_USB_HID_IN_REPORT_ID 0x20 // same as XONE
#define G920_USB_HID_IN_GUIDE_REPORT_ID 0x07 // same as XONE

// buttons1

#define G920_VIEW_MASK 0x08
#define G920_MENU_MASK 0x04

// buttons2

#define G920_UP_MASK     0x01
#define G920_RIGHT_MASK  0x08
#define G920_DOWN_MASK   0x02
#define G920_LEFT_MASK   0x04

// buttons1

#define G920_Y_MASK 0x80
#define G920_B_MASK 0x20
#define G920_A_MASK 0x10
#define G920_X_MASK 0x40

// buttons2

#define G920_LB_MASK   0x10
#define G920_RB_MASK   0x20
#define G920_LT_MASK   0x80
#define G920_RT_MASK   0x40

#define G920_GEAR_SHIFTER_1  0x01
#define G920_GEAR_SHIFTER_2  0x02
#define G920_GEAR_SHIFTER_3  0x03
#define G920_GEAR_SHIFTER_4  0x04
#define G920_GEAR_SHIFTER_5  0x05
#define G920_GEAR_SHIFTER_6  0x06
#define G920_GEAR_SHIFTER_R  0xff

#define G920_GUIDE_MASK 0x01

GLOG_GET(GLOG_NAME)

static s_axis axes[AXIS_MAX] = {

        [g920Xonea_wheel]       = { .name = "wheel",    .max_unsigned_value = MAX_AXIS_VALUE_16BITS },

        [g920Xonea_gasPedal]    = { .name = "gas",      .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_brakePedal]  = { .name = "brake",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_clutchPedal] = { .name = "clutch",   .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

        [g920Xonea_view]        = { .name = "view",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_menu]        = { .name = "menu",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_guide]       = { .name = "guide",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

        [g920Xonea_up]          = { .name = "up",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_right]       = { .name = "right",    .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_down]        = { .name = "down",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_left]        = { .name = "left",     .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_Y]           = { .name = "Y",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_B]           = { .name = "B",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_A]           = { .name = "A",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_X]           = { .name = "X",        .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_LB]          = { .name = "LB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_RB]          = { .name = "RB",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_LT]          = { .name = "LT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_RT]          = { .name = "RT",       .max_unsigned_value = MAX_AXIS_VALUE_8BITS },

        [g920Xonea_gearShifter1] = {.name = "gear shifter 1", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifter2] = {.name = "gear shifter 2", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifter3] = {.name = "gear shifter 3", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifter4] = {.name = "gear shifter 4", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifter5] = {.name = "gear shifter 5", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifter6] = {.name = "gear shifter 6", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
        [g920Xonea_gearShifterR] = {.name = "gear shifter R", .max_unsigned_value = MAX_AXIS_VALUE_8BITS },
};

static s_axis_name_dir axis_name_dirs[] = {

        {.name = "wheel",        {.axis = g920Xonea_wheel,      .props = AXIS_PROP_CENTERED}},

        {.name = "gas",          {.axis = g920Xonea_gasPedal,   .props = AXIS_PROP_POSITIVE}},
        {.name = "brake",        {.axis = g920Xonea_brakePedal, .props = AXIS_PROP_POSITIVE}},
        {.name = "clutch",       {.axis = g920Xonea_clutchPedal,.props = AXIS_PROP_POSITIVE}},

        {.name = "RT",           {.axis = g920Xonea_RT,         .props = AXIS_PROP_TOGGLE}},
        {.name = "LT",           {.axis = g920Xonea_LT,         .props = AXIS_PROP_TOGGLE}},

        {.name = "view",         {.axis = g920Xonea_view,       .props = AXIS_PROP_TOGGLE}},
        {.name = "menu",         {.axis = g920Xonea_menu,       .props = AXIS_PROP_TOGGLE}},
        {.name = "guide",        {.axis = g920Xonea_guide,      .props = AXIS_PROP_TOGGLE}},
        {.name = "up",           {.axis = g920Xonea_up,         .props = AXIS_PROP_TOGGLE}},
        {.name = "down",         {.axis = g920Xonea_down,       .props = AXIS_PROP_TOGGLE}},
        {.name = "right",        {.axis = g920Xonea_right,      .props = AXIS_PROP_TOGGLE}},
        {.name = "left",         {.axis = g920Xonea_left,       .props = AXIS_PROP_TOGGLE}},
        {.name = "RB",           {.axis = g920Xonea_RB,         .props = AXIS_PROP_TOGGLE}},
        {.name = "LB",           {.axis = g920Xonea_LB,         .props = AXIS_PROP_TOGGLE}},
        {.name = "A",            {.axis = g920Xonea_A,          .props = AXIS_PROP_TOGGLE}},
        {.name = "B",            {.axis = g920Xonea_B,          .props = AXIS_PROP_TOGGLE}},
        {.name = "X",            {.axis = g920Xonea_X,          .props = AXIS_PROP_TOGGLE}},
        {.name = "Y",            {.axis = g920Xonea_Y,          .props = AXIS_PROP_TOGGLE}},

        {.name = "gear shifter 1",    {.axis = g920Xonea_gearShifter1,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter 2",    {.axis = g920Xonea_gearShifter2,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter 3",    {.axis = g920Xonea_gearShifter3,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter 4",    {.axis = g920Xonea_gearShifter4,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter 5",    {.axis = g920Xonea_gearShifter5,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter 6",    {.axis = g920Xonea_gearShifter6,   .props = AXIS_PROP_TOGGLE}},
        {.name = "gear shifter R",    {.axis = g920Xonea_gearShifterR,   .props = AXIS_PROP_TOGGLE}},
};

static s_report_g920Xone default_report = {
        .input = {
                .type = G920_USB_HID_IN_REPORT_ID,
                .unknown1 = 0x00,
                .counter = 0x00,
                .size = 0x11,
                .buttons1 = 0x00,
                .buttons2 = 0x00,
                .wheel = CENTER_AXIS_VALUE_16BITS,
                .gas = 0x00,
                .unknown2 = 0x00,
                .brake = 0x00,
                .unknown3 = 0x00,
                .clutch = 0x00,
                .unknown4 = 0x00,
                .unknown5 = 0x00,
                .unknown6 = 0xe6,
                .shifter = 0x00,
                .range = 0x0384, // 900
                .unknown7 = 0xff,
                .unknown8 = 0xe8,
        }
};

typedef enum {
    INPUT_REPORT_INDEX = 0,
    GUIDE_REPORT_INDEX = 1,
    MAX_REPORT_INDEX = 2
} report_index;

static void init_report(s_report * report)
{
  memcpy(report, &default_report, sizeof(default_report));
}

static unsigned int build_report(const int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS]) {

    unsigned int index;

    uint8_t guide_button = axis[g920Xonea_guide] ? G920_GUIDE_MASK : 0x00;

    if(guide_button ^ report[GUIDE_REPORT_INDEX].value.g920Xone.guide.button) {
        index = GUIDE_REPORT_INDEX;
        report[index].length = sizeof(report->value.g920Xone.guide);
        s_report_g920Xone * g920Xone = &report[index].value.g920Xone;

        g920Xone->guide.type = XONE_USB_HID_IN_GUIDE_REPORT_ID;
        g920Xone->guide.unknown1 = 0x20;
        g920Xone->guide.counter++;
        g920Xone->guide.size = 0x02;
        g920Xone->guide.button = guide_button;
        g920Xone->guide.unknown2 = 0x5b;
    }
    else {
        index = INPUT_REPORT_INDEX;
        report[index].length = sizeof(report->value.g920Xone.input);
        s_report_g920Xone * g920Xone = &report[index].value.g920Xone;

        g920Xone->input.type = G920_USB_HID_IN_REPORT_ID;
        g920Xone->input.unknown1 = 0x00;
        ++g920Xone->input.counter;
        if (g920Xone->input.counter == 0x00) {
            g920Xone->input.counter = 0x01;
        }

        g920Xone->input.size = 0x11;

        g920Xone->input.wheel = clamp(0, axis[g920Xonea_wheel] + CENTER_AXIS_VALUE_16BITS, MAX_AXIS_VALUE_16BITS);

        g920Xone->input.gas = clamp(0, axis[g920Xonea_gasPedal], MAX_AXIS_VALUE_8BITS);
        g920Xone->input.brake = clamp(0, axis[g920Xonea_brakePedal], MAX_AXIS_VALUE_8BITS);
        g920Xone->input.clutch = clamp(0, axis[g920Xonea_clutchPedal], MAX_AXIS_VALUE_8BITS);

        g920Xone->input.buttons1 = 0x00;
        g920Xone->input.buttons2 = 0x00;
        g920Xone->input.shifter = 0x00;

        if (axis[g920Xonea_right]) {
            g920Xone->input.buttons2 |= G920_RIGHT_MASK;
        }
        if (axis[g920Xonea_left]) {
            g920Xone->input.buttons2 |= G920_LEFT_MASK;
        }
        if (axis[g920Xonea_down]) {
            g920Xone->input.buttons2 |= G920_DOWN_MASK;
        }
        if (axis[g920Xonea_up]) {
            g920Xone->input.buttons2 |= G920_UP_MASK;
        }

        if (axis[g920Xonea_X]) {
            g920Xone->input.buttons1 |= G920_X_MASK;
        }
        if (axis[g920Xonea_A]) {
            g920Xone->input.buttons1 |= G920_A_MASK;
        }
        if (axis[g920Xonea_B]) {
            g920Xone->input.buttons1 |= G920_B_MASK;
        }
        if (axis[g920Xonea_Y]) {
            g920Xone->input.buttons1 |= G920_Y_MASK;
        }

        if (axis[g920Xonea_LB]) {
            g920Xone->input.buttons2 |= G920_LB_MASK;
        }
        if (axis[g920Xonea_RB]) {
            g920Xone->input.buttons2 |= G920_RB_MASK;
        }
        if (axis[g920Xonea_LT]) {
            g920Xone->input.buttons2 |= G920_LT_MASK;
        }
        if (axis[g920Xonea_RT]) {
            g920Xone->input.buttons2 |= G920_RT_MASK;
        }

        if (axis[g920Xonea_view]) {
            g920Xone->input.buttons1 |= G920_VIEW_MASK;
        }
        if (axis[g920Xonea_menu]) {
            g920Xone->input.buttons1 |= G920_MENU_MASK;
        }

        if (axis[g920Xonea_gearShifter1]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_1;
        }
        if (axis[g920Xonea_gearShifter2]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_2;
        }
        if (axis[g920Xonea_gearShifter3]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_3;
        }
        if (axis[g920Xonea_gearShifter4]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_4;
        }
        if (axis[g920Xonea_gearShifter5]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_5;
        }
        if (axis[g920Xonea_gearShifter6]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_6;
        }
        if (axis[g920Xonea_gearShifterR]) {
            g920Xone->input.shifter |= G920_GEAR_SHIFTER_R;
        }
        if (g920Xone->input.shifter == 0x00) {
            g920Xone->input.shifter = 0xff;
        }
    }

    return index;
}

static const struct {
    size_t length;
    uint8_t data[64];
} step1[] = {
        {
                .length = 64,
                .data =
                {
                        0x04, 0xf0, 0x01, 0x3a, 0x85, 0x03, 0x10, 0x00,
                        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x85, 0x01, 0xbc, 0x00,
                        0x16, 0x00, 0x1b, 0x00, 0x1c, 0x00, 0x24, 0x00,
                        0x2b, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
                        0x00, 0x00, 0x07, 0x01, 0x02, 0x03, 0x04, 0x06,
                        0x07, 0x1f, 0x06, 0x01, 0x04, 0x05, 0x06, 0x0a
                }
        },
        {
                .length = 64,
                .data =
                {
                        0x04, 0xb0, 0x01, 0xba, 0x00, 0x3a, 0x1f, 0x03,
                        0x1a, 0x00, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x73,
                        0x6f, 0x66, 0x74, 0x2e, 0x58, 0x62, 0x6f, 0x78,
                        0x2e, 0x49, 0x6e, 0x70, 0x75, 0x74, 0x2e, 0x57,
                        0x68, 0x65, 0x65, 0x6c, 0x18, 0x00, 0x57, 0x69,
                        0x6e, 0x64, 0x6f, 0x77, 0x73, 0x2e, 0x58, 0x62,
                        0x6f, 0x78, 0x2e, 0x49, 0x6e, 0x70, 0x75, 0x74,
                        0x2e, 0x57, 0x68, 0x65, 0x65, 0x6c, 0x27, 0x00
                }
        },
        {
                .length = 64,
                .data =
                {
                        0x04, 0xb0, 0x01, 0xba, 0x00, 0x74, 0x57, 0x69,
                        0x6e, 0x64, 0x6f, 0x77, 0x73, 0x2e, 0x58, 0x62,
                        0x6f, 0x78, 0x2e, 0x49, 0x6e, 0x70, 0x75, 0x74,
                        0x2e, 0x4e, 0x61, 0x76, 0x69, 0x67, 0x61, 0x74,
                        0x69, 0x6f, 0x6e, 0x43, 0x6f, 0x6e, 0x74, 0x72,
                        0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x03, 0xcf, 0x79,
                        0x69, 0x64, 0x71, 0x6b, 0x96, 0x4e, 0x8d, 0xf9,
                        0x59, 0xe3, 0x98, 0xd7, 0x42, 0x0c, 0xe7, 0x1f
                }
        },
        {
                .length = 64,
                .data =
                {
                        0x04, 0xb0, 0x01, 0x3a, 0xae, 0x01, 0xf3, 0xb8,
                        0x86, 0x73, 0xe9, 0x40, 0xa9, 0xf8, 0x2f, 0x21,
                        0x26, 0x3a, 0xcf, 0xb7, 0x56, 0xff, 0x76, 0x97,
                        0xfd, 0x9b, 0x81, 0x45, 0xad, 0x45, 0xb6, 0x45,
                        0xbb, 0xa5, 0x26, 0xd6, 0x08, 0x17, 0x00, 0x20,
                        0x11, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x21, 0x0b
                }
        },
        {
                .length = 64,
                .data =
                {
                        0x04, 0xb0, 0x01, 0x3a, 0xe8, 0x01, 0x00, 0x01,
                        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x17, 0x00, 0x25, 0x31, 0x00, 0x01, 0x00,
                        0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x17, 0x00, 0x0a, 0x03, 0x00, 0x01, 0x00, 0x08,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                }
        },
        {
                .length = 64,
                .data =
                {
                        0x04, 0xb0, 0x01, 0x3a, 0xa2, 0x02, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x0b,
                        0x3c, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x0c, 0x09,
                        0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x17, 0x00, 0x0d, 0x35, 0x00
                }
        },
        {
                .length = 47,
                .data =
                {
                        0x04, 0xb0, 0x01, 0x29, 0xdc, 0x02, 0x01, 0x00,
                        0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x17, 0x00, 0x0e, 0x38, 0x00, 0x01, 0x00, 0x08,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                }
        },
        {
                .length = 6,
                .data =
                {
                        0x04, 0xb0, 0x01, 0x00, 0x85, 0x03
                }
        },
};

static const unsigned char desc0220[] = {
        0x02, 0x20, 0x01, 0x1c, 0x7e, 0xed, 0x82, 0xec,
        0x8b, 0x97, 0x00, 0x00, 0x6d, 0x04, 0x61, 0xc2,
        0x01, 0x00, 0x00, 0x00, 0x43, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
};

static const unsigned char desc2100[] = {
        0x21, 0x00, 0x01, 0x0b, 0x10, 0x08, 0x08, 0x08,
        0x08, 0x0e, 0x01, 0x84, 0x03, 0x01, 0x48
};

struct controller_state {
    unsigned int step;
    unsigned int substep;
    int sent[sizeof(step1) / sizeof(*step1)];
    int sendDesc2100;
    struct {
        int send;
        size_t len;
        uint8_t data[G920_INTERRUPT_PACKET_SIZE];
    } in;
    struct {
        int send;
        size_t len;
        uint8_t data[G920_INTERRUPT_PACKET_SIZE];
    } out;
    s_report_packet reports[MAX_REPORT_INDEX];
    int status;
    uint8_t cpt0220;
    const int * axes;
    int toggle;
};

static struct controller_state * init_state(const int * axes) {

    struct controller_state * state = calloc(1, sizeof(struct controller_state));

    if (state != NULL) {
        memcpy(&state->reports[INPUT_REPORT_INDEX], &default_report, sizeof(default_report));
    }

    state->axes = axes;

    init_report(&state->reports[INPUT_REPORT_INDEX].value);

    return state;
}

static void clean_state(struct controller_state * state) {

    free(state);
}

static inline void set_step(struct controller_state * state, int step) {

    state->step = step;
    state->substep = 0;
    if (GLOG_LEVEL(GLOG_NAME,DEBUG)) {
        printf("%s:%d %s: step=%d\n", __FILE__, __LINE__, __func__, state->step);
    }
}

static void process_out(struct controller_state * state, const uint8_t * data, size_t len) {

    if ((data[0] == 0x0b || data[0] == 0x0d) && data[1] == 0x00) { // ffb data
        return;
    }

    if (data[0] == 0x0a && data[1] == 0x00) {
        if (data[4] == 0x01) {
            state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.range = data[6] << 8 | data[5];
            return;
        } else if (data[4] == 0x00) {
            state->sendDesc2100 = 1;
            return;
        }
    }

    if (data[0] == 0x04 && data[1] == 0x20) { // start of step 1
        set_step(state, 1);
        return;
    }

    if (state->step == 1) {
        if (data[0] == 0x01 && data[1] == 0x20) { // ack
            ++state->substep;
        }
        return;
    }

    if (data[0] == 0x05 && data[1] == 0x20 && data[3] == 0x01) {
       state->status = 1;
       if (data[4] == 0x05) {
           state->toggle = 1;
       }
    }

    if (state->out.send) {
        PRINT_ERROR_OTHER("overwrite out report")
    }
    state->out.len = len;
    memcpy(state->out.data, data, len);
    state->out.send = 1;
}

static const uint8_t * get_out(struct controller_state * state, size_t * len) {

    const uint8_t * ret = NULL;

    if (state->out.send) {
        *len = state->out.len;
        ret = state->out.data;
        state->out.send = 0;
    }

    return ret;
}

static void process_in(struct controller_state * state, const uint8_t * data, size_t len) {

    if (data[0] == 0x02 && data[1] == 0x20) {
        return;
    }

    state->in.len = len;
    memcpy(state->in.data, data, len);

    if (state->in.send) {
        PRINT_ERROR_OTHER("overwrite in report")
    }
    state->in.send = 1;
}

static const uint8_t * get_in(struct controller_state * state, size_t * len) {

    const uint8_t * ret = NULL;

    if (state->step == 0) {
        state->in.len = sizeof(desc0220);
        memcpy(state->in.data, desc0220, sizeof(desc0220));
        state->in.data[2] = state->cpt0220;
        state->in.send = 1;
        ++state->cpt0220;
    } else if (state->step == 1) {
        if (state->substep < sizeof(step1) / sizeof(*step1) && state->sent[state->substep] == 0) {
            *len = step1[state->substep].length;
            ret = step1[state->substep].data;
            state->sent[state->substep] = 1;
            if (state->substep == sizeof(step1) / sizeof(*step1) - 1) {
                set_step(state, 2); // step 2
            }
        }
    }

    if (ret == NULL && state->sendDesc2100) {
        *len = sizeof(desc2100);
        ret = desc2100;
        state->sendDesc2100 = 0;
    }

    if (ret == NULL && state->in.send) {
        *len = state->in.len;
        ret = state->in.data;
        state->in.send = 0;
    }

    if (ret == NULL && state->status) {
        unsigned int index = build_report(state->axes, state->reports);
        s_report_packet * packet = state->reports + index;
        *len = packet->length;
        ret = (uint8_t *)&packet->value;
        if (index == INPUT_REPORT_INDEX && state->toggle) {
            if (state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.unknown8 == 0xe8) {
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.unknown6 = 0x06;
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.shifter = 0x00;
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.unknown8 = 0x00;
            } else {
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.unknown6 = 0xe6;
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.shifter = 0xff;
                state->reports[INPUT_REPORT_INDEX].value.g920Xone.input.unknown8 = 0xe8;
                state->toggle = 0;
            }
        }
    }

    return ret;
}

struct controller_interface interface = {
        .fp_process_out = process_out,
        .fp_process_in = process_in,
        .fp_get_in = get_in,
        .fp_get_out = get_out,
};

typedef unsigned int (*fp_build_report)(int axis[AXIS_MAX], s_report_packet report[MAX_REPORTS]);

static s_controller controller =
{
  .name = "G920 XONE",
  .vid = 0x046d,
  .pid = 0xc261,
  .refresh_period = { .min_value = 1000, .default_value = 4000 },
  .auth_required = 1,
  .axes = axes,
  .axis_name_dirs = { .nb = sizeof(axis_name_dirs)/sizeof(*axis_name_dirs), .values = axis_name_dirs },
  .fp_build_report = (fp_build_report) build_report,
  .fp_init_report = init_report,

  .fp_init = init_state,
  .fp_clean = clean_state,
  .interface = &interface
};

void g920Xone_init(void) __attribute__((constructor));
void g920Xone_init(void)
{
  controller_register(C_TYPE_G920_XONE, &controller);
}
