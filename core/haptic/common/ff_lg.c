/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include <stdio.h>

#include <haptic/common/ff_lg.h>
#include <haptic/haptic_common.h>

static const char * cmd_names[] = {
    [FF_LG_CMD_DOWNLOAD]           = "DOWNLOAD",
    [FF_LG_CMD_DOWNLOAD_AND_PLAY]  = "DOWNLOAD_AND_PLAY",
    [FF_LG_CMD_PLAY]               = "PLAY",
    [FF_LG_CMD_STOP]               = "STOP",
    [FF_LG_CMD_DEFAULT_SPRING_ON]  = "DEFAULT_SPRING_ON",
    [FF_LG_CMD_DEFAULT_SPRING_OFF] = "DEFAULT_SPRING_OFF",
    [FF_LG_CMD_RESERVED_1]         = "RESERVED_1",
    [FF_LG_CMD_RESERVED_2]         = "RESERVED_2",
    [FF_LG_CMD_NORMAL_MODE]        = "NORMAL_MODE",
    [FF_LG_CMD_SET_LED]            = "SET_LED",
    [FF_LG_CMD_SET_WATCHDOG]       = "SET_WATCHDOG",
    [FF_LG_CMD_RAW_MODE]           = "RAW_MODE",
    [FF_LG_CMD_REFRESH_FORCE]      = "REFRESH_FORCE",
    [FF_LG_CMD_FIXED_TIME_LOOP]    = "FIXED_TIME_LOOP",
    [FF_LG_CMD_SET_DEFAULT_SPRING] = "SET_DEFAULT_SPRING",
    [FF_LG_CMD_SET_DEAD_BAND]      = "SET_DEAD_BAND",
};

const char * ff_lg_get_cmd_name(unsigned char header) {
    if (header == FF_LG_CMD_EXTENDED_COMMAND) {
        return "EXTENDED_COMMAND";
    } else {
        unsigned char cmd = header & FF_LG_CMD_MASK;
        if (cmd < sizeof(cmd_names) / sizeof(*cmd_names)) {
            return cmd_names[cmd];
        } else {
            return "UNKNOWN";
        }
    }
}

char * slot_names[] = {
        [0b0000] = "",
        [0b0001] = "slot 1",
        [0b0010] = "slot 2",
        [0b0011] = "slots 1,2",
        [0b0100] = "slot 3",
        [0b0101] = "slots 1,3",
        [0b0110] = "slots 2,3",
        [0b0111] = "slots 1,2,3",
        [0b1000] = "slot 4",
        [0b1001] = "slots 1,4",
        [0b1010] = "slots 2,4",
        [0b1011] = "slots 1,2,4",
        [0b1100] = "slots 3,4",
        [0b1101] = "slots 1,3,4",
        [0b1110] = "slots 2,3,4",
        [0b1111] = "slots 1,2,3,4",
};

const char * ff_lg_get_slot_names(unsigned char header) {
    if (header == FF_LG_CMD_EXTENDED_COMMAND) {
        return "";
    } else {
        return slot_names[header >> 4];
    }
}

static struct {
    unsigned char value;
    const char * name;
} ext_cmd_names[] = {
    { FF_LG_EXT_CMD_CHANGE_MODE_DFP,           "CHANGE_MODE_DFP" },
    { FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES,   "WHEEL_RANGE_200_DEGREES" },
    { FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES,   "WHEEL_RANGE_900_DEGREES" },
    { FF_LG_EXT_CMD_CHANGE_MODE,               "CHANGE_MODE" },
    { FF_LG_EXT_CMD_REVERT_IDENTITY,           "REVERT_IDENTITY" },
    { FF_LG_EXT_CMD_CHANGE_MODE_G25,           "CHANGE_MODE_G25" },
    { FF_LG_EXT_CMD_CHANGE_MODE_G25_NO_DETACH, "CHANGE_MODE_G25_NO_DETACH" },
    { FF_LG_EXT_CMD_SET_RPM_LEDS,              "SET_RPM_LEDS" },
    { FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE,        "CHANGE_WHEEL_RANGE" },
};

