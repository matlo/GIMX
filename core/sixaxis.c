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

int clamp(int min, int val, int max)
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
}

int sixaxis_periodic_report(struct sixaxis_state *state)
{
    int ret;
    ret = state->sys.reporting_enabled;
    return ret;
}

const int digital_order[17] = {
    sa_select, sa_l3, sa_r3, sa_start,
    sa_up, sa_right, sa_down, sa_left,
    sa_l2, sa_r2, sa_l1, sa_r1,
    sa_triangle, sa_circle, sa_cross, sa_square,
    sa_ps };

const int analog_order[12] = {
    sa_up, sa_right, sa_down, sa_left,
    sa_l2, sa_r2, sa_l1, sa_r1,
    sa_triangle, sa_circle, sa_cross, sa_square };

/* Main input report from Sixaxis -- assemble it */
int assemble_input_01(uint8_t *buf, int maxlen, struct sixaxis_state *state)
{
    struct sixaxis_state_user *u = &state->user;
    int i;

    if (maxlen < 48) return -1;
    memset(buf, 0, 48);

    /* Digital button state */
    for (i = 0; i < 17; i++) {
        int byte = 1 + (i / 8);
        int offset = i % 8;
        if (u->axis[digital_order[i]])
            buf[byte] |= (1 << offset);
    }

    /* Axes */
    buf[5] = clamp(0, u->axis[sa_lstick_x] + 128, 255);
    buf[6] = clamp(0, u->axis[sa_lstick_y] + 128, 255);
    buf[7] = clamp(0, u->axis[sa_rstick_x] + 128, 255);
    buf[8] = clamp(0, u->axis[sa_rstick_y] + 128, 255);

    /* Analog button state */
    for (i = 0; i < 12; i++)
        buf[13 + i] = u->axis[analog_order[i]];

    /* Charging status? */
    buf[28] = 0x03;

    /* Power rating? */
    buf[29] = 0x05;

    /* Bluetooth and rumble status? */
    buf[30] = 0x16;

    /* Unknown, some values fluctuate? */
    buf[31] = 0x00;
    buf[32] = 0x00;
    buf[33] = 0x00;
    buf[34] = 0x00;
    buf[35] = 0x33;
    buf[36] = 0x02;
    buf[37] = 0x77;
    buf[38] = 0x01;
    buf[39] = 0x9e;

    /* Accelerometers */
    *(uint16_t *)&buf[40] = htons(clamp(0, u->axis[sa_acc_x] + 512, 1023));
    *(uint16_t *)&buf[42] = htons(clamp(0, u->axis[sa_acc_y] + 512, 1023));
    *(uint16_t *)&buf[44] = htons(clamp(0, u->axis[sa_acc_z] + 512, 1023));
    *(uint16_t *)&buf[46] = htons(clamp(0, u->axis[sa_gyro] + 512, 1023));

    return 48;
}

