/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "sixaxis.h"
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <arpa/inet.h> /* for htons */
#else
#include <winsock2.h> /* for htons */
#endif

inline int clamp(int min, int val, int max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void sixaxis_init(struct sixaxis_state *state)
{
    memset(state, 0, sizeof(struct sixaxis_state));

    state->sys.reporting_enabled = 0;
    state->sys.feature_ef_byte_6 = 0xb0;

    state->user.X = 128;
    state->user.Y = 128;
    state->user.Z = 128;
    state->user.Rz = 128;
    state->user.acc_x[0] = 512 >> 8;
    state->user.acc_x[1] = 512 & 0xFF;
    state->user.acc_y[0] = 512 >> 8;
    state->user.acc_y[1] = 512 & 0xFF;
    state->user.acc_z[0] = 400 >> 8;
    state->user.acc_z[1] = 400 & 0xFF;
    state->user.gyro[0] = 512 >> 8;
    state->user.gyro[1] = 512 & 0xFF;
}

int sixaxis_periodic_report(struct sixaxis_state *state)
{
    int ret;
    ret = state->sys.reporting_enabled;
    return ret;
}

/* Main input report from Sixaxis -- assemble it */
int assemble_input_01(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    if (maxlen < sizeof(s_report_ds3)-1) return -1;

    memcpy(buf, ((uint8_t*)&state->user)+1, sizeof(s_report_ds3)-1);

    buf[30] = 0x16;//bluetooth+rumble off (0x14 = on)

    /* Unknown, some values fluctuate? */
    buf[35] = 0x33;
    buf[36] = 0xfa;
    buf[37] = 0x77;
    buf[38] = 0x01;
    buf[39] = 0xc0;//rumble off (0x40 = on)

    return sizeof(s_report_ds3)-1;
}

/* Main input report from Sixaxis -- decode it */
int process_input_01(const uint8_t *buf, int len, struct sixaxis_state *state)
{
    if (len < sizeof(s_report_ds3)) return -1;

    memcpy(&state->user, buf, sizeof(s_report_ds3));

    return 0;
}

int sixaxis_number = -1;

/* Unknown */
int assemble_feature_01(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    uint8_t data[] = {
        0x01, 0x03, 0x00, 0x04, 0x0c, 0x01, 0x02, 0x18,
        0x18, 0x18, 0x18, 0x09, 0x0a, 0x10, 0x11, 0x12,
        0x13, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x02,
        0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x04, 0x04,
        0x04, 0x04, 0x00, 0x00, 0x02, 0x01, 0x02, 0x00,
        0x64, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    data[3] += sixaxis_number;
    int len = sizeof(data);
    if (len > maxlen) return -1;
    memcpy(buf, data, len);

    return len;
}

/* Unknown */
int assemble_feature_ef(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    const uint8_t data[] = {
        0xef, 0x04, 0x00, 0x05, 0x03, 0x01, 0xb0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, /* acc_x center & radius? */
        0x02, 0x00, 0x03, 0xff, /* acc_y center & radius? */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04
    };
    int len = sizeof(data);
    if (len > maxlen) return -1;
    memcpy(buf, data, len);

    /* Byte 6 must match the byte set by the PS3 */
    buf[6] = state->sys.feature_ef_byte_6;

    return len;
}

/* Unknown */
int assemble_feature_f2(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    uint8_t data[] = {
        0xff, 0xff, 0x00,
        0x00, 0x1e, 0x3d, 0x24, 0x97, 0xde, /* device bdaddr */
        0x00, 0x03, 0x50, 0x89, 0xc0, 0x01, 0x8a,
        0x09
    };
#ifndef WIN32 //remove compilation warnings
    sscanf(state->bdaddr_src, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", data+3, data+4, data+5, data+6, data+7, data+8);
#endif
    int len = sizeof(data);
    if (len > maxlen) return -1;
    memcpy(buf, data, len);

    return len;
}

/* Unknown */
int assemble_feature_f7(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    const uint8_t data[] = {
        0x00, 0x02, 0xe4, 0x02, 0xa9, 0x01, 0x05, 0xff, 0x14, 0x23, 0x00
    };
    int len = sizeof(data);
    if (len > maxlen) return -1;
    memcpy(buf, data, len);

    return len;
}

/* Unknown */
int assemble_feature_f8(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    const uint8_t data[] = {
        0x01, 0x00, 0x00, 0x00
    };
    int len = sizeof(data);
    if (len > maxlen) return -1;
    memcpy(buf, data, len);

    return len;
}

/* Main output report from PS3 including rumble and LEDs */
int process_output_01(const uint8_t *buf, int len, struct sixaxis_state *state)
{
    int i;

    if (len < 48)
        return -1;

    /* Decode LEDs.  This ignores a lot of details of the LED bits */
    for (i = 0; i < 5; i++) {
        if (buf[9] & (1 << i)) {
            if (buf[13 + 5 * i] == 0)
                state->sys.led[i] = LED_ON;
            else
                state->sys.led[i] = LED_FLASH;
        } else {
            state->sys.led[i] = LED_OFF;
        }
    }

    /* Decode rumble.  Again, ignores some details */
    state->sys.rumble[0] = buf[1] ? buf[2] : 0;
    state->sys.rumble[1] = buf[3] ? buf[4] : 0;

    return 0;
}

/* Unknown */
int process_feature_ef(const uint8_t *buf, int len, struct sixaxis_state *state)
{
    if (len < 7)
        return -1;
    /* Need to remember byte 6 for assemble_feature_ef */
    state->sys.feature_ef_byte_6 = buf[6];
    return 0;
}

/* Enable reporting */
int process_feature_f4(const uint8_t *buf, int len, struct sixaxis_state *state)
{
    /* Enable event reporting */
    if(buf[1] == 0x08)
    {
      state->sys.shutdown = 1;
    }
    else
    {
      state->sys.reporting_enabled = 1;
    }
    return 0;
}

struct sixaxis_assemble_t sixaxis_assemble[] = {
    { HID_TYPE_INPUT, 0x01, assemble_input_01 },
    { HID_TYPE_FEATURE, 0x01, assemble_feature_01 },
    { HID_TYPE_FEATURE, 0xef, assemble_feature_ef },
    { HID_TYPE_FEATURE, 0xf2, assemble_feature_f2 },
    { HID_TYPE_FEATURE, 0xf7, assemble_feature_f7 },
    { HID_TYPE_FEATURE, 0xf8, assemble_feature_f8 },
    { 0 }
};

struct sixaxis_process_t sixaxis_process[] = {
    { HID_TYPE_INPUT, 0x01, process_input_01 },
    { HID_TYPE_OUTPUT, 0x01, process_output_01 },
    { HID_TYPE_FEATURE, 0xef, process_feature_ef },
    { HID_TYPE_FEATURE, 0xf4, process_feature_f4 },
    { 0 }
};
