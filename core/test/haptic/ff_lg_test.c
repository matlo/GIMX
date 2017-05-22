/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define SDL_MAIN_HANDLED

#include <gimx.h>
#include <limits.h>
#include <string.h>
#include <haptic/ff_lg.h>

s_gimx_params gimx_params = { 0 }; // { .debug = { .ff_lg = 1 } };

#define CONSTANT(LEVEL) \
        .type = FF_LG_FTYPE_CONSTANT, \
        .parameters = { LEVEL },

#define VARIABLE(LEVEL) \
        .type = FF_LG_FTYPE_VARIABLE, \
        .parameters = { LEVEL },

#define LR_DAMPER(K1, S1, K2, S2) \
        .type = FF_LG_FTYPE_DAMPER, \
        .parameters = { K1, S1, K2, S2 }

#define HR_DAMPER(K1, S1, K2, S2, CLIP) \
        .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER, \
        .parameters = { K1, S1, K2, S2, CLIP }

#define LR_SPRING(D1, D2, K2, K1, S2, S1, CLIP) \
        .type = FF_LG_FTYPE_SPRING, \
        .parameters = { D1, D2, K2 | K1, S2 | S1, CLIP }

#define HR_SPRING(D1, D2, K2, K1, D2L, S2, D1L, S1, CLIP) \
        .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING, \
        .parameters = { D1, D2, K2 | K1, D2L | S2 | D1L | S1, CLIP }

#define CONSTANT_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, C_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { C_GAIN, 0, 0 }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define VARIABLE_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, C_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { C_GAIN, 0, 0 }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define HR_SPRING_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, S_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { 0, S_GAIN, 0 }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define HR_DAMPER_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, D_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { 0, 0, D_GAIN }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

#define SPRING_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, S_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { 0, S_GAIN, 0 }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }


#define DAMPER_TEST(NAME, SRC_PRODUCT_ID, DST_PRODUCT_ID, INVERT, D_GAIN, RANGE, F_IN, F_OUT) \
    { \
            .name = NAME, \
            .src_pid = SRC_PRODUCT_ID, \
            .in.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .in.force = { F_IN }, \
            .invert = INVERT, \
            .gain = { 0, 0, D_GAIN }, \
            .dst_pid = DST_PRODUCT_ID, \
            .dst_range = RANGE, \
            .out.cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .out.force = { F_OUT }, \
    }