const char * ff_lg_get_ext_cmd_name(unsigned char ext) {
    unsigned int i;
    for (i = 0; i < sizeof(ext_cmd_names) / sizeof(*ext_cmd_names); ++i) {
        if(ext_cmd_names[i].value == ext) {
            return ext_cmd_names[i].name;
        }
    }
    static char unknown[] = "UNKNOWN (255)";
    snprintf(unknown, sizeof(unknown), "UNKNOWN %hu", ext);
    return unknown;
}

static const char * ftype_names [] = {
    [FF_LG_FTYPE_CONSTANT]                           = "CONSTANT",
    [FF_LG_FTYPE_SPRING]                             = "SPRING",
    [FF_LG_FTYPE_DAMPER]                             = "DAMPER",
    [FF_LG_FTYPE_AUTO_CENTER_SPRING]                 = "AUTO_CENTER_SPRING",
    [FF_LG_FTYPE_SAWTOOTH_UP]                        = "SAWTOOTH_UP",
    [FF_LG_FTYPE_SAWTOOTH_DOWN]                      = "SAWTOOTH_DOWN",
    [FF_LG_FTYPE_TRAPEZOID]                          = "TRAPEZOID",
    [FF_LG_FTYPE_RECTANGLE]                          = "RECTANGLE",
    [FF_LG_FTYPE_VARIABLE]                           = "VARIABLE",
    [FF_LG_FTYPE_RAMP]                               = "RAMP",
    [FF_LG_FTYPE_SQUARE_WAVE]                        = "SQUARE_WAVE",
    [FF_LG_FTYPE_HIGH_RESOLUTION_SPRING]             = "HIGH_RESOLUTION_SPRING",
    [FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER]             = "HIGH_RESOLUTION_DAMPER",
    [FF_LG_FTYPE_HIGH_RESOLUTION_AUTO_CENTER_SPRING] = "HIGH_RESOLUTION_AUTO_CENTER_SPRING",
    [FF_LG_FTYPE_FRICTION]                           = "FRICTION",
};

const char * ff_lg_get_ftype_name(unsigned char ftype) {
    if (ftype < sizeof(ftype_names) / sizeof(*ftype_names)) {
        return ftype_names[ftype];
    } else {
        return "UNKNOWN";
    }
}

static struct {
    unsigned short product;
    unsigned short range;
} wheel_ranges[] = {
        { USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, 200 },
        { USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE,    200 },
        { USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL,       270 },
        { USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2,      240 },
};

/*
 * Get the wheel range. 0 means that the range can be changed.
 */
unsigned short ff_lg_get_wheel_range(unsigned short pid) {

    unsigned int i;
    for (i = 0; i < sizeof(wheel_ranges) / sizeof(*wheel_ranges); ++i) {
        if (wheel_ranges[i].product == pid) {
            return wheel_ranges[i].range;
        }
    }
    return 0;
}

uint8_t ff_lg_get_caps(uint16_t pid) {

    uint8_t caps = 0;

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
        break;
    default:
        caps |= FF_LG_CAPS_HIGH_RES_COEF;
        break;
    }

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
    case USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE:
        caps |= FF_LG_CAPS_OLD_LOW_RES_COEF;
        break;
    default:
        break;
    }

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
    case USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE:
    case USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL:
    case USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2:
        // older than Driving Force Pro
        break;
    default:
        caps |= FF_LG_CAPS_HIGH_RES_DEADBAND;
        break;
    }

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_DFP_WHEEL:
        caps |= FF_LG_CAPS_DAMPER_CLIP;
        break;
    default:
        break;
    }

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_G27_WHEEL:
    case USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL:
        caps |= FF_LG_CAPS_LEDS;
        break;
    default:
        break;
    }

    if (ff_lg_get_wheel_range(pid) == 0) {
        if (pid == USB_PRODUCT_ID_LOGITECH_DFP_WHEEL) {
            caps |= FF_LG_CAPS_RANGE_200_900;
        } else {
            caps |= FF_LG_CAPS_RANGE;
        }
    }

    return caps;
}

typedef struct {
    unsigned char num;
    unsigned char den;
} s_coef;

/*
 * \brief Get the spring or damper force coefficient, normalized to [0..1].
 *
 * \param caps the capabilities of the wheel (bitfield of FF_LG_CAPS)
 * \param k    the constant selector
 *
 * \return the force coefficient
 */
