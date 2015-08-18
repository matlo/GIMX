/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>

#include <ffb_logitech.h>

#define FSLOT_1 0x10
#define FSLOT_2 0x20
#define FSLOT_3 0x40
#define FSLOT_4 0x80

#define CMD_DOWNLOAD           0x00
#define CMD_DOWNLOAD_AND_PLAY  0x01
#define CMD_PLAY               0x02
#define CMD_STOP               0x03
#define CMD_DEFAULT_SPRING_ON  0x04
#define CMD_DEFAULT_SPRING_OFF 0x05
#define CMD_RESERVED_1         0x06
#define CMD_RESERVED_2         0x07
#define CMD_NORMAL_MODE        0x08
#define CMD_EXTENDED_COMMAND   0xF8
#define CMD_SET_LED            0x09
#define CMD_SET_WATCHDOG       0x0A
#define CMD_RAW_MODE           0x0B
#define CMD_REFRESH_FORCE      0x0C
#define CMD_FIXED_TIME_LOOP    0x0D
#define CMD_SET_DEFAULT_SPRING 0x0E
#define CMD_SET_DEAD_BAND      0x0F

#define FTYPE_CONSTANT                           0x00
#define FTYPE_SPRING                             0x01
#define FTYPE_DAMPER                             0x02
#define FTYPE_AUTO_CENTER_SPRING                 0x03
#define FTYPE_SAWTOOTH_UP                        0x04
#define FTYPE_SAWTOOTH_DOWN                      0x05
#define FTYPE_TRAPEZOID                          0x06
#define FTYPE_RECTANGLE                          0x07
#define FTYPE_VARIABLE                           0x08
#define FTYPE_RAMP                               0x09
#define FTYPE_SQUARE_WAVE                        0x0A
#define FTYPE_HIGH_RESOLUTION_SPRING             0x0B
#define FTYPE_HIGH_RESOLUTION_DAMPER             0x0C
#define FTYPE_HIGH_RESOLUTION_AUTO_CENTER_SPRING 0x0D
#define FTYPE_FRICTION                           0x0E

static unsigned char fslot_nbits [] = {
    [0b0000] = 0,
    [0b0001] = 1,
    [0b0010] = 1,
    [0b0011] = 2,
    [0b0100] = 1,
    [0b0101] = 2,
    [0b0110] = 2,
    [0b0111] = 3,
    [0b1000] = 1,
    [0b1001] = 2,
    [0b1010] = 2,
    [0b1011] = 3,
    [0b1100] = 2,
    [0b1101] = 3,
    [0b1110] = 3,
    [0b1111] = 4
};

static const char * cmd_names[] = {
    [CMD_DOWNLOAD]           = "DOWNLOAD",
    [CMD_DOWNLOAD_AND_PLAY]  = "DOWNLOAD_AND_PLAY",
    [CMD_PLAY]               = "PLAY",
    [CMD_STOP]               = "STOP",
    [CMD_DEFAULT_SPRING_ON]  = "DEFAULT_SPRING_ON",
    [CMD_DEFAULT_SPRING_OFF] = "DEFAULT_SPRING_OFF",
    [CMD_RESERVED_1]         = "RESERVED_1",
    [CMD_RESERVED_2]         = "RESERVED_2",
    [CMD_NORMAL_MODE]        = "NORMAL_MODE",
    [CMD_SET_LED]            = "SET_LED",
    [CMD_SET_WATCHDOG]       = "SET_WATCHDOG",
    [CMD_RAW_MODE]           = "RAW_MODE",
    [CMD_REFRESH_FORCE]      = "REFRESH_FORCE",
    [CMD_FIXED_TIME_LOOP]    = "FIXED_TIME_LOOP",
    [CMD_SET_DEFAULT_SPRING] = "SET_DEFAULT_SPRING",
    [CMD_SET_DEAD_BAND]      = "SET_DEAD_BAND",
};

const char * get_cmd_name(unsigned char header) {
    if (header == CMD_EXTENDED_COMMAND) {
        return "EXTENDED_COMMAND";
    } else {
        unsigned char cmd = header & 0x0f;
        if (cmd < sizeof(cmd_names) / sizeof(*cmd_names)) {
            return cmd_names[cmd];
        } else {
            return "UNKNOWN";
        }
    }
}

static const char * ftype_names [] = {
    [FTYPE_CONSTANT]                           = "CONSTANT",
    [FTYPE_SPRING]                             = "SPRING",
    [FTYPE_DAMPER]                             = "DAMPER",
    [FTYPE_AUTO_CENTER_SPRING]                 = "AUTO_CENTER_SPRING",
    [FTYPE_SAWTOOTH_UP]                        = "SAWTOOTH_UP",
    [FTYPE_SAWTOOTH_DOWN]                      = "SAWTOOTH_DOWN",
    [FTYPE_TRAPEZOID]                          = "TRAPEZOID",
    [FTYPE_RECTANGLE]                          = "RECTANGLE",
    [FTYPE_VARIABLE]                           = "VARIABLE",
    [FTYPE_RAMP]                               = "RAMP",
    [FTYPE_SQUARE_WAVE]                        = "SQUARE_WAVE",
    [FTYPE_HIGH_RESOLUTION_SPRING]             = "HIGH_RESOLUTION_SPRING",
    [FTYPE_HIGH_RESOLUTION_DAMPER]             = "HIGH_RESOLUTION_DAMPER",
    [FTYPE_HIGH_RESOLUTION_AUTO_CENTER_SPRING] = "HIGH_RESOLUTION_AUTO_CENTER_SPRING",
    [FTYPE_FRICTION]                           = "FRICTION",
};