static const struct {
    char * name;
    uint16_t src_pid;
    union {
        unsigned char data[FF_LG_OUTPUT_REPORT_SIZE];
        struct PACKED {
            unsigned char cmd;
            s_ff_lg_force force;
        };
    } in;
    int invert;
    struct {
        int constant;
        int spring;
        int damper;
    } gain;
    uint16_t dst_pid;
    uint16_t dst_range;
    union {
        unsigned char data[FF_LG_OUTPUT_REPORT_SIZE];
        struct PACKED {
            unsigned char cmd;
            s_ff_lg_force force;
        };
    } out;
} test_cases[] = {

//        CONSTANT_TEST("G29 constant force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(0xff), CONSTANT(0xff)),
//        CONSTANT_TEST("G29 constant force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(0), CONSTANT(0)),
//        CONSTANT_TEST("G29 constant force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, CONSTANT(127), CONSTANT(127)),
//
//        CONSTANT_TEST("G29 constant force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(0xff), CONSTANT(0xbf)),
//        CONSTANT_TEST("G29 constant force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(0), CONSTANT(0x40)),
//        CONSTANT_TEST("G29 constant force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, CONSTANT(127), CONSTANT(127)),
//
//        CONSTANT_TEST("G29 constant force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(0xff), CONSTANT(0)),
//        CONSTANT_TEST("G29 constant force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(0), CONSTANT(0xff)),
//        CONSTANT_TEST("G29 constant force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, CONSTANT(127), CONSTANT(127)),
//
//        DAMPER_TEST("G29 damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),
//
//        DAMPER_TEST("G29 damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x07, 0x00, 0x00, 0x00, 0x7f)),
//        DAMPER_TEST("G29 damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x00, 0x00, 0x07, 0x00, 0x7f)),
//        DAMPER_TEST("G29 damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0x7f)),
//        DAMPER_TEST("G29 damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x07, 0x01, 0x07, 0x01, 0x7f)),
//
//        DAMPER_TEST("G29 damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_DAMPER(0x07, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_DAMPER(0x00, 0x00, 0x07, 0x00), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_DAMPER(0x00, 0x00, 0x00, 0x00), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
//        DAMPER_TEST("G29 damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_DAMPER(0x07, 0x01, 0x07, 0x01), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),
//
//        DAMPER_TEST("FFGP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
//                LR_DAMPER(0x05, 0x00, 0x05, 0x00), HR_DAMPER(0x0b, 0x00, 0x0b, 0x00, 0xff)),
//        DAMPER_TEST("FFGP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
//                LR_DAMPER(0x06, 0x00, 0x06, 0x00), HR_DAMPER(0x07, 0x00, 0x07, 0x00, 0xff)),
//
//        DAMPER_TEST("DFP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x05, 0x00, 0x05, 0x00), HR_DAMPER(0x07, 0x00, 0x07, 0x00, 0xff)),
//        DAMPER_TEST("DFP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_DAMPER(0x06, 0x00, 0x06, 0x00), HR_DAMPER(0x0b, 0x00, 0x0b, 0x00, 0xff)),
//
//        SPRING_TEST("G29 spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0x60, 0x00, 0x06, 0x00, 255)),
//        SPRING_TEST("G29 spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0x60, 0x00, 0x06, 0x00, 255)),
//        SPRING_TEST("G29 spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        SPRING_TEST("G29 spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xf0, 0x00, 0x0f, 0x00, 0xff)),
//
//        SPRING_TEST("G29 spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x60, 0x00, 0x06, 0x00, 0x7f)),
//        SPRING_TEST("G29 spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x60, 0x00, 0x06, 0x00, 0x7f)),
//        SPRING_TEST("G29 spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        SPRING_TEST("G29 spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0x70, 0x07, 0xe0, 0x10, 0x0e, 0x01, 0x7f)),
//
//        SPRING_TEST("G29 spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0x60, 0x00, 0x06, 0x00, 255)),
//        SPRING_TEST("G29 spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0x00, 0x00, 255), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0x60, 0x00, 0x06, 0x00, 255)),
//        SPRING_TEST("G29 spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        SPRING_TEST("G29 spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                LR_SPRING(0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff)),
//
//        SPRING_TEST("FFGP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
//                LR_SPRING(0x7f, 0x7f, 0x50, 0x05, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xb0, 0x0b, 0x60, 0x00, 0x06, 0x00, 0xff)),
//        SPRING_TEST("FFGP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 200,
//                LR_SPRING(0x7f, 0x7f, 0x60, 0x06, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x07, 0x60, 0x00, 0x06, 0x00, 0xff)),
//
//        SPRING_TEST("DFP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x50, 0x05, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x07, 0x60, 0x00, 0x06, 0x00, 0xff)),
//        SPRING_TEST("DFP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                LR_SPRING(0x7f, 0x7f, 0x60, 0x06, 0x00, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xb0, 0x0b, 0x60, 0x00, 0x06, 0x00, 0xff)),
//
//        VARIABLE_TEST("G29 variable force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(0xff), CONSTANT(0xff)),
//        VARIABLE_TEST("G29 variable force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(0), CONSTANT(0)),
//        VARIABLE_TEST("G29 variable force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0, VARIABLE(127), CONSTANT(127)),
//
//        VARIABLE_TEST("G29 variable force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(0xff), CONSTANT(0xbf)),
//        VARIABLE_TEST("G29 variable force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(0), CONSTANT(0x40)),
//        VARIABLE_TEST("G29 variable force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0, VARIABLE(127), CONSTANT(127)),
//
//        VARIABLE_TEST("G29 variable force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(0xff), CONSTANT(0)),
//        VARIABLE_TEST("G29 variable force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(0), CONSTANT(0xff)),
//        VARIABLE_TEST("G29 variable force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0, VARIABLE(127), CONSTANT(127)),
//
//        HR_SPRING_TEST("G29 high resolution spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
//        HR_SPRING_TEST("G29 high resolution spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
//        HR_SPRING_TEST("G29 high resolution spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        HR_SPRING_TEST("G29 high resolution spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff)),
//
//        HR_SPRING_TEST("G29 high resolution spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x00, 0x07, 0xe0, 0x00, 0x0e, 0x00, 0x7f)),
//        HR_SPRING_TEST("G29 high resolution spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x70, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0x7f)),
//        HR_SPRING_TEST("G29 high resolution spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        HR_SPRING_TEST("G29 high resolution spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0x70, 0x07, 0xe0, 0x10, 0x0e, 0x01, 0x7f)),
//
//        HR_SPRING_TEST("G29 high resolution spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
//        HR_SPRING_TEST("G29 high resolution spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_SPRING(0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff), HR_SPRING(0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff)),
//        HR_SPRING_TEST("G29 high resolution spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0), HR_SPRING(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0)),
//        HR_SPRING_TEST("G29 high resolution spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff), HR_SPRING(0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff)),
//
//        HR_DAMPER_TEST("G29 high resolution damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 100, 0,
//                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),
//
//        HR_DAMPER_TEST("G29 high resolution damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x07, 0x00, 0x00, 0x00, 0x7f)),
//        HR_DAMPER_TEST("G29 high resolution damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x07, 0x00, 0x7f)),
//        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0x7f)),
//        HR_DAMPER_TEST("G29 high resolution damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 0, 50, 0,
//                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x07, 0x01, 0x07, 0x01, 0x7f)),
//
//        HR_DAMPER_TEST("G29 high resolution damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff), HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_DAMPER(0x00, 0x00, 0x0f, 0x00, 0xff), HR_DAMPER(0x0f, 0x00, 0x00, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0), HR_DAMPER(0x00, 0x00, 0x00, 0x00, 0xff)),
//        HR_DAMPER_TEST("G29 high resolution damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, 1, 100, 0,
//                HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff), HR_DAMPER(0x0f, 0x01, 0x0f, 0x01, 0xff)),
};