static s_coef ff_lg_get_force_coefficient(uint8_t caps, unsigned char k) {

    s_coef coef;

    if (caps & FF_LG_CAPS_HIGH_RES_COEF) {
        coef.num = k;
        coef.den = 0x0F;
    } else {
        if (caps & FF_LG_CAPS_OLD_LOW_RES_COEF) {
            static const s_coef old_coefs[] = { { 1, 16 }, { 1, 8 }, { 3, 16 }, { 1, 4 }, { 3, 8 }, { 3, 4 }, { 2, 4 }, { 4, 4 } };
            coef = old_coefs[k];
        } else {
            static const s_coef coefs[] = { { 1, 16 }, { 1, 8 }, { 3, 16 }, { 1, 4 }, { 3, 8 }, { 2, 4 }, { 3, 4 }, { 4, 4 } };
            coef = coefs[k];
        }
    }
    return coef;
}

static int16_t ff_lg_get_condition_coef(uint8_t caps, unsigned char k, unsigned char s) {

    s_coef coef = ff_lg_get_force_coefficient(caps, k);
    int value = (s ? -SHRT_MAX : SHRT_MAX) * coef.num / coef.den;
    return value;
}

static uint16_t ff_lg_get_spring_deadband(uint8_t caps, unsigned char d, unsigned char dL) {

    uint16_t deadband;
    if (caps & FF_LG_CAPS_HIGH_RES_DEADBAND) {
        deadband = ((d << 3) | dL) * USHRT_MAX / 0x7FF;
    } else {
        deadband = d * USHRT_MAX / UCHAR_MAX;
    }
    return deadband;
}

static uint16_t ff_lg_get_damper_clip(uint8_t caps, unsigned char c) {

    uint16_t clip;
    if (caps & FF_LG_CAPS_DAMPER_CLIP) {
        clip = c * USHRT_MAX / UCHAR_MAX;
    } else {
        clip = USHRT_MAX;
    }
    return clip;
}

static void dump(const unsigned char* packet, unsigned char length) {
    int i;
    for (i = 0; i < length; ++i) {
        printf("0x%02x ", packet[i]);
    }
}

void ff_lg_decode_extended(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    dprintf("%s %s", ff_lg_get_cmd_name(data[0]), ff_lg_get_ext_cmd_name(data[1]));

    switch(data[1]) {
    case FF_LG_EXT_CMD_CHANGE_MODE_DFP:
    case FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES:
    case FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES:
    case FF_LG_EXT_CMD_CHANGE_MODE_G25:
    case FF_LG_EXT_CMD_CHANGE_MODE_G25_NO_DETACH:
      break;
    case FF_LG_EXT_CMD_CHANGE_MODE:
    {
      const char * mode = NULL;
      switch(data[2]) {
      case 0x00:
        mode = "Logitech Driving Force EX";
        break;
      case 0x01:
        mode = "Logitech Driving Force Pro";
        break;
      case 0x02:
        mode = "Logitech G25 Racing Wheel";
        break;
      case 0x03:
        mode = "Logitech Driving Force GT";
        break;
      case 0x04:
        mode = "Logitech G27 Racing Wheel";
        break;
      }
      if(mode == NULL) {
          dprintf(" - unknown mode (0x%02x)", data[2]);
      }
      else {
          dprintf(" - %s", mode);
      }
      dprintf(" - %s", data[3] ? "DETACH" : "NO DETACH");
    }
      break;
    case FF_LG_EXT_CMD_REVERT_IDENTITY:
      dprintf(" - %s", data[2] ? "REVERT" : "DO NOT REVERT");
      break;
    case FF_LG_EXT_CMD_SET_RPM_LEDS:
      dprintf(" - 0x%02x", data[2]);
      break;
    case FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE:
      dprintf(" - %hu", (data[3] << 8) | data[2]);
      break;
    default:
      dprintf(" - ");
      dump(data + 2, FF_LG_OUTPUT_REPORT_SIZE - 2);
      break;
    }
    dprintf("\n");
}

