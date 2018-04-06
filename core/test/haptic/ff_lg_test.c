/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define SDL_MAIN_HANDLED

#include <limits.h>
#include <string.h>
#include <haptic/common/ff_lg.h>
#include <haptic/haptic_tweaks.h>

s_gimx_params gimx_params = { 0 }; // { .debug = { .ff_lg = 1 } };

#define CONSTANT(LEVEL) \
        .force_type = FF_LG_FTYPE_CONSTANT, \
        .parameters = { LEVEL },

#define VARIABLE(LEVEL) \
        .force_type = FF_LG_FTYPE_VARIABLE, \
        .parameters = { LEVEL },

#define LR_DAMPER(K1, S1, K2, S2) \
        .force_type = FF_LG_FTYPE_DAMPER, \
        .parameters = { K1, S1, K2, S2 }

#define HR_DAMPER(K1, S1, K2, S2, CLIP) \
        .force_type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER, \
        .parameters = { K1, S1, K2, S2, CLIP }

#define LR_SPRING(D1, D2, K2, K1, S2, S1, CLIP) \
        .force_type = FF_LG_FTYPE_SPRING, \
        .parameters = { D1, D2, K2 | K1, S2 | S1, CLIP }

#define HR_SPRING(D1, D2, K2, K1, D2L, S2, D1L, S1, CLIP) \
        .force_type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING, \
        .parameters = { D1, D2, K2 | K1, D2L | S2 | D1L | S1, CLIP }

#define CONSTANT_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, C_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = C_GAIN, \
                            .spring = 0, \
                            .damper = 0 \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 0, \
            .out.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define VARIABLE_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, C_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = C_GAIN, \
                            .spring = 0, \
                            .damper = 0 \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 0, \
            .out.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define HR_SPRING_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, S_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = 0, \
                            .spring = S_GAIN, \
                            .damper = 0 \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 1, \
            .out.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define HR_DAMPER_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, D_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_3 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = 0, \
                            .spring = 0, \
                            .damper = D_GAIN \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 2, \
            .out.cmd = FF_LG_FSLOT_3 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define SPRING_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, S_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = 0, \
                            .spring = S_GAIN, \
                            .damper = 0 \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 1, \
            .out.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }


#define DAMPER_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, D_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_3 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .tweaks = { \
                    .invert = INVERT, \
                    .gain = { \
                            .constant = 0, \
                            .spring = 0, \
                            .damper = D_GAIN \
                    }, \
            }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .dst_slot = 2, \
            .out.cmd = FF_LG_FSLOT_3 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

