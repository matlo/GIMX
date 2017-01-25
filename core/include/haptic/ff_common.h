/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FF_COMMON_H_
#define FF_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <haptic/ff_lg.h>
#include <gimx.h>

#define dprintf(...) if(gimx_params.debug.ff_common) printf(__VA_ARGS__)

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define FIFO_SIZE 16

typedef struct {
    unsigned char cmd;
    unsigned char ext;
} s_cmd;

static inline int compare_cmd(s_cmd cmd1, s_cmd cmd2) {
    return cmd1.cmd == cmd2.cmd && (cmd1.cmd != FF_LG_CMD_EXTENDED_COMMAND || cmd1.ext == cmd2.ext);
}

static inline void fifo_push(s_cmd fifo[FIFO_SIZE], s_cmd cmd, int replace) {
    int i;
    for (i = 0; i < FIFO_SIZE; ++i) {
        if (!fifo[i].cmd) {
            fifo[i] = cmd; //add
            dprintf("push:");
            break;
        } else if (replace && compare_cmd(fifo[i], cmd)) {
            dprintf("already queued:");
            break;
        }
    }
    if(i == FIFO_SIZE) {
        PRINT_ERROR_OTHER("no more space in fifo")
        dprintf("can't push:");
    }
    dprintf(" %02x", cmd.cmd);
    if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
        dprintf(" %02x", cmd.ext);
    }
    dprintf("\n");
}

static inline s_cmd fifo_peek(s_cmd fifo[FIFO_SIZE]) {
    s_cmd cmd = fifo[0];
    if (cmd.cmd) {
        dprintf("peek: %02x", cmd.cmd);
        if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
            dprintf(" %02x", cmd.ext);
        }
        dprintf("\n");
    }
    return cmd;
}

static inline void fifo_remove(s_cmd fifo[FIFO_SIZE], s_cmd cmd) {
    int i;
    for (i = 0; i < FIFO_SIZE; ++i) {
        if (!fifo[i].cmd) {
            break;
        } else if (compare_cmd(fifo[i], cmd)) {
            dprintf("remove %02x", cmd.cmd);
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

#endif /* FF_COMMON_H_ */
