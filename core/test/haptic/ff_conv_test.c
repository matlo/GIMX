/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define SDL_MAIN_HANDLED

#include <gimx.h>
#include <limits.h>
#include <string.h>
#include <haptic/ff_conv.h>

s_gimx_params gimx_params = { 0 }; // { .debug = { .ff_lg = 1, .ff_conv = 1 } };

#define CONSTANT(NAME, PRODUCT_ID, LEVEL_IN, LEVEL_OUT) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_CONSTANT, \
                    .parameters = { LEVEL_IN }, \
            }, \
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

#define VARIABLE(NAME, PRODUCT_ID, LEVEL_IN, LEVEL_OUT) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_VARIABLE, \
                    .parameters = { LEVEL_IN }, \
            }, \
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

#define HR_SPRING(NAME, PRODUCT_ID, D1, D2, K2, K1, D2L, S2, D1L, S1, CLIP, SL, SR, CL, CR, C, D) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING, \
                    .parameters = { D1, D2, K2 | K1, D2L | S2 | D1L | S1, CLIP } \
            }, \
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

#define HR_DAMPER(NAME, PRODUCT_ID, K1, S1, K2, S2, CLIP, SL, SR, CL, CR) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER, \
                    .parameters = { K1, S1, K2, S2, CLIP } \
            }, \
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

#define SPRING(NAME, PRODUCT_ID, D1, D2, K2, K1, S2, S1, CLIP, SL, SR, CL, CR, C, D) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_SPRING, \
                    .parameters = { D1, D2, K2 | K1, S2 | S1, CLIP } \
            }, \
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