static const struct {
    char * name;
    uint16_t src_pid;
    union {
        unsigned char data[FF_LG_OUTPUT_REPORT_SIZE];
        struct PACKED {
            unsigned char cmd;
            s_ff_lg_command force;
        };
    } in;
    s_haptic_core_tweaks tweaks;
    uint16_t dst_pid;
    uint16_t dst_range;
    uint8_t dst_slot;
    union {
        unsigned char data[FF_LG_OUTPUT_REPORT_SIZE];
        struct PACKED {
            unsigned char cmd;
            s_ff_lg_command force;
        };
    } out;
} test_cases[] = {

        CONSTANT_TEST("G29 constant force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(0xff), CONSTANT(0xff)),
        CONSTANT_TEST("G29 constant force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(0), CONSTANT(0)),
        CONSTANT_TEST("G29 constant force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(127), CONSTANT(127)),

        CONSTANT_TEST("G29 constant force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(0xff), CONSTANT(0xbf)),
        CONSTANT_TEST("G29 constant force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(0), CONSTANT(0x3f)),
        CONSTANT_TEST("G29 constant force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(127), CONSTANT(127)),

        CONSTANT_TEST("G29 constant force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(0xff), CONSTANT(0)),
        CONSTANT_TEST("G29 constant force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(0), CONSTANT(0xff)),
        CONSTANT_TEST("G29 constant force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(127), CONSTANT(127)),

        DAMPER_TEST("G29 damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),

        DAMPER_TEST("G29 damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x07, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x00, 0x00, 0x07, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x07, 0x01, 0x07, 0x01, 0xff)),

        DAMPER_TEST("G29 damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        DAMPER_TEST("G29 damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),

        DAMPER_TEST("FFGP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
                LR_DAMPER(0x05, 0x00, 0x05, 0x00), HR_DAMPER(0x0b, 0x00, 0x0b, 0x00, 0xff)),
        DAMPER_TEST("FFGP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
                LR_DAMPER(0x06, 0x00, 0x06, 0x00), HR_DAMPER(0x07, 0x00, 0x07, 0x00, 0xff)),

        DAMPER_TEST("DFP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x05, 0x00, 0x05, 0x00), HR_DAMPER(0x07, 0x00, 0x07, 0x00, 0xff)),
        DAMPER_TEST("DFP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_DAMPER(0x06, 0x00, 0x06, 0x00), HR_DAMPER(0x0b, 0x00, 0x0b, 0x00, 0xff)),

        SPRING_TEST("G29 spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 255)),
        SPRING_TEST("G29 spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 255)),
        SPRING_TEST("G29 spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
        SPRING_TEST("G29 spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xf0, 0x00, 0x0f, 0x00, 0xff)),

        SPRING_TEST("G29 spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0xe0, 0x00, 0x0e, 0x00, 0x7f)),
        SPRING_TEST("G29 spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0x7f)),
        SPRING_TEST("G29 spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
        SPRING_TEST("G29 spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0x70, 0x07, 0xe0, 0x10, 0x0e, 0x01, 0x7f)),

        SPRING_TEST("G29 spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 255)),
        SPRING_TEST("G29 spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 255)),
        SPRING_TEST("G29 spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0xff, 0xff, 0x00, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0)),
        SPRING_TEST("G29 spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0x00, 0x00, 0xf0, 0x0f, 0x00, 0x10, 0x00, 0x01, 0xff)),

        SPRING_TEST("FFGP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
                LR_SPRING(0x7f, 0x7f, 0x50, 0x05, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xb0, 0x0b, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
        SPRING_TEST("FFGP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
                LR_SPRING(0x7f, 0x7f, 0x60, 0x06, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x07, 0xe0, 0x00, 0x0e, 0x00, 0xff)),

        SPRING_TEST("DFP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x50, 0x05, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x07, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
        SPRING_TEST("DFP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                LR_SPRING(0x7f, 0x7f, 0x60, 0x06, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xb0, 0x0b, 0xe0, 0x00, 0x0e, 0x00, 0xff)),

        VARIABLE_TEST("G29 variable force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(0xff), CONSTANT(0xff)),
        VARIABLE_TEST("G29 variable force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(0), CONSTANT(0)),
        VARIABLE_TEST("G29 variable force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(127), CONSTANT(127)),

        VARIABLE_TEST("G29 variable force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(0xff), CONSTANT(0xbf)),
        VARIABLE_TEST("G29 variable force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(0), CONSTANT(0x3f)),
        VARIABLE_TEST("G29 variable force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(127), CONSTANT(127)),

        VARIABLE_TEST("G29 variable force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(0xff), CONSTANT(0)),
        VARIABLE_TEST("G29 variable force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(0), CONSTANT(0xff)),
        VARIABLE_TEST("G29 variable force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(127), CONSTANT(127)),

        HR_SPRING_TEST("G29 high resolution spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xc0, 0x00, 0x0c, 0x00, 0xff)),
        HR_SPRING_TEST("G29 high resolution spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xc0, 0x00, 0x0c, 0x00, 0xff)),
        HR_SPRING_TEST("G29 high resolution spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
        HR_SPRING_TEST("G29 high resolution spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff)),

        HR_SPRING_TEST("G29 high resolution spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0xc0, 0x00, 0x0c, 0x00, 0x7f)),
        HR_SPRING_TEST("G29 high resolution spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0xc0, 0x00, 0x0c, 0x00, 0x7f)),
        HR_SPRING_TEST("G29 high resolution spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
        HR_SPRING_TEST("G29 high resolution spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0x70, 0x07, 0xe0, 0x10, 0x0e, 0x01, 0x7f)),

        HR_SPRING_TEST("G29 high resolution spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x80, 0x80, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff)),
        HR_SPRING_TEST("G29 high resolution spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x80, 0x80, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff)),
        HR_SPRING_TEST("G29 high resolution spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0xff, 0xff, 0x00, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0)),
        HR_SPRING_TEST("G29 high resolution spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0x00, 0x00, 0xf0, 0x0f, 0x00, 0x10, 0x00, 0x01, 0xff)),

        HR_DAMPER_TEST("G29 high resolution damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),

        HR_DAMPER_TEST("G29 high resolution damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x07, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x07, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x07, 0x01, 0x07, 0x01, 0xff)),

        HR_DAMPER_TEST("G29 high resolution damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
        HR_DAMPER_TEST("G29 high resolution damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),
};

#define CHECK_PARAM(PARAM) \
        if (PARAM(fref) != PARAM(fres)) \
        { \
            fprintf(stderr, #PARAM ": %02x (ref) vs %02x (res)\n", PARAM(fref), PARAM(fres)); \
            ret = 1; \
        }


int compare_forces(const unsigned char * ref, const unsigned char * res) {

    if (ref[0] != res[0]) {
        fprintf(stderr, "cmd: %02x (ref) vs %02x (res)\n", ref[0], res[0]);
        return 1;
    }

    const s_ff_lg_command * fref = (const s_ff_lg_command *)(ref + 1);
    const s_ff_lg_command * fres = (const s_ff_lg_command *)(res + 1);

    if (fref->force_type != fres->force_type) {
        fprintf(stderr, "type: %u vs %u\n", fref->force_type, fres->force_type);
        return 1;
    }

    int ret = 0;

    switch (fref->force_type) {
    case FF_LG_FTYPE_CONSTANT:
        if (FF_LG_CONSTANT_LEVEL(fref, 0) != FF_LG_CONSTANT_LEVEL(fres, 0))
        {
            fprintf(stderr, "level: %u (ref) vs %u (res)\n", FF_LG_CONSTANT_LEVEL(fref, 0), FF_LG_CONSTANT_LEVEL(fres, 0));
            ret = 1;
        }
        break;
    case FF_LG_FTYPE_VARIABLE:
        CHECK_PARAM(FF_LG_VARIABLE_L1)
        CHECK_PARAM(FF_LG_VARIABLE_L2)
        break;
    case FF_LG_FTYPE_SPRING:
        CHECK_PARAM(FF_LG_SPRING_K1)
        CHECK_PARAM(FF_LG_SPRING_S1)
        CHECK_PARAM(FF_LG_SPRING_D1)
        CHECK_PARAM(FF_LG_SPRING_K2)
        CHECK_PARAM(FF_LG_SPRING_S2)
        CHECK_PARAM(FF_LG_SPRING_D2)
        CHECK_PARAM(FF_LG_SPRING_CLIP)
        break;
    case FF_LG_FTYPE_DAMPER:
        CHECK_PARAM(FF_LG_DAMPER_K1)
        CHECK_PARAM(FF_LG_DAMPER_S1)
        CHECK_PARAM(FF_LG_DAMPER_K2)
        CHECK_PARAM(FF_LG_DAMPER_S2)
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_SPRING:
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_K1)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_S1)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_D1)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_D1L)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_K2)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_S2)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_D2)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_D2L)
        CHECK_PARAM(FF_LG_HIGHRES_SPRING_CLIP)
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER:
        CHECK_PARAM(FF_LG_HIGHRES_DAMPER_K1)
        CHECK_PARAM(FF_LG_HIGHRES_DAMPER_S1)
        CHECK_PARAM(FF_LG_HIGHRES_DAMPER_K2)
        CHECK_PARAM(FF_LG_HIGHRES_DAMPER_S2)
        CHECK_PARAM(FF_LG_HIGHRES_DAMPER_CLIP)
        break;
    default:
        if (memcmp(fref->parameters, fres->parameters, sizeof(fref->parameters))) {
            fprintf(stderr, "unmodified report\n");
            return 1;
        }
        break;
    }

    return ret;
}

int main(int argc __attribute__((unused)), char * argv[] __attribute__((unused))) {

    unsigned int i;
    for (i = 0; i < sizeof(test_cases) / sizeof(*test_cases); ++i) {

        printf("test case: %s: ", test_cases[i].name);

        s_haptic_core_data data;

        ff_lg_convert_force(ff_lg_get_caps(test_cases[i].src_pid), 0, &test_cases[i].in.force, 1, &data);

        haptic_tweak_apply(&test_cases[i].tweaks, &data);

        s_ff_lg_report report;

        ff_lg_convert_slot(&data, test_cases[i].dst_slot, &report, ff_lg_get_caps(test_cases[i].dst_pid));

        if (compare_forces(test_cases[i].out.data, report.data + 1)) {
            printf("failed\n");
            continue;
        }

        printf("success\n");
    }

    return 0;
}