const char * get_ftype_name(unsigned char ftype) {
    if (ftype < sizeof(ftype_names) / sizeof(*ftype_names)) {
        return ftype_names[ftype];
    } else {
        return "UNKNOWN";
    }
}

#warning TODO MLA: support multiple instances

static struct {
    unsigned char mask;
    unsigned char active;
    unsigned char send;
    unsigned char parameters[FFB_LOGITECH_OUTPUT_REPORT_SIZE - 1];
} forces[] = {
        { .mask = FSLOT_1 },
        { .mask = FSLOT_2 },
        { .mask = FSLOT_3 },
        { .mask = FSLOT_4 },
};

#define FORCES_NB (sizeof(forces) / sizeof(*forces))

static const int debug = 0;

#define dprintf(...) if(debug) printf(__VA_ARGS__)

static unsigned char slot_index[] = { [FSLOT_1 >> 4] = 0, [FSLOT_2 >> 4] = 1, [FSLOT_3 >> 4] = 2, [FSLOT_4 >> 4] = 3, };

static unsigned char fifo[FORCES_NB] = { };

static void fifo_push(unsigned char force) {
    int i;
    for (i = 0; i < FORCES_NB; ++i) {
        if (!fifo[i]) {
            fifo[i] = force; //add
            dprintf("push %02x\n", force);
            break;
        } else if (fifo[i] == force) {
            break; //already present
        }
    }
}

static unsigned char fifo_pop() {
    unsigned char force = fifo[0];
    if (force) {
        dprintf("pop %02x\n", force);
        memmove(fifo, fifo + 1, FORCES_NB - 1);
        fifo[FORCES_NB - 1] = 0;
    }
    return force;
}

static void fifo_remove(unsigned char force) {
    int i;
    for (i = 0; i < FORCES_NB; ++i) {
        if (!fifo[i]) {
            break;
        } else if (fifo[i] == force) {
            dprintf("remove %02x\n", force);
            memmove(fifo + i, fifo + i + 1, FORCES_NB - i - 1);
            fifo[FORCES_NB - i - 1] = 0;
            break;
        }
    }
}

void ffb_logitech_process_report(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]) {

    if(data[0] == CMD_EXTENDED_COMMAND) {
        dprintf("> %s %02x\n", get_cmd_name(data[0]), data[0]);
        return;
    }

    unsigned char slots = data[0] & 0xf0;
    unsigned char cmd = data[0] & 0x0f;

    dprintf("> %s %02x\n", get_cmd_name(data[0]), data[0]);

    int i;
    for (i = 0; i < FORCES_NB; ++i) {
        if (slots & forces[i].mask) {
            if (cmd == CMD_DOWNLOAD_AND_PLAY) {
                forces[i].send = 1;
                forces[i].active = 1;
                memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
            } else if (cmd == CMD_STOP) {
                forces[i].send = 1;
                forces[i].active = 0;
                memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
            } else {
                continue;
            }
            fifo_push(forces[i].mask);
        }
    }
}

#define STOP(mask,slot) \
    if (mask & slot) { \
        forces[slot_index[slot >> 4]].send = 0; \
        fifo_remove(slot); \
    }

static void stop_forces(unsigned char mask) {
    STOP(mask, FSLOT_1)
    STOP(mask, FSLOT_2)
    STOP(mask, FSLOT_3)
    STOP(mask, FSLOT_4)
}

int ffb_logitech_get_report(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]) {

    int i;

    unsigned char mask = 0;
    // look for forces to stop
    for (i = 0; i < FORCES_NB; ++i) {
        if (forces[i].send && !forces[i].active) {
            mask |= forces[i].mask;
        }
    }

    // if multiple forces have to be stopped, then stop them
    if (fslot_nbits[mask >> 4] > 1) {
        data[0] = mask | CMD_STOP;
        stop_forces(mask);
        dprintf("< %s %02x\n", get_cmd_name(data[0]), data[0]);
        return 1;
    }

    unsigned char force = fifo_pop();
    if (force) {
        unsigned char index = slot_index[force >> 4];
        if (forces[index].active) {
            data[0] = force | CMD_DOWNLOAD_AND_PLAY;
            memcpy(data + 1, forces[index].parameters, sizeof(forces[i].parameters));
        } else {
            stop_forces(force);
            data[0] = force | CMD_STOP;
        }
        dprintf("< %s %02x\n", get_cmd_name(data[0]), data[0]);
        fifo_remove(force);
        forces[index].send = 0;
        return 1;
    }

    return 0;
}

