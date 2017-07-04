/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define SDL_MAIN_HANDLED

#include <gimx.h>
#include <limits.h>
#include <string.h>
#include <haptic/ff_conv.h>

s_gimx_params gimx_params = { 0 }; // { .debug = { .ff_lg = 1, .ff_conv = 1 } };

#define CONSTANT(NAME, PRODUCT_ID, INVERT, C_GAIN, LEVEL_IN, LEVEL_OUT) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_CONSTANT, \
                    .parameters = { LEVEL_IN }, \
            }, \
            .invert = INVERT, \
            .gain = { C_GAIN, 0, 0 }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jconstant = { \
                                    .type = GE_JOYCONSTANTFORCE, \
                                    .level = LEVEL_OUT \
                            } \
                    } \
            } \
    }

#define VARIABLE(NAME, PRODUCT_ID, INVERT, C_GAIN, LEVEL_IN, LEVEL_OUT) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_VARIABLE, \
                    .parameters = { LEVEL_IN }, \
            }, \
            .invert = INVERT, \
            .gain = { C_GAIN, 0, 0 }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jconstant = { \
                                    .type = GE_JOYCONSTANTFORCE, \
                                    .level = LEVEL_OUT \
                            } \
                    } \
            } \
    }

#define HR_SPRING(NAME, PRODUCT_ID, INVERT, S_GAIN, D1, D2, K2, K1, D2L, S2, D1L, S1, CLIP, SL, SR, CL, CR, C, D) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING, \
                    .parameters = { D1, D2, K2 | K1, D2L | S2 | D1L | S1, CLIP } \
            }, \
            .invert = INVERT, \
            .gain = { 0, S_GAIN, 0 }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jcondition = { \
                                    .type = GE_JOYSPRINGFORCE, \
                                    .saturation = { \
                                            .left = SL, \
                                            .right = SR \
                                    }, \
                                    .coefficient = { \
                                            .left = CL, \
                                            .right = CR \
                                    }, \
                                    .center = C, \
                                    .deadband = D, \
                            } \
                    } \
            } \
    }

#define HR_DAMPER(NAME, PRODUCT_ID, INVERT, D_GAIN, K1, S1, K2, S2, CLIP, SL, SR, CL, CR) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER, \
                    .parameters = { K1, S1, K2, S2, CLIP } \
            }, \
            .invert = INVERT, \
            .gain = { 0, 0, D_GAIN }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jcondition = { \
                                    .type = GE_JOYDAMPERFORCE, \
                                    .saturation = { \
                                            .left = SL, \
                                            .right = SR \
                                    }, \
                                    .coefficient = { \
                                            .left = CL, \
                                            .right = CR \
                                    } \
                            } \
                    } \
            } \
    }

#define SPRING(NAME, PRODUCT_ID, INVERT, S_GAIN, D1, D2, K2, K1, S2, S1, CLIP, SL, SR, CL, CR, C, D) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_SPRING, \
                    .parameters = { D1, D2, K2 | K1, S2 | S1, CLIP } \
            }, \
            .invert = INVERT, \
            .gain = { 0, S_GAIN, 0 }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jcondition = { \
                                    .type = GE_JOYSPRINGFORCE, \
                                    .saturation = { \
                                            .left = SL, \
                                            .right = SR \
                                    }, \
                                    .coefficient = { \
                                            .left = CL, \
                                            .right = CR \
                                    }, \
                                    .center = C, \
                                    .deadband = D, \
                            } \
                    } \
            } \
    }

#define DAMPER_TEST(NAME, PRODUCT_ID, INVERT, D_GAIN, K1, K2, S1, S2, CL, CR) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_DAMPER, \
                    .parameters = { K1, K2, S1, S2 } \
            }, \
            .invert = INVERT, \
            .gain = { 0, 0, D_GAIN }, \
            .nb_events = 1, \
            .events = { \
                    { \
                            .jcondition = { \
                                    .type = GE_JOYDAMPERFORCE, \
                                    .saturation = { \
                                            .left = 0, \
                                            .right = 0 \
                                    }, \
                                    .coefficient = { \
                                            .left = CL, \
                                            .right = CR \
                                    }, \
                                    .center = 0, \
                                    .deadband = 0, \
                            } \
                    } \
            } \
    }

