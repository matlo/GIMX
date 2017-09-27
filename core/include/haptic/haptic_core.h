/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HAPTIC_CORE_H_
#define HAPTIC_CORE_H_

#include <stdint.h>
#include <stddef.h>

#define MAX_DATA_SIZE 64 // max size for a HID report

typedef struct {
    uint16_t weak;
    uint16_t strong;
} s_haptic_core_rumble;

typedef struct {
    int16_t level;
} s_haptic_core_constant;

typedef struct {
    struct {
      uint16_t left;
      uint16_t right;
    } saturation;
    struct {
      int16_t left;
      int16_t right;
    } coefficient;
    int16_t center;
    uint16_t deadband;
} s_haptic_core_condition;

typedef enum {
    E_DATA_TYPE_NONE,
    E_DATA_TYPE_RUMBLE,
    E_DATA_TYPE_CONSTANT,
    E_DATA_TYPE_SPRING,
    E_DATA_TYPE_DAMPER,
    E_DATA_TYPE_LEDS,
    E_DATA_TYPE_RANGE,
} e_haptic_core_data_type;

typedef struct {
    uint8_t value;
} s_haptic_core_leds;

typedef struct {
    uint16_t value;
} s_haptic_core_range;

typedef struct {
    e_haptic_core_data_type type;
    uint8_t playing;
    union {
        s_haptic_core_rumble rumble;
        s_haptic_core_constant constant;
        s_haptic_core_condition spring;
        s_haptic_core_condition damper;
        s_haptic_core_leds leds;
        s_haptic_core_range range;
    };
} s_haptic_core_data;

typedef struct {
    uint16_t vid;
    uint16_t pid;
} s_haptic_core_ids;

typedef struct {
    int invert;
    struct {
        int rumble;
        int constant;
        int spring;
        int damper;
    } gain;
} s_haptic_core_tweaks;

struct haptic_core;

struct haptic_core * haptic_core_init(s_haptic_core_ids source_ids, int source_joystick);
int haptic_core_clean(struct haptic_core * core);

void haptic_core_set_tweaks(struct haptic_core * core, const s_haptic_core_tweaks * tweaks);

void haptic_core_process_report(struct haptic_core * core, size_t size, const unsigned char * data);
void haptic_core_update(struct haptic_core * core);

#endif /* HAPTIC_CORE_H_ */