/* Main input report from Sixaxis -- decode it */
int process_input_01(const uint8_t *buf, int len, struct sixaxis_state *state)
{
    struct sixaxis_state_user *u = &state->user;
    int i;

    if (len < 48) return -1;
    /* Digital button state */
    for (i = 0; i < 17; i++) {
        int byte = 1 + (i / 8);
        int offset = i % 8;
        if (buf[byte] & (1 << offset))
            u->axis[digital_order[i]] = 255;
        else
            u->axis[digital_order[i]] = 0;
    }

    /* Axes */
    u->axis[sa_lstick_x] = buf[5] - 128;
    u->axis[sa_lstick_y] = buf[6] - 128;
    u->axis[sa_rstick_x] = buf[7] - 128;
    u->axis[sa_rstick_y] = buf[8] - 128;

    /* Analog button state */
    for (i = 0; i < 12; i++)
        u->axis[analog_order[i]] = buf[13 + i];

    /* Accelerometers */
    u->axis[sa_acc_x] = ntohs(*(uint16_t *)&buf[40]) - 512;
    u->axis[sa_acc_y] = ntohs(*(uint16_t *)&buf[42]) - 512;
    u->axis[sa_acc_z] = ntohs(*(uint16_t *)&buf[44]) - 512;
    u->axis[sa_gyro] = ntohs(*(uint16_t *)&buf[46]) - 512;

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

#define MAX_NAME_LENGTH 32

static const char *axis_name[SA_MAX] = {
    [sa_lstick_x] = "lstick x",
    [sa_lstick_y] = "lstick y",
    [sa_rstick_x] = "rstick x",
    [sa_rstick_y] = "rstick y",
    [sa_acc_x] = "acc x",
    [sa_acc_y] = "acc y",
    [sa_acc_z] = "acc z",
    [sa_gyro] = "gyro",
    [sa_select] = "select",
    [sa_start] = "start",
    [sa_ps] = "PS",
    [sa_up] = "up",
    [sa_right] = "right",
    [sa_down] = "down",
    [sa_left] = "left",
    [sa_triangle] = "triangle",
    [sa_circle] = "circle",
    [sa_cross] = "cross",
    [sa_square] = "square",
    [sa_l1] = "l1",
    [sa_r1] = "r1",
    [sa_l2] = "l2",
    [sa_r2] = "r2",
    [sa_l3] = "l3",
    [sa_r3] = "r3"
};

const char* get_axis_name(int index)
{
  return axis_name[index];
}

int get_button_index_from_name(const char* name)
{
  int i;
  for(i=0; i<SA_MAX; ++i)
  {
    if(!strncmp(axis_name[i], name, MAX_NAME_LENGTH))
    {
      return i;
    }
  }
  return -1;
}

typedef struct {
    char name[MAX_NAME_LENGTH];
    s_axis_index aindex;
} s_axis_name_index;

s_axis_name_index axis_name_index[] =
{
    {.name="rstick x",     {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick y",     {.value=-1,  .index=sa_rstick_y}},
    {.name="lstick x",     {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick y",     {.value=-1,  .index=sa_lstick_y}},
    {.name="rstick left",  {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick right", {.value= 1,  .index=sa_rstick_x}},
    {.name="rstick up",    {.value=-1,  .index=sa_rstick_y}},
    {.name="rstick down",  {.value= 1,  .index=sa_rstick_y}},
    {.name="lstick left",  {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick right", {.value= 1,  .index=sa_lstick_x}},
    {.name="lstick up",    {.value=-1,  .index=sa_lstick_y}},
    {.name="lstick down",  {.value= 1,  .index=sa_lstick_y}},
    {.name="acc x",        {.value=-1,  .index=sa_acc_x}},
    {.name="acc y",        {.value=-1,  .index=sa_acc_y}},
    {.name="acc z",        {.value=-1,  .index=sa_acc_z}},
    {.name="gyro",         {.value=-1,  .index=sa_gyro}},
    {.name="acc x -",      {.value=-1,  .index=sa_acc_x}},
    {.name="acc y -",      {.value=-1,  .index=sa_acc_y}},
    {.name="acc z -",      {.value=-1,  .index=sa_acc_z}},
    {.name="gyro -",       {.value=-1,  .index=sa_gyro}},
    {.name="acc x +",      {.value= 1,  .index=sa_acc_x}},
    {.name="acc y +",      {.value= 1,  .index=sa_acc_y}},
    {.name="acc z +",      {.value= 1,  .index=sa_acc_z}},
    {.name="gyro +",       {.value= 1,  .index=sa_gyro}},
    {.name="up",           {.value= 0,  .index=sa_up}},
    {.name="down",         {.value= 0,  .index=sa_down}},
    {.name="right",        {.value= 0,  .index=sa_right}},
    {.name="left",         {.value= 0,  .index=sa_left}},
    {.name="r1",           {.value= 0,  .index=sa_r1}},
    {.name="r2",           {.value= 0,  .index=sa_r2}},
    {.name="l1",           {.value= 0,  .index=sa_l1}},
    {.name="l2",           {.value= 0,  .index=sa_l2}},
    {.name="circle",       {.value= 0,  .index=sa_circle}},
    {.name="square",       {.value= 0,  .index=sa_square}},
    {.name="cross",        {.value= 0,  .index=sa_cross}},
    {.name="triangle",     {.value= 0,  .index=sa_triangle}},
};

s_axis_index get_axis_index_from_name(const char* name)
{
  int i;
  s_axis_index none = {-1, -1};
  for(i=0; i<sizeof(axis_name_index)/sizeof(s_axis_name_index); ++i)
  {
    if(!strncmp(axis_name_index[i].name, name, MAX_NAME_LENGTH))
    {
      return axis_name_index[i].aindex;
    }
  }
  return none;
}