static const struct {
    char * name;
    unsigned short product;
    union {
        unsigned char data[FF_LG_OUTPUT_REPORT_SIZE];
        struct PACKED {
            unsigned char cmd;
            s_ff_lg_force report;
        };
    };
    int invert;
    struct {
        int constant;
        int spring;
        int damper;
    } gain;
    unsigned int nb_events;
    GE_Event events[FF_LG_FSLOTS_NB];
} test_cases[] = {

        CONSTANT("G29 constant force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 0xff, 32767),
        CONSTANT("G29 constant force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 0, -32767),
        CONSTANT("G29 constant force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 128, 0),

//        CONSTANT("G29 constant force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 0xff, 16383),
//        CONSTANT("G29 constant force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 0, -16383),
//        CONSTANT("G29 constant force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 128, 0),

        CONSTANT("G29 constant force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 0xff, -32767),
        CONSTANT("G29 constant force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 0, 32767),
        CONSTANT("G29 constant force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 128, 0),

        DAMPER_TEST("G29 damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x07, 0x00, 0x00, 0x00,
                32767, 2047),
        DAMPER_TEST("G29 damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x00, 0x00, 0x07, 0x00,
                2047, 32767),
        DAMPER_TEST("G29 damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x00, 0x00, 0x00, 0x00,
                2047, 2047),
        DAMPER_TEST("G29 damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x07, 0x01, 0x07, 0x01,
                -32767, -32767),

//        DAMPER_TEST("G29 damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x07, 0x00, 0x00, 0x00,
//                16383, 1023),
//        DAMPER_TEST("G29 damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x00, 0x00, 0x07, 0x00,
//                1023, 16383),
//        DAMPER_TEST("G29 damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x00, 0x00, 0x00, 0x00,
//                1023, 1023),
//        DAMPER_TEST("G29 damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x07, 0x01, 0x07, 0x01,
//                -16383, -16383),

        DAMPER_TEST("G29 damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x07, 0x00, 0x00, 0x00,
                2047, 32767),
        DAMPER_TEST("G29 damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x00, 0x00, 0x07, 0x00,
                32767, 2047),
        DAMPER_TEST("G29 damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x00, 0x00, 0x00, 0x00,
                2047, 2047),
        DAMPER_TEST("G29 damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x07, 0x01, 0x07, 0x01,
                -32767, -32767),

        DAMPER_TEST("FFGP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, 0, 100,
                0x05, 0x00, 0x05, 0x00,
                24575, 24575),
        DAMPER_TEST("FFGP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, 0, 100,
                0x06, 0x00, 0x06, 0x00,
                16383, 16383),

        DAMPER_TEST("DFP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, 0, 100,
                0x05, 0x00, 0x05, 0x00,
                16383, 16383),
        DAMPER_TEST("DFP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, 0, 100,
                0x06, 0x00, 0x06, 0x00,
                24575, 24575),

        SPRING("G29 spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                128, 128, 0x00, 0x07, 0x00, 0x00, 255,
                65535, 65535, 32767, 2047, 0, 0),
        SPRING("G29 spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                128, 128, 0x70, 0x00, 0x00, 0x00, 255,
                65535, 65535, 2047, 32767, 0, 0),
        SPRING("G29 spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0, 0, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 2047, 2047, -32767, 0),
        SPRING("G29 spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff,
                65535, 65535, -32767, -32767, 32767, 0),

//        SPRING("G29 spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                128, 128, 0x00, 0x07, 0x00, 0x00, 255,
//                32767, 32767, 16383, 1023, 0, 0),
//        SPRING("G29 spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                128, 128, 0x70, 0x00, 0x00, 0x00, 255,
//                32767, 32767, 1023, 16383, 0, 0),
//        SPRING("G29 spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0, 0, 0x00, 0x00, 0x00, 0x00, 0,
//                0, 0, 1023, 1023, -32767, 0),
//        SPRING("G29 spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff,
//                32767, 32767, -16383, -16383, 32767, 0),

        SPRING("G29 spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                128, 128, 0x00, 0x07, 0x00, 0x00, 255,
                65535, 65535, 2047, 32767, 0, 0),
        SPRING("G29 spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                128, 128, 0x70, 0x00, 0x00, 0x00, 255,
                65535, 65535, 32767, 2047, 0, 0),
        SPRING("G29 spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0, 0, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 2047, 2047, 32767, 0),
        SPRING("G29 spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff,
                65535, 65535, -32767, -32767, -32767, 0),

        SPRING("FFGP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, 0, 100,
                128, 128, 0x50, 0x05, 0x00, 0x00, 0,
                0, 0, 24575, 24575, 0, 0),
        SPRING("FFGP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, 0, 100,
                128, 128, 0x60, 0x06, 0x00, 0x00, 0,
                0, 0, 16383, 16383, 0, 0),

        SPRING("DFP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, 0, 100,
                128, 128, 0x50, 0x05, 0x00, 0x00, 0,
                0, 0, 16383, 16383, 0, 0),
        SPRING("DFP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, 0, 100,
                128, 128, 0x60, 0x06, 0x00, 0x00, 0,
                0, 0, 24575, 24575, 0, 0),

        VARIABLE("G29 variable force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 0xff, 32767),
        VARIABLE("G29 variable force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 0, -32767),
        VARIABLE("G29 variable force (null, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100, 128, 0),

//        VARIABLE("G29 variable force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 0xff, 16383),
//        VARIABLE("G29 variable force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 0, -16383),
//        VARIABLE("G29 variable force (null, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50, 128, 0),

        VARIABLE("G29 variable force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 0xff, -32767),
        VARIABLE("G29 variable force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 0, 32767),
        VARIABLE("G29 variable force (null, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100, 128, 0),

        HR_SPRING("G29 high resolution spring force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 32767, 0, 0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX, 0),
        HR_SPRING("G29 high resolution spring force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 0, 32767, 0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX, 0),
        HR_SPRING("G29 high resolution spring force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 0, 0, -32767, 0),
        HR_SPRING("G29 high resolution spring force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff,
                65535, 65535, -32767, -32767, 32767, 0),

//        HR_SPRING("G29 high resolution spring force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff,
//                32767, 32767, 16383, 0, 0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX, 0),
//        HR_SPRING("G29 high resolution spring force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff,
//                32767, 32767, 0, 16383, 0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX, 0),
//        HR_SPRING("G29 high resolution spring force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0,
//                0, 0, 0, 0, -32767, 0),
//        HR_SPRING("G29 high resolution spring force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff,
//                32767, 32767, -16383, -16383, 32767, 0),

        HR_SPRING("G29 high resolution spring force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 0, 32767, - (0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX), 0),
        HR_SPRING("G29 high resolution spring force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 32767, 0, - (0x3FF * USHRT_MAX / 0x7FF - SHRT_MAX), 0),
        HR_SPRING("G29 high resolution spring force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 0, 0, 32767, 0),
        HR_SPRING("G29 high resolution spring force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff,
                65535, 65535, -32767, -32767, -32767, 0),

        HR_DAMPER("G29 high resolution damper force (left, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x0f, 0x00, 0x00, 0x00, 0xff,
                65535, 65535, 32767, 0),
        HR_DAMPER("G29 high resolution damper force (right, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x00, 0x00, 0x0f, 0x00, 0xff,
                65535, 65535, 0, 32767),
        HR_DAMPER("G29 high resolution damper force (zeroes, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x00, 0x00, 0x00, 0x00, 0,
                65535, 65535, 0, 0),
        HR_DAMPER("G29 high resolution damper force (ones, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 100,
                0x0f, 0x01, 0x0f, 0x01, 0xff,
                65535, 65535, -32767, -32767),

//        HR_DAMPER("G29 high resolution damper force (left, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x0f, 0x00, 0x00, 0x00, 0xff,
//                32767, 32767, 16383, 0),
//        HR_DAMPER("G29 high resolution damper force (right, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x00, 0x00, 0x0f, 0x00, 0xff,
//                32767, 32767, 0, 16383),
//        HR_DAMPER("G29 high resolution damper force (zeroes, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x00, 0x00, 0x00, 0x00, 0,
//                32767, 32767, 0, 0),
//        HR_DAMPER("G29 high resolution damper force (ones, 50%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, 50,
//                0x0f, 0x01, 0x0f, 0x01, 0xff,
//                32767, 32767, -16383, -16383),

        HR_DAMPER("G29 high resolution damper force (left, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x0f, 0x00, 0x00, 0x00, 0xff,
                65535, 65535, 0, 32767),
        HR_DAMPER("G29 high resolution damper force (right, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x00, 0x00, 0x0f, 0x00, 0xff,
                65535, 65535, 32767, 0),
        HR_DAMPER("G29 high resolution damper force (zeroes, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x00, 0x00, 0x00, 0x00, 0,
                65535, 65535, 0, 0),
        HR_DAMPER("G29 high resolution damper force (ones, inverted, 100%)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 1, 100,
                0x0f, 0x01, 0x0f, 0x01, 0xff,
                65535, 65535, -32767, -32767),
};

int compare_forces(const GE_Event * left, const GE_Event * right) {

    if (left->type != right->type) {
        return 1;
    }

    int ret = 0;

    switch (left->type) {
    case GE_JOYCONSTANTFORCE:
        if (left->jconstant.level != right->jconstant.level) {
            fprintf(stderr, "level: %d (ref) vs %d (res)\n", left->jconstant.level, right->jconstant.level);
            ret = 1;
        }
        break;
    case GE_JOYSPRINGFORCE:
    case GE_JOYDAMPERFORCE:
        if (left->jcondition.saturation.left != right->jcondition.saturation.left) {
            fprintf(stderr, "saturation.left: %u (ref) vs %u (res)\n", left->jcondition.saturation.left, right->jcondition.saturation.left);
            ret = 1;
        }
        if (left->jcondition.saturation.right != right->jcondition.saturation.right) {
            fprintf(stderr, "saturation.right: %u (ref) vs %u (res)\n", left->jcondition.saturation.right, right->jcondition.saturation.right);
            ret = 1;
        }
        if (left->jcondition.coefficient.left != right->jcondition.coefficient.left) {
            fprintf(stderr, "coefficient.left: %d (ref) vs %d (res)\n", left->jcondition.coefficient.left, right->jcondition.coefficient.left);
            ret = 1;
        }
        if (left->jcondition.coefficient.right != right->jcondition.coefficient.right) {
            fprintf(stderr, "coefficient.right: %d (ref) vs %d (res)\n", left->jcondition.coefficient.right, right->jcondition.coefficient.right);
            ret = 1;
        }
        if (left->jcondition.center != right->jcondition.center) {
            fprintf(stderr, "center: %d (ref) vs %d (res)\n", left->jcondition.center, right->jcondition.center);
            ret = 1;
        }
        if (left->jcondition.deadband != right->jcondition.deadband) {
            fprintf(stderr, "deadband: %u (ref) vs %u (res)\n", left->jcondition.deadband, right->jcondition.deadband);
            ret = 1;
        }
        break;
    }

    return ret;
}

int main(int argc __attribute__((unused)), char * argv[] __attribute__((unused))) {

    unsigned int i;
    for (i = 0; i < sizeof(test_cases) / sizeof(*test_cases); ++i) {

        ff_conv_init(0, test_cases[i].product);

        ff_conv_set_tweaks(0, test_cases[i].invert);

        ff_conv_process_report(0, test_cases[i].data);

        GE_Event event = {};
        ff_conv_get_event(0, &event);

        printf("test case: %s: ", test_cases[i].name);

        if (compare_forces(test_cases[i].events, &event)) {
            printf("failed: ff_conv returned an incorrect event\n");
            continue;
        }

        ff_conv_ack(0);

        unsigned char stop[FF_LG_OUTPUT_REPORT_SIZE] = { (test_cases[i].data[0] & FF_LG_FSLOT_MASK) | FF_LG_CMD_STOP };

        ff_conv_process_report(0, stop);

        ff_conv_get_event(0, &event);

        GE_Event stopEvent = { .type = event.type, .which = event.which };

        if (compare_forces(&stopEvent, &event)) {
            printf("failed: ff_conv returned an incorrect stop event\n");
            continue;
        }

        ff_conv_ack(0);

        printf("success\n");
    }

    return 0;
}