#define DAMPER(NAME, PRODUCT_ID, K1, K2, S1, S2, CL, CR) \
    { \
            .name = NAME, \
            .product = PRODUCT_ID, \
            .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY, \
            .report = { \
                    .type = FF_LG_FTYPE_DAMPER, \
                    .parameters = { K1, K2, S1, S2 } \
            }, \
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
    unsigned int nb_events;
    GE_Event events[FF_LG_FSLOTS_NB];
} test_cases[] = {

        CONSTANT("G29 constant force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0xff, 32767),
        CONSTANT("G29 constant force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, -32768),
        CONSTANT("G29 constant force (null)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 128, 0),

        DAMPER("G29 damper force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x07, 0x00, 0x00, 0x00,
                32767, 2047),
        DAMPER("G29 damper force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x00, 0x00, 0x07, 0x00,
                2047, 32767),
        DAMPER("G29 damper force (zeroes)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x00, 0x00, 0x00, 0x00,
                2047, 2047),
        DAMPER("G29 damper force (ones)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x07, 0x01, 0x07, 0x01,
                -32768, -32768),

        DAMPER("FFGP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
                0x05, 0x00, 0x05, 0x00,
                24575, 24575),
        DAMPER("FFGP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
                0x06, 0x00, 0x06, 0x00,
                16383, 16383),

        DAMPER("DFP damper force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
                0x05, 0x00, 0x05, 0x00,
                16383, 16383),
        DAMPER("DFP damper force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
                0x06, 0x00, 0x06, 0x00,
                24575, 24575),

        SPRING("G29 spring force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                128, 128, 0x00, 0x07, 0x00, 0x00, 255,
                65535, 65535, 32767, 2047, 0, 0),
        SPRING("G29 spring force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                128, 128, 0x70, 0x00, 0x00, 0x00, 255,
                65535, 65535, 2047, 32767, 0, 0),
        SPRING("G29 spring force (zeroes)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0, 0, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 2047, 2047, -32768, 0),
        SPRING("G29 spring force (ones)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0xff, 0xff, 0x70, 0x07, 0x10, 0x01, 0xff,
                65535, 65535, -32768, -32768, 32767, 0),

        SPRING("FFGP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
                128, 128, 0x50, 0x05, 0x00, 0x00, 0,
                0, 0, 24575, 24575, 0, 0),
        SPRING("FFGP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
                128, 128, 0x60, 0x06, 0x00, 0x00, 0,
                0, 0, 16383, 16383, 0, 0),

        SPRING("DFP spring force (c=5)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
                128, 128, 0x50, 0x05, 0x00, 0x00, 0,
                0, 0, 16383, 16383, 0, 0),
        SPRING("DFP spring force (c=6)", USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
                128, 128, 0x60, 0x06, 0x00, 0x00, 0,
                0, 0, 24575, 24575, 0, 0),

        VARIABLE("G29 variable force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0xff, 32767),
        VARIABLE("G29 variable force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 0, -32768),
        VARIABLE("G29 variable force (null)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL, 128, 0),

        HR_SPRING("G29 high resolution spring force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x7f, 0x7f, 0x00, 0x0f, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 32767, 0, 0x3FF * 65535 / 0x7FF - -32768, 0),
        HR_SPRING("G29 high resolution spring force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x7f, 0x7f, 0xf0, 0x00, 0xe0, 0x00, 0x0e, 0x00, 0xff,
                65535, 65535, 0, 32767, 0x3FF * 65535 / 0x7FF - -32768, 0),
        HR_SPRING("G29 high resolution spring force (zeroes)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0,
                0, 0, 0, 0, -32768, 0),
        HR_SPRING("G29 high resolution spring force (ones)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0xff, 0xff, 0xf0, 0x0f, 0xe0, 0x10, 0x0e, 0x01, 0xff,
                65535, 65535, -32768, -32768, 32767, 0),

        HR_DAMPER("G29 high resolution damper force (left)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x0f, 0x00, 0x00, 0x00, 0xff,
                65535, 65535, 32767, 0),
        HR_DAMPER("G29 high resolution damper force (right)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x00, 0x00, 0x0f, 0x00, 0xff,
                65535, 65535, 0, 32767),
        HR_DAMPER("G29 high resolution damper force (zeroes)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x00, 0x00, 0x00, 0x00, 0,
                65535, 65535, 0, 0),
        HR_DAMPER("G29 high resolution damper force (ones)", USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                0x0f, 0x01, 0x0f, 0x01, 0xff,
                65535, 65535, -32768, -32768),
};

int compare_events(const GE_Event * left, const GE_Event * right) {

    if (left->type != right->type) {
        return 1;
    }

    switch (left->type) {
    case GE_JOYCONSTANTFORCE:
        if (left->jconstant.level != right->jconstant.level) {
            return 1;
        }
        break;
    case GE_JOYSPRINGFORCE:
    case GE_JOYDAMPERFORCE:
        if (left->jcondition.saturation.left != right->jcondition.saturation.left) {
            return 1;
        }
        if (left->jcondition.saturation.right != right->jcondition.saturation.right) {
            return 1;
        }
        if (left->jcondition.coefficient.left != right->jcondition.coefficient.left) {
            return 1;
        }
        if (left->jcondition.coefficient.right != right->jcondition.coefficient.right) {
            return 1;
        }
        if (left->jcondition.center != right->jcondition.center) {
            return 1;
        }
        if (left->jcondition.deadband != right->jcondition.deadband) {
            return 1;
        }
        break;
    }

    return 0;
}

int main(int argc __attribute__((unused)), char * argv[] __attribute__((unused))) {

    unsigned int i;
    for (i = 0; i < sizeof(test_cases) / sizeof(*test_cases); ++i) {

        ff_conv_init(0, test_cases[i].product);

        ff_conv_process_report(0, test_cases[i].data);

        GE_Event event = {};
        ff_conv_get_event(0, &event);

        printf("test case: %s: ", test_cases[i].name);

        if (compare_events(test_cases[i].events, &event)) {
            printf("failed: ff_conv returned an incorrect event\n");
            continue;
        }

        printf("success\n");

        ff_conv_ack(0);
    }

    return 0;
}