void ff_lg_decode_command(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    dprintf("%s ", ff_lg_get_cmd_name(data[0]));
    const char * slots = ff_lg_get_slot_names(data[0]);
    if (*slots != '\0') {
        dprintf("- %s", slots);
    }

    switch(data[0] & FF_LG_CMD_MASK) {
    case FF_LG_CMD_PLAY:
    case FF_LG_CMD_STOP:
        break;
    case FF_LG_CMD_DOWNLOAD:
    case FF_LG_CMD_DOWNLOAD_AND_PLAY:
    case FF_LG_CMD_REFRESH_FORCE:
        dprintf(" - %s", ff_lg_get_ftype_name(data[1]));
        dprintf(" - ");
        dump(data + 2, FF_LG_OUTPUT_REPORT_SIZE - 2);
        break;
    case FF_LG_CMD_DEFAULT_SPRING_ON:
    case FF_LG_CMD_DEFAULT_SPRING_OFF:
    case FF_LG_CMD_NORMAL_MODE:
    case FF_LG_CMD_RAW_MODE:
        break;
    case FF_LG_CMD_SET_LED:
        dprintf(" - 0x%02x", data[1]);
        break;
    case FF_LG_CMD_SET_WATCHDOG:
        dprintf(" - 0x%02x", data[1]);
        break;
    case FF_LG_CMD_FIXED_TIME_LOOP:
        dprintf(" - %s", data[1] ? "ON" : "OFF");
        break;
    case FF_LG_CMD_SET_DEFAULT_SPRING:
        dprintf(" - ");
        dump(data + 1, FF_LG_OUTPUT_REPORT_SIZE - 1);
        break;
    case FF_LG_CMD_SET_DEAD_BAND:
        dprintf(" - %s", data[1] ? "ON" : "OFF");
        break;
    }
    dprintf("\n");
}

