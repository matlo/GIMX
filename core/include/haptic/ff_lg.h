/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_LG_H_
#define FF_LG_H_

#include <gimxinput/include/ginput.h>

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
#define USB_PRODUCT_ID_LOGITECH_G920_WHEEL       0xc262 // hid++ protocol
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

#define FF_LG_CONSTANT_LEVEL(FORCE, SLOT_INDEX) (FORCE->parameters)[SLOT_INDEX] // force level (index n for slot n)

#define FF_LG_VARIABLE_L1(FORCE) (FORCE->parameters)[0]                 // initial level for force 0
#define FF_LG_VARIABLE_L2(FORCE) (FORCE->parameters)[1]                 // initial level for force 2
#define FF_LG_VARIABLE_S1(FORCE) ((FORCE->parameters)[2] & 0x0f)        // force 0 step size
#define FF_LG_VARIABLE_T1(FORCE) (((FORCE->parameters)[2] & 0xf0) >> 4) // force 0 step duration (in main loops)
#define FF_LG_VARIABLE_S2(FORCE) ((FORCE->parameters)[3] & 0x0f)        // force 2 step size
#define FF_LG_VARIABLE_T2(FORCE) (((FORCE->parameters)[3] & 0xf0) >> 4) // force 2 step duration (in main loops)
#define FF_LG_VARIABLE_D1(FORCE) ((FORCE->parameters)[4] & 0x01)        // force 0 direction (0 = increasing, 1 = decreasing)
#define FF_LG_VARIABLE_D2(FORCE) (((FORCE->parameters)[4] & 0x10) >> 4) // force 2 direction (0 = increasing, 1 = decreasing)

#define FF_LG_SPRING_D1(FORCE)   (FORCE->parameters)[0]                 // lower limit of central dead band
#define FF_LG_SPRING_D2(FORCE)   (FORCE->parameters)[1]                 // upper limit of central dead band
#define FF_LG_SPRING_K1(FORCE)   ((FORCE->parameters)[2] & 0x07)        // low (left or push) side spring constant selector
#define FF_LG_SPRING_K2(FORCE)   (((FORCE->parameters)[2] & 0x70) >> 4) // high (right or pull) side spring constant selector
#define FF_LG_SPRING_S1(FORCE)   ((FORCE->parameters)[3] & 0x01)        // low side slope inversion (1 = inverted)
#define FF_LG_SPRING_S2(FORCE)   (((FORCE->parameters)[3] & 0x10) >> 4) // high side slope inversion (1 = inverted)
#define FF_LG_SPRING_CLIP(FORCE) (FORCE->parameters)[4]                 // clip level (maximum force), on either side

#define FF_LG_DAMPER_K1(FORCE)   ((FORCE->parameters)[0] & 0x07) // low (left or push) side damper coefficient
#define FF_LG_DAMPER_S1(FORCE)   ((FORCE->parameters)[1] & 0x01) // low side inversion (1 = inverted)
#define FF_LG_DAMPER_K2(FORCE)   ((FORCE->parameters)[2] & 0x07) // high (right or pull) side damper coefficient
#define FF_LG_DAMPER_S2(FORCE)   ((FORCE->parameters)[3] & 0x01) // high side inversion (1 = inverted)

#define FF_LG_HIGHRES_SPRING_D1(FORCE)   (FORCE->parameters)[0]                 // lower limit of central dead band
#define FF_LG_HIGHRES_SPRING_D2(FORCE)   (FORCE->parameters)[1]                 // upper limit of central dead band
#define FF_LG_HIGHRES_SPRING_K1(FORCE)   ((FORCE->parameters)[2] & 0x0f)        // low (left or push) side spring constant selector
#define FF_LG_HIGHRES_SPRING_K2(FORCE)   (((FORCE->parameters)[2] & 0xf0) >> 4) // high (right or pull) side spring constant selector
#define FF_LG_HIGHRES_SPRING_S1(FORCE)   ((FORCE->parameters)[3] & 0x01)        // low side slope inversion (1 = inverted)
#define FF_LG_HIGHRES_SPRING_D1L(FORCE)  (((FORCE->parameters)[3] & 0x0e) >> 1) // low order bits (since Driving Force Pro)
#define FF_LG_HIGHRES_SPRING_S2(FORCE)   (((FORCE->parameters)[3] & 0x10) >> 4) // high side slope inversion (1 = inverted)
#define FF_LG_HIGHRES_SPRING_D2L(FORCE)  (((FORCE->parameters)[3] & 0xe0) >> 5) // low order bits (since Driving Force Pro)
#define FF_LG_HIGHRES_SPRING_CLIP(FORCE) (FORCE->parameters)[4]                 // clip level (maximum force), on either side

#define FF_LG_HIGHRES_DAMPER_K1(FORCE)   ((FORCE->parameters)[0] & 0x0f) // low (left or push) side damper constant selector
#define FF_LG_HIGHRES_DAMPER_S1(FORCE)   ((FORCE->parameters)[1] & 0x01) // low side inversion (1 = inverted)
#define FF_LG_HIGHRES_DAMPER_K2(FORCE)   ((FORCE->parameters)[2] & 0x0f) // high (right or pull) side damper constant selector
#define FF_LG_HIGHRES_DAMPER_S2(FORCE)   ((FORCE->parameters)[3] & 0x01) // high side inversion (1 = inverted)
#define FF_LG_HIGHRES_DAMPER_CLIP(FORCE) (FORCE->parameters)[4]          // clip level (maximum force), on either side (since Driving Force Pro)

typedef struct PACKED {
    unsigned char type;
    unsigned char parameters[FF_LG_OUTPUT_REPORT_SIZE - 2];
} s_ff_lg_force;

typedef struct {
  unsigned char data[FF_LG_OUTPUT_REPORT_SIZE + 1];
} s_ff_lg_report;

int ff_lg_init(int device, unsigned short pid_from, unsigned short pid_to);
void ff_lg_decode_extended(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
void ff_lg_decode_command(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
void ff_lg_process_report(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
int ff_lg_get_report(int device, s_ff_lg_report * report);
void ff_lg_ack(int device);

int16_t ff_lg_get_condition_coef(unsigned short pid, unsigned char hr, unsigned char k, unsigned char s);
uint16_t ff_lg_get_spring_deadband(unsigned short pid, unsigned char d, unsigned char dL);
uint16_t ff_lg_get_damper_clip(unsigned short pid, unsigned char c);

int ff_lg_is_logitech_wheel(unsigned short vendor, unsigned short product);

const char * ff_lg_get_ftype_name(unsigned char ftype);

unsigned short ff_lg_get_wheel_range(unsigned short pid);

#endif /* FF_LG_H_ */