#define CHECK_PARAM(PARAM) \
        if (PARAM(fref) != PARAM(fres)) \
        { \
            fprintf(stderr, #PARAM ": %u (ref) vs %u (res)\n", PARAM(fref), PARAM(fres)); \
            ret = 1; \
        }


int compare_forces(const unsigned char * ref, const unsigned char * res) {

    if (ref[0] != res[0]) {
        fprintf(stderr, "cmd: %u vs %u\n", ref[0], res[0]);
        return 1;
    }

    const s_ff_lg_force * fref = (const s_ff_lg_force *)(ref + 1);
    const s_ff_lg_force * fres = (const s_ff_lg_force *)(res + 1);

    if (fref->type != fres->type) {
        fprintf(stderr, "type: %u vs %u\n", fref->type, fres->type);
        return 1;
    }

    int ret = 0;

    switch (fref->type) {
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

        ff_lg_init(0, test_cases[i].src_pid, test_cases[i].dst_pid);

        ff_lg_process_report(0, test_cases[i].in.data);

        s_ff_lg_report report;
        ff_lg_get_report(0, &report);

        printf("test case: %s: ", test_cases[i].name);

        if (test_cases[i].dst_range) {
            if (report.data[1] != FF_LG_CMD_EXTENDED_COMMAND) {
                fprintf(stderr, "cmd: %u vs %u\n", FF_LG_CMD_EXTENDED_COMMAND, report.data[1]);
                printf("failed: ff_lg_get_report returned an incorrect event\n");
                continue;
            }
            if (report.data[2] != FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE) {
                fprintf(stderr, "ext: %u vs %u\n", FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE, report.data[2]);
                printf("failed: ff_lg_get_report returned an incorrect event\n");
                continue;
            }
            uint16_t range = (report.data[4] << 8) | report.data[3];
            if (range != test_cases[i].dst_range) {
                fprintf(stderr, "range: %u vs %u\n", test_cases[i].dst_range, range);
                printf("failed: ff_lg_get_report returned an incorrect event\n");
                continue;
            }

            ff_lg_ack(0);
            ff_lg_get_report(0, &report);
        }

        if (compare_forces(test_cases[i].out.data, report.data + 1)) {
            printf("failed: ff_lg_get_report returned an incorrect event\n");
            continue;
        }

        ff_lg_ack(0);

        s_ff_lg_report stop = { .data = { (test_cases[i].in.data[0] & FF_LG_FSLOT_MASK) | FF_LG_CMD_STOP } };

        ff_lg_process_report(0, stop.data);

        ff_lg_get_report(0, &report);

        if (compare_forces(stop.data, report.data + 1)) {
            printf("failed: ff_lg returned an incorrect stop event\n");
            continue;
        }

        ff_lg_ack(0);

        printf("success\n");
    }

    return 0;
}
