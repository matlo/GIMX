/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define SDL_MAIN_HANDLED

#include <gimx.h>
#include <limits.h>
#include <string.h>
#include <haptic/ff_conv.h>

s_gimx_params gimx_params = { 0 };

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
    int nb_events;
    GE_Event events[FF_LG_FSLOTS_NB];
} test_cases[] = {
        {
                .name = "G29 variable force (left)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_VARIABLE,
                        .parameters = { UCHAR_MAX },
                },
                .nb_events = 1,
                .events = {
                        {
                                .jconstant = {
                                        .type = GE_JOYCONSTANTFORCE,
                                        .level = SHRT_MAX
                                }
                        }
                }
        },
        {
                .name = "G29 variable force (right)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_VARIABLE,
                        .parameters = { 0 },
                },
                .nb_events = 1,
                .events = {
                        {
                                .jconstant = {
                                        .type = GE_JOYCONSTANTFORCE,
                                        .level = SHRT_MIN
                                }
                        }
                }
        },
        {
                .name = "G29 variable force (null)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_1 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_VARIABLE,
                        .parameters = { 128 },
                },
                .nb_events = 1,
                .events = {
                        {
                                .jconstant = {
                                        .type = GE_JOYCONSTANTFORCE,
                                        .level = 0
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution spring force (left)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                        .parameters = {
                                0x7f,      //d1
                                0x7f,      //d2
                                  0xf0     //k1
                                | 0x00,    //k2
                                  0xe0     //d1L
                                | 0x00     //s1
                                | 0x0e     // d2L
                                | 0x00,    //s2
                                UCHAR_MAX  //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYSPRINGFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = SHRT_MAX,
                                                .right = 0
                                        },
                                        .center = 0x3FF * USHRT_MAX / 0x7FF - SHRT_MIN,
                                        .deadband = 0,
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution spring force (right)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                        .parameters = {
                                0x7f,      //d1
                                0x7f,      //d2
                                  0x00     //k1
                                | 0x0f,    //k2
                                  0xe0     //d1L
                                | 0x00     //s1
                                | 0x0e     // d2L
                                | 0x00,    //s2
                                UCHAR_MAX  //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYSPRINGFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = 0,
                                                .right = SHRT_MAX
                                        },
                                        .center = 0x3FF * USHRT_MAX / 0x7FF - SHRT_MIN,
                                        .deadband = 0,
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution spring force (zeroes)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                        .parameters = {
                                0,         //d1
                                0,         //d2
                                  0x00     //k1
                                | 0x00,    //k2
                                  0x00     //d1L
                                | 0x00     //s1
                                | 0x00     // d2L
                                | 0x00,    //s2
                                0          //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYSPRINGFORCE,
                                        .saturation = {
                                                .left = 0,
                                                .right = 0
                                        },
                                        .coefficient = {
                                                .left = 0,
                                                .right = 0
                                        },
                                        .center = SHRT_MIN,
                                        .deadband = 0,
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution spring force (ones)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_2 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                        .parameters = {
                                UCHAR_MAX, //d1
                                UCHAR_MAX, //d2
                                  0xf0     //k1
                                | 0x0f,    //k2
                                  0xe0     //d1L
                                | 0x10     //s1
                                | 0x0e     // d2L
                                | 0x01,    //s2
                                UCHAR_MAX  //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYSPRINGFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = SHRT_MIN,
                                                .right = SHRT_MIN
                                        },
                                        .center = SHRT_MAX,
                                        .deadband = 0,
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution damper force (left)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER,
                        .parameters = {
                                0x0f,    //k1
                                0x00,    //s1
                                0x00,    //k2
                                0x00,    //s2
                                UCHAR_MAX  //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYDAMPERFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = SHRT_MAX,
                                                .right = 0
                                        },
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution damper force (right)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER,
                        .parameters = {
                                0x00,      //k1
                                0x00,      //s1
                                0x0f,      //k2
                                0x00,      //s2
                                UCHAR_MAX  //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYDAMPERFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = 0,
                                                .right = SHRT_MAX
                                        },
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution damper force (zeroes)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER,
                        .parameters = {
                                0x00,      //k1
                                0x00,      //s1
                                0x00,      //k2
                                0x00,      //s2
                                0          //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYDAMPERFORCE,
                                        .saturation = {
                                                .left = 0,
                                                .right = 0
                                        },
                                        .coefficient = {
                                                .left = 0,
                                                .right = 0
                                        },
                                }
                        }
                }
        },
        {
                .name = "G29 high resolution damper force (ones)",
                .product = USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
                .cmd = FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                .report = {
                        .type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER,
                        .parameters = {
                                0x0f,     //k1
                                0x01,     //s1
                                0x0f,     //k2
                                0x01,     //s2
                                UCHAR_MAX //clip
                        }
                },
                .nb_events = 1,
                .events = {
                        {
                                .jcondition = {
                                        .type = GE_JOYDAMPERFORCE,
                                        .saturation = {
                                                .left = USHRT_MAX,
                                                .right = USHRT_MAX
                                        },
                                        .coefficient = {
                                                .left = SHRT_MIN,
                                                .right = SHRT_MIN
                                        },
                                }
                        }
                }
        },
};

int main(int argc, char * argv[]) {

    unsigned int i;
    for (i = 0; i < sizeof(test_cases) / sizeof(*test_cases); ++i) {

        ff_conv_init(0, test_cases[i].product);

        GE_Event events[FF_LG_FSLOTS_NB] = {};

        int nb_events = ff_conv(0, test_cases[i].data, events);

        printf("test case: %s: ", test_cases[i].name);

        if (nb_events != test_cases[i].nb_events) {
            printf("failed: ff_conv returned an incorrect number of events\n");
            continue;
        }

        if (memcmp(test_cases[i].events, events, sizeof(test_cases[i].events))) {
            printf("failed: ff_conv returned an incorrect event\n");
            continue;
        }

        printf("success\n");
    }

    return 0;
}
