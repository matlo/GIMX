/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_LG_H_
#define FF_LG_H_

#include <ginput.h>

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

#define USB_VENDOR_ID_LOGITECH                  0x046d

#define USB_PRODUCT_ID_LOGITECH_FORMULA_YELLOW   0xc202 // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_GP       0xc20e // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE    0xc291
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP 0xc293
#define USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE    0xc294
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL       0xc295
#define USB_PRODUCT_ID_LOGITECH_DFP_WHEEL        0xc298
#define USB_PRODUCT_ID_LOGITECH_G25_WHEEL        0xc299
#define USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL       0xc29a
#define USB_PRODUCT_ID_LOGITECH_G27_WHEEL        0xc29b
#define USB_PRODUCT_ID_LOGITECH_WII_WHEEL        0xc29c // rumble only
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2      0xca03
#define USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL  0xca04 // rumble only
#define USB_PRODUCT_ID_LOGITECH_G920_WHEEL       0xc262
#define USB_PRODUCT_ID_LOGITECH_G29_WHEEL        0xc24f

#define FF_LG_OUTPUT_REPORT_SIZE 7

#define FF_LG_FSLOTS_NB 4

#define FF_LG_FSLOT_MASK 0xf0

#define FF_LG_FSLOT_1 0x10
#define FF_LG_FSLOT_2 0x20
#define FF_LG_FSLOT_3 0x40
#define FF_LG_FSLOT_4 0x80

#define FF_LG_CMD_MASK 0x0f

#define FF_LG_CMD_DOWNLOAD           0x00
#define FF_LG_CMD_DOWNLOAD_AND_PLAY  0x01
#define FF_LG_CMD_PLAY               0x02
#define FF_LG_CMD_STOP               0x03
#define FF_LG_CMD_DEFAULT_SPRING_ON  0x04
#define FF_LG_CMD_DEFAULT_SPRING_OFF 0x05
#define FF_LG_CMD_RESERVED_1         0x06
#define FF_LG_CMD_RESERVED_2         0x07
#define FF_LG_CMD_NORMAL_MODE        0x08
#define FF_LG_CMD_EXTENDED_COMMAND   0xF8
#define FF_LG_CMD_SET_LED            0x09
#define FF_LG_CMD_SET_WATCHDOG       0x0A
#define FF_LG_CMD_RAW_MODE           0x0B
#define FF_LG_CMD_REFRESH_FORCE      0x0C
#define FF_LG_CMD_FIXED_TIME_LOOP    0x0D
#define FF_LG_CMD_SET_DEFAULT_SPRING 0x0E
#define FF_LG_CMD_SET_DEAD_BAND      0x0F

#define FF_LG_EXT_CMD_NB 16

#define FF_LG_EXT_CMD_CHANGE_MODE_DFP           0x01
#define FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES   0x02
#define FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES   0x03
#define FF_LG_EXT_CMD_CHANGE_MODE               0x09
#define FF_LG_EXT_CMD_REVERT_IDENTITY           0x0a
#define FF_LG_EXT_CMD_CHANGE_MODE_G25           0x10
#define FF_LG_EXT_CMD_CHANGE_MODE_G25_NO_DETACH 0x11
#define FF_LG_EXT_CMD_SET_RPM_LEDS              0x12
#define FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE        0x81

#define FF_LG_FTYPE_CONSTANT                           0x00
#define FF_LG_FTYPE_SPRING                             0x01
#define FF_LG_FTYPE_DAMPER                             0x02
#define FF_LG_FTYPE_AUTO_CENTER_SPRING                 0x03
#define FF_LG_FTYPE_SAWTOOTH_UP                        0x04
#define FF_LG_FTYPE_SAWTOOTH_DOWN                      0x05
#define FF_LG_FTYPE_TRAPEZOID                          0x06
#define FF_LG_FTYPE_RECTANGLE                          0x07
#define FF_LG_FTYPE_VARIABLE                           0x08
#define FF_LG_FTYPE_RAMP                               0x09
#define FF_LG_FTYPE_SQUARE_WAVE                        0x0A
#define FF_LG_FTYPE_HIGH_RESOLUTION_SPRING             0x0B
#define FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER             0x0C
#define FF_LG_FTYPE_HIGH_RESOLUTION_AUTO_CENTER_SPRING 0x0D
#define FF_LG_FTYPE_FRICTION                           0x0E

typedef struct PACKED {
    unsigned char type; // FTYPE_CONSTANT
    unsigned char levels[4]; // index n for slot n
    unsigned char : 8;
} s_ff_lg_constant;

