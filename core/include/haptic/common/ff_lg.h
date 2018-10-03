/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_LG_H_
#define FF_LG_H_

#include <stdint.h>
#include <string.h>
#include <limits.h>

#include <haptic/haptic_core.h>
#include <haptic/haptic_common.h>

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

#define USB_VENDOR_ID_LOGITECH                  0x046d

#define USB_PRODUCT_ID_LOGITECH_FORMULA_YELLOW   0xc202 // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_GP       0xc20e // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE    0xc291 // i-force protocol
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP 0xc293 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE    0xc294 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL       0xc295 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DFP_WHEEL        0xc298 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G25_WHEEL        0xc299 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL       0xc29a // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G27_WHEEL        0xc29b // classic protocol
#define USB_PRODUCT_ID_LOGITECH_WII_WHEEL        0xc29c // rumble only
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2      0xca03 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL  0xca04 // rumble only
#define USB_PRODUCT_ID_LOGITECH_G920_XONE_WHEEL  0xc261 // Xbox One protocol
#define USB_PRODUCT_ID_LOGITECH_G920_WHEEL       0xc262 // hid++ protocol only
#define USB_PRODUCT_ID_LOGITECH_G29_PC_WHEEL     0xc24f // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL    0xc260 // classic protocol with 1 byte offset

#define FF_LG_OUTPUT_REPORT_SIZE 7

#define FF_LG_FSLOTS_NB 4
#define FF_LG_FSLOTS_OFFSET 4

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
#define FF_LG_HIGHRES_DAMPER_CLIP(FORCE) (FORCE->parameters)[4]          // clip level (maximum force), on either side (only for Driving Force Pro)

#define FF_LG_CAPS_HIGH_RES_COEF     (1 << 0)
#define FF_LG_CAPS_OLD_LOW_RES_COEF  (1 << 1)
#define FF_LG_CAPS_HIGH_RES_DEADBAND (1 << 2)
#define FF_LG_CAPS_DAMPER_CLIP       (1 << 3)
#define FF_LG_CAPS_LEDS              (1 << 4)
#define FF_LG_CAPS_RANGE_200_900     (1 << 5)
#define FF_LG_CAPS_RANGE             (1 << 6)

typedef struct PACKED {
    union {
        unsigned char force_type;
        unsigned char cmd_param;
    };
    unsigned char parameters[FF_LG_OUTPUT_REPORT_SIZE - 2];
} s_ff_lg_command;

typedef struct {
  unsigned char data[FF_LG_OUTPUT_REPORT_SIZE + 1];
} s_ff_lg_report;

const char * ff_lg_get_cmd_name(unsigned char header);
const char * ff_lg_get_ext_cmd_name(unsigned char ext);
const char * ff_lg_get_ftype_name(unsigned char ftype);
unsigned short ff_lg_get_wheel_range(unsigned short pid);

void ff_lg_decode_extended(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);
void ff_lg_decode_command(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);

uint8_t ff_lg_get_caps(uint16_t pid);

int ff_lg_convert_force(uint8_t caps, uint8_t slot_index, const s_ff_lg_command * force, uint8_t playing, s_haptic_core_data * to);
int ff_lg_convert_extended(const s_ff_lg_command * cmd, s_haptic_core_data * to);
void ff_lg_convert_slot(const s_haptic_core_data * from, int slot, s_ff_lg_report * to, uint8_t caps);

/*
 * Convert a Logitech wheel position to a signed 16-bit value.
 *
 * input values 127 and 128 are center positions and are translated to output value 0
 */
static inline int16_t ff_lg_u8_to_s16(uint8_t c) {
    // 127 and 128 are center positions
    if (c < 128) {
        ++c;
    }
    int value = (c - 128) * SHRT_MAX / 127;
    return value;
}

/*
 * Convert a signed 16-bit wheel position to an unsigned 8-bit wheel position.
 *
 * input value is in [-32767, 32767], -32768 is not expected
 */
static inline uint8_t ff_lg_s16_to_u8(int16_t s) {
    if (s < 0) {
        --s;
    }
    int value = s + 32768;
    return value * UCHAR_MAX / USHRT_MAX;
}

/*
 * Convert a signed 16-bit wheel position to an unsigned 11-bit wheel position.
 *
 * input value is in [-32767, 32767], -32768 is not expected
 */
static inline uint16_t ff_lg_s16_to_u11(int16_t s) {
    int value = s + 32767;
    if (s > 0) {
        ++value;
    }
    value = value * 2047 / USHRT_MAX;
    return value;
}

static inline uint16_t ff_lg_u8_to_u16(uint8_t c) {
    return c * 65535 / UCHAR_MAX;
}

static inline int16_t ff_lg_u16_to_s16(uint16_t s) {
    // 32767 and 32768 are center positions
    int value = s - 32768;
    if (value < 0) {
        ++value;
    }
    return value;
}

#define FIFO_SIZE 16

typedef struct {
    uint8_t cmd;
    uint8_t ext;
} s_cmd;

static inline int compare_cmd(s_cmd cmd1, s_cmd cmd2) {
    return cmd1.cmd == cmd2.cmd && (cmd1.cmd != FF_LG_CMD_EXTENDED_COMMAND || cmd1.ext == cmd2.ext);
}

static inline void ff_lg_fifo_push(s_cmd fifo[FIFO_SIZE], s_cmd cmd, int replace) {
    int i;
    for (i = 0; i < FIFO_SIZE; ++i) {
        if (!fifo[i].cmd) {
            fifo[i] = cmd; //add
            dprintf("> push:");
            break;
        } else if (replace && compare_cmd(fifo[i], cmd)) {
            dprintf("> already queued:");
            break;
        }
    }
    if(i == FIFO_SIZE) {
        PRINT_ERROR_OTHER("no more space in fifo")
        dprintf("> can't push:");
    }
    dprintf(" %02x", cmd.cmd);
    if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
        dprintf(" %02x", cmd.ext);
    }
    dprintf("\n");
}

static inline s_cmd ff_lg_fifo_peek(s_cmd fifo[FIFO_SIZE]) {
    s_cmd cmd = fifo[0];
    if (cmd.cmd) {
        dprintf("> peek: %02x", cmd.cmd);
        if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
            dprintf(" %02x", cmd.ext);
        }
        dprintf("\n");
    }
    return cmd;
}

static inline void ff_lg_fifo_remove(s_cmd fifo[FIFO_SIZE], s_cmd cmd) {
    int i;
    for (i = 0; i < FIFO_SIZE; ++i) {
        if (!fifo[i].cmd) {
            break;
        } else if (compare_cmd(fifo[i], cmd)) {
            dprintf("> remove %02x", cmd.cmd);
            if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
                dprintf(" %02x", cmd.ext);
            }
            dprintf("\n");
            memmove(fifo + i, fifo + i + 1, (FF_LG_FSLOTS_NB - i - 1) * sizeof(*fifo));
            memset(fifo + FF_LG_FSLOTS_NB - i - 1, 0x00, sizeof(*fifo));
            break;
        }
    }
}

#endif /* FF_LG_H_ */