int ff_lg_convert_force(uint8_t caps, uint8_t slot_index, const s_ff_lg_command * force, uint8_t playing, s_haptic_core_data * to) {

    int ret = 0;

    memset(to, 0x00, sizeof(*to));

    switch (force->force_type) {
    case FF_LG_FTYPE_CONSTANT:
        to->type = E_DATA_TYPE_CONSTANT;
        if (playing) {
            to->playing = 1;
            to->constant.level = ff_lg_u8_to_s16(FF_LG_CONSTANT_LEVEL(force, slot_index));
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_VARIABLE:
        to->type = E_DATA_TYPE_CONSTANT;
        if (playing) {
            static int warned = 0;
            if (slot_index == 0) {
                if (FF_LG_VARIABLE_T1(force) && FF_LG_VARIABLE_S1(force)) {
                    if (warned == 0) {
                        gwarn("variable force cannot be converted to constant force (l1=%hu, t1=%hu, s1=%hu, d1=%hu\n",
                            FF_LG_VARIABLE_L1(force), FF_LG_VARIABLE_T1(force), FF_LG_VARIABLE_S1(force), FF_LG_VARIABLE_D1(force));
                        warned = 1;
                    }
                } else {
                    to->playing = 1;
                    to->constant.level = ff_lg_u8_to_s16(FF_LG_VARIABLE_L1(force));
                }
            } else if (slot_index == 2) {
                if (FF_LG_VARIABLE_T2(force) && FF_LG_VARIABLE_S2(force)) {
                    if (warned == 0) {
                        gwarn("variable force cannot be converted to constant force (l2=%hu, t2=%hu, s2=%hu, d2=%hu\n",
                            FF_LG_VARIABLE_L2(force), FF_LG_VARIABLE_T2(force), FF_LG_VARIABLE_S2(force), FF_LG_VARIABLE_D2(force));
                        warned = 1;
                    }
                } else {
                    to->playing = 1;
                    to->constant.level = ff_lg_u8_to_s16(FF_LG_VARIABLE_L2(force));
                }
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_SPRING:
        to->type = E_DATA_TYPE_SPRING;
        if (playing) {
            to->playing = 1;
            to->spring.saturation.left = ff_lg_u8_to_u16(FF_LG_SPRING_CLIP(force));
            to->spring.saturation.right = ff_lg_u8_to_u16(FF_LG_SPRING_CLIP(force));
            to->spring.coefficient.left =
                ff_lg_get_condition_coef(caps & FF_LG_CAPS_OLD_LOW_RES_COEF, FF_LG_SPRING_K1(force), FF_LG_SPRING_S1(force));
            to->spring.coefficient.right =
                ff_lg_get_condition_coef(caps & FF_LG_CAPS_OLD_LOW_RES_COEF, FF_LG_SPRING_K2(force), FF_LG_SPRING_S2(force));
            to->spring.center = ff_lg_u8_to_s16((FF_LG_SPRING_D1(force) + FF_LG_SPRING_D2(force)) / 2);
            to->spring.deadband = ff_lg_u8_to_u16(FF_LG_SPRING_D2(force) - FF_LG_SPRING_D1(force));
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_DAMPER:
        to->type = E_DATA_TYPE_DAMPER;
        if (playing) {
            to->playing = 1;
            to->damper.saturation.left = USHRT_MAX;
            to->damper.saturation.right = USHRT_MAX;
            to->damper.coefficient.left =
                ff_lg_get_condition_coef(caps & FF_LG_CAPS_OLD_LOW_RES_COEF, FF_LG_DAMPER_K1(force), FF_LG_DAMPER_S1(force));
            to->damper.coefficient.right =
                ff_lg_get_condition_coef(caps & FF_LG_CAPS_OLD_LOW_RES_COEF, FF_LG_DAMPER_K2(force), FF_LG_DAMPER_S2(force));
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_SPRING:
        to->type = E_DATA_TYPE_SPRING;
        if (playing) {
            to->playing = 1;
            to->spring.saturation.left = ff_lg_u8_to_u16(FF_LG_HIGHRES_SPRING_CLIP(force));
            to->spring.saturation.right = ff_lg_u8_to_u16(FF_LG_HIGHRES_SPRING_CLIP(force));
            to->spring.coefficient.left =
                ff_lg_get_condition_coef(FF_LG_CAPS_HIGH_RES_COEF, FF_LG_HIGHRES_SPRING_K1(force), FF_LG_HIGHRES_SPRING_S1(force));
            to->spring.coefficient.right =
                ff_lg_get_condition_coef(FF_LG_CAPS_HIGH_RES_COEF, FF_LG_HIGHRES_SPRING_K2(force), FF_LG_HIGHRES_SPRING_S2(force));
            uint16_t d2 = ff_lg_get_spring_deadband(caps, FF_LG_HIGHRES_SPRING_D2(force), FF_LG_HIGHRES_SPRING_D2L(force));
            uint16_t d1 = ff_lg_get_spring_deadband(caps, FF_LG_HIGHRES_SPRING_D1(force), FF_LG_HIGHRES_SPRING_D1L(force));
            to->spring.center = ff_lg_u16_to_s16((d1 + d2) / 2);
            to->spring.deadband = d2 - d1;
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER:
        to->type = E_DATA_TYPE_DAMPER;
        if (playing) {
            to->playing = 1;
            to->damper.saturation.left = ff_lg_get_damper_clip(caps, FF_LG_HIGHRES_DAMPER_CLIP(force));
            to->damper.saturation.right = ff_lg_get_damper_clip(caps, FF_LG_HIGHRES_DAMPER_CLIP(force));
            to->damper.coefficient.left =
                ff_lg_get_condition_coef(FF_LG_CAPS_HIGH_RES_COEF, FF_LG_HIGHRES_DAMPER_K1(force), FF_LG_HIGHRES_DAMPER_S1(force));
            to->damper.coefficient.right =
                ff_lg_get_condition_coef(FF_LG_CAPS_HIGH_RES_COEF, FF_LG_HIGHRES_DAMPER_K2(force), FF_LG_HIGHRES_DAMPER_S2(force));
            to->damper.center = 0;
            to->damper.deadband = 0;
        }
        ret = 1;
        break;
    default:
        //TODO MLA: other force types
        {
            static int warned[0x0F] = {};
            if (force->force_type < sizeof(warned) / sizeof(*warned) && warned[force->force_type] == 0) {
                gwarn("unsupported force type: %s\n", ff_lg_get_ftype_name(force->force_type));
                fflush(stdout);
                warned[force->force_type] = 1;
            }
        }
        break;
    }

    return ret;
}

int ff_lg_convert_extended(const s_ff_lg_command * ext, s_haptic_core_data * to) {

    int ret = 0;

    memset(to, 0x00, sizeof(*to));

    switch (ext->cmd_param) {
    case FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES:
        to->type = E_DATA_TYPE_RANGE;
        to->range.value = 200;
        ret = 1;
        break;
    case FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES:
        to->type = E_DATA_TYPE_RANGE;
        to->range.value = 900;
        ret = 1;
        break;
    case FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE:
        to->type = E_DATA_TYPE_RANGE;
        to->range.value = (ext->parameters[1] << 8) | ext->parameters[0];
        ret = 1;
        break;
    case FF_LG_EXT_CMD_SET_RPM_LEDS:
        to->type = E_DATA_TYPE_LEDS;
        to->leds.value = ext->parameters[0];
        ret = 1;
        break;
    default:
        break;
    }

    return ret;
}

static inline unsigned char convert_coef_lr2lr(unsigned char k) {

    static const unsigned char map[] = { 0, 1, 2, 3, 4, 6, 5, 7 };
    return map[k];
}

#define GET_CLIP(SL, SR) ((SL > SR ? SL : SR) * 255 / USHRT_MAX)

#define CLAMP(MIN,VALUE,MAX) (((VALUE) < MIN) ? (MIN) : (((VALUE) > MAX) ? (MAX) : (VALUE)))

void ff_lg_convert_slot(const s_haptic_core_data * from, int slot, s_ff_lg_report * to, uint8_t caps) {

    memset(to, 0x00, sizeof(*to));

    uint8_t * cmd = to->data + 1;
    s_ff_lg_command * command = (s_ff_lg_command *)(to->data + 2);

    switch (from->type) {
    case E_DATA_TYPE_CONSTANT:
    case E_DATA_TYPE_SPRING:
    case E_DATA_TYPE_DAMPER:
        if (from->playing) {
            *cmd = FF_LG_CMD_DOWNLOAD_AND_PLAY;
        } else {
            *cmd = FF_LG_CMD_STOP;
        }
        *cmd |= ((1 << slot) << FF_LG_FSLOTS_OFFSET);
        break;
    case E_DATA_TYPE_LEDS:
    case E_DATA_TYPE_RANGE:
        break;
    default:
        break;
    }

    if (*cmd == FF_LG_CMD_STOP) {
        return;
    }

    switch (from->type) {
    case E_DATA_TYPE_CONSTANT:
        command->force_type = FF_LG_FTYPE_VARIABLE;
        command->parameters[0] = ff_lg_s16_to_u8(from->constant.level);
        command->parameters[1] = 0x80;
        break;
    case E_DATA_TYPE_SPRING:
    {
        uint16_t d1;
        uint16_t d2;
        uint8_t k1;
        uint8_t k2;
        uint8_t s1 = 0x00;
        int16_t left = from->spring.coefficient.left;
        int16_t right = from->spring.coefficient.right;
        if (left < 0) {
            s1 = 0x01;
            left *= -1;
        }
        uint8_t s2 = 0x00;
        if (right < 0) {
            s2 = 0x01;
            right *= -1;
        }
        uint8_t clip = GET_CLIP(from->spring.saturation.left, from->spring.saturation.right);
        uint16_t d = (from->spring.deadband / 2);
        if (caps & FF_LG_CAPS_HIGH_RES_COEF) {
            command->force_type = FF_LG_FTYPE_HIGH_RESOLUTION_SPRING;
            int32_t v = from->spring.center - d;
            d1 = ff_lg_s16_to_u11(CLAMP(-SHRT_MAX, v, SHRT_MAX));
            v = from->spring.center + d;
            d2 = ff_lg_s16_to_u11(CLAMP(-SHRT_MAX, v, SHRT_MAX));
            k1 = left * 0x0f / SHRT_MAX;
            k2 = right * 0x0f / SHRT_MAX;
        } else {
            command->force_type = FF_LG_FTYPE_SPRING;
            int32_t v = from->spring.center - d;
            d1 = ff_lg_s16_to_u8(CLAMP(-SHRT_MAX, v, SHRT_MAX)) << 3;
            v = from->spring.center + d;
            d2 = ff_lg_s16_to_u8(CLAMP(-SHRT_MAX, v, SHRT_MAX)) << 3;
            k1 = left * 0x07 / SHRT_MAX;
            k2 = right * 0x07 / SHRT_MAX;
            if (caps & FF_LG_CAPS_OLD_LOW_RES_COEF) {
                k1 = convert_coef_lr2lr(k1);
                k2 = convert_coef_lr2lr(k2);
            }
        }
        command->parameters[0] = d1 >> 3;
        command->parameters[1] = d2 >> 3;
        command->parameters[2] = (k2 << 4) | k1;
        command->parameters[3] = ((d2 & 0x0f) << 5) | (s2 << 4) | ((d1 & 0x07) << 1) | s1;
        command->parameters[4] = clip;
        break;
    }
    case E_DATA_TYPE_DAMPER:
    {
        uint8_t k1;
        uint8_t s1 = 0x00;
        uint8_t k2;
        uint8_t s2 = 0x00;
        uint8_t clip = 0;
        int16_t left = from->spring.coefficient.left;
        int16_t right = from->spring.coefficient.right;
        if (left < 0) {
            s1 = 0x01;
            left *= -1;
        }
        if (right < 0) {
            s2 = 0x01;
            right *= -1;
        }
        if (caps & FF_LG_CAPS_HIGH_RES_COEF) {
            command->force_type = FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER;
            k1 = left * 0x0f / SHRT_MAX;
            k2 = right * 0x0f / SHRT_MAX;
            if (caps & FF_LG_CAPS_DAMPER_CLIP) {
                clip = GET_CLIP(from->damper.saturation.left, from->damper.saturation.right);
            } else {
                clip = 0xff;
            }
        } else {
            command->force_type = FF_LG_FTYPE_DAMPER;
            k1 = left * 0x07 / SHRT_MAX;
            k2 = right * 0x07 / SHRT_MAX;
            if (caps & FF_LG_CAPS_OLD_LOW_RES_COEF) {
                k1 = convert_coef_lr2lr(k1);
                k2 = convert_coef_lr2lr(k2);
            }
        }
        command->parameters[0] = k1;
        command->parameters[1] = s1;
        command->parameters[2] = k2;
        command->parameters[3] = s2;
        command->parameters[4] = clip;
        break;
    }
    case E_DATA_TYPE_LEDS:
        *cmd = FF_LG_CMD_EXTENDED_COMMAND;
        command->cmd_param = FF_LG_EXT_CMD_SET_RPM_LEDS;
        command->parameters[0] = from->leds.value;
        break;
    case E_DATA_TYPE_RANGE:
        if (caps & FF_LG_CAPS_RANGE_200_900) {
            *cmd = FF_LG_CMD_EXTENDED_COMMAND;
            unsigned short full_range;
            if (from->range.value > 200) {
                command->cmd_param = FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES;
                full_range = 900;
            } else {
                command->cmd_param = FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES;
                full_range = 200;
            }

            ginfo("wheel range adjusted to %hu degrees\n", full_range);
            if (from->range.value != full_range) {
                static int warn = 1;
                if (warn == 1) {
                    gwarn("Driving Force Pro currently only supports 200 and 900 degree ranges\n");
                    warn = 0;
                }
                // division by 2 is performed when computing high and low order bits
                /*unsigned short d1 = (full_range - range + 1) * 0x7FF / full_range;
                unsigned short d2 = 0xFFF - d1;
                uint8_t report2[FF_LG_OUTPUT_REPORT_SIZE] = {
                        FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                        FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                        d1 >> 4,
                        d2 >> 4,
                        0xff,
                        (d2 & 0x0e) << 4 | (d1 & 0x0e),
                        0xff
                };*/
            }
        } else {
            *cmd = FF_LG_CMD_EXTENDED_COMMAND;
            command->cmd_param = FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE;
            command->parameters[0] = from->range.value & 0xFF;
            command->parameters[1] = from->range.value >> 8;

            ginfo("wheel range adjusted to %hu degrees\n", from->range.value);
        }
        break;
    default:
        break;
    }
}