typedef struct PACKED {
    unsigned char type; // FTYPE_VARIABLE
    unsigned char l1; // initial level for force 0
    unsigned char l2; // initial level for force 2
    unsigned char t1 : 4; // force 0 step duration (in main loops)
    unsigned char s1 : 4; // force 0 step size
    unsigned char t2 : 4; // force 2 step duration (in main loops)
    unsigned char s2 : 4; // force 2 step size
    unsigned char : 3;
    unsigned char d1 : 1; // force 0 direction (0 = increasing, 1 = decreasing)
    unsigned char : 3;
    unsigned char d2 : 1; // force 2 direction (0 = increasing, 1 = decreasing)
} s_ff_lg_variable;

typedef struct PACKED {
    unsigned char type; // FTYPE_SPRING
    unsigned char d1; // lower limit of central dead band
    unsigned char d2; // upper limit of central dead band
    unsigned char : 1;
    unsigned char k1 : 3; //Low (left or push) side spring constant selector
    unsigned char : 1;
    unsigned char k2 : 3; //High (right or pull) side spring constant selector
    unsigned char : 3;
    unsigned char s1 : 1; //Low side slope inversion (1 = inverted)
    unsigned char : 3;
    unsigned char s2 : 1; //High side slope inversion (1 = inverted)
    unsigned char clip; //Clip level (maximum force), on either side
} s_ff_lg_spring;

typedef struct PACKED {
    unsigned char type; // FTYPE_DAMPER
    unsigned char : 5;
    unsigned char k1 : 3; //Low (left or push) side damper coefficient
    unsigned char : 7;
    unsigned char s1 : 1; //Low side inversion (1 = inverted)
    unsigned char : 5;
    unsigned char k2 : 3; //High (right or pull) side damper coefficient
    unsigned char : 7;
    unsigned char s2 : 1; //High side inversion (1 = inverted)
    unsigned char : 8;
} s_ff_lg_damper;

typedef struct PACKED {
    unsigned char type; // FTYPE_HIGH_RESOLUTION_SPRING
    unsigned char d1; // lower limit of central dead band
    unsigned char d2; // upper limit of central dead band
    unsigned char k1 : 4; //Low (left or push) side spring constant selector
    unsigned char k2 : 4; //High (right or pull) side spring constant selector
    unsigned char d1L : 3; // low order bits (since Driving Force Pro)
    unsigned char s1 : 1; //Low side slope inversion (1 = inverted)
    unsigned char d2L : 3; // low order bits (since Driving Force Pro)
    unsigned char s2 : 1; //High side slope inversion (1 = inverted)
    unsigned char clip; //Clip level (maximum force), on either side
} s_ff_lg_hr_spring;

typedef struct PACKED {
    unsigned char type; // FTYPE_HIGH_RESOLUTION_DAMPER
    unsigned char : 4;
    unsigned char k1 : 4; //Low (left or push) side damper coefficient
    unsigned char : 7;
    unsigned char s1 : 1; //Low side inversion (1 = inverted)
    unsigned char : 4;
    unsigned char k2 : 4; //High (right or pull) side damper coefficient
    unsigned char : 7;
    unsigned char s2 : 1; //High side inversion (1 = inverted)
    unsigned char clip; // (since Driving Force Pro)
} s_ff_lg_hr_damper;

typedef union {
    unsigned char type;
    s_ff_lg_constant constant;
    s_ff_lg_variable variable;
    s_ff_lg_spring spring;
    s_ff_lg_damper damper;
    s_ff_lg_hr_spring hr_spring;
    s_ff_lg_hr_damper hr_damper;
} s_ff_lg_force;

typedef struct {
  unsigned char data[FF_LG_OUTPUT_REPORT_SIZE + 1];
} s_ff_lg_report;

int ff_lg_init(int device, unsigned short pid_from, unsigned short pid_to);
void ff_lg_decode_extended(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
void ff_lg_decode_command(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
void ff_lg_process_report(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
s_ff_lg_report * ff_lg_get_report(int device);
void ff_lg_ack(int device);

int16_t ff_lg_get_condition_coef(unsigned short pid, unsigned char hr, unsigned char k, unsigned char s);
uint16_t ff_lg_get_spring_deadband(unsigned short pid, unsigned char d, unsigned char dL);
uint16_t ff_lg_get_damper_clip(unsigned short pid, unsigned char c);

int ff_lg_is_logitech_wheel(unsigned short vendor, unsigned short product);

#endif /* FF_LG_H_ */
