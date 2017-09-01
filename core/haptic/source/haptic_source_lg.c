/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <haptic/common/ff_lg.h>
#include <gimxcommon/include/gerror.h>
#include <haptic/haptic_common.h>
#include <haptic/haptic_source.h>

#define FIFO_SIZE 16

typedef struct {
    unsigned char mask;
    unsigned char playing;
    unsigned char updated;
    unsigned char parameters[FF_LG_OUTPUT_REPORT_SIZE - 1];
} s_force;

typedef struct {
    unsigned char updated;
    unsigned char cmd[FF_LG_OUTPUT_REPORT_SIZE];
} s_ext_cmd;

struct haptic_source_state {
    uint8_t caps;
    uint8_t cmd_offset;
    uint16_t range; // the current wheel range (0 means unknown)
    s_force forces[FF_LG_FSLOTS_NB];
    s_ext_cmd ext_cmds[FF_LG_EXT_CMD_NB];
    s_cmd fifo[FIFO_SIZE];
};

static void process_extended(struct haptic_source_state * state, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]);

static void set_wheel_range(struct haptic_source_state * state, unsigned short range) {

    if (range == state->range) {
        return;
    }

    state->range = range;

    unsigned char change_wheel_range[FF_LG_OUTPUT_REPORT_SIZE] = {
            FF_LG_CMD_EXTENDED_COMMAND,
            FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE,
            range & 0xFF,
            range >> 8
    };
    process_extended(state, change_wheel_range);
}

struct haptic_source_state * haptic_source_lg_init(s_haptic_core_ids ids) {

    void * ptr = calloc(1, sizeof(struct haptic_source_state));

    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc")
        return NULL;
    }

    struct haptic_source_state * state = (struct haptic_source_state *) ptr;

    unsigned int j;
    for(j = 0; j < FF_LG_FSLOTS_NB; ++j) {
        state->forces[j].mask = 0x10 << j;
    }

    state->caps = ff_lg_get_caps(ids.pid);

    if (ids.pid == USB_PRODUCT_ID_LOGITECH_G29_WHEEL) {
        state->cmd_offset = 1;
    }

    set_wheel_range(state, ff_lg_get_wheel_range(ids.pid));

    return state;
}

static void haptic_source_lg_clean(struct haptic_source_state * state) {

    free(state);
}
static void process_extended(struct haptic_source_state * state, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    s_cmd cmd = { FF_LG_CMD_EXTENDED_COMMAND, data[1] };

    int i;
    for (i = 0; i < FF_LG_EXT_CMD_NB; ++i) {
        s_ext_cmd * ext_cmd = state->ext_cmds + i;
        if(!ext_cmd->cmd[0]) {
            memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
            ext_cmd->updated = 1;
            ff_lg_fifo_push(state->fifo, cmd, 1);
            break;
        } else if(ext_cmd->cmd[1] == data[1]) {
            if(memcmp(ext_cmd->cmd, data, sizeof(ext_cmd->cmd))) {
                memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
                ext_cmd->updated = 1;
                ff_lg_fifo_push(state->fifo, cmd, 1);
            } else {
                dprintf("> no change\n");
            }
            break;
        }
    }
}

static void haptic_source_lg_process(struct haptic_source_state * state, size_t size __attribute((unused)), const unsigned char * data) {

    data += state->cmd_offset;

    if(gimx_params.debug.haptic) {
        dprintf("> ");
        if(data[0] == FF_LG_CMD_EXTENDED_COMMAND) {
            ff_lg_decode_extended(data);
        }
        else {
            ff_lg_decode_command(data);
        }
    }

    if(data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

        unsigned char slots = data[0] & FF_LG_FSLOT_MASK;
        unsigned char cmd = data[0] & FF_LG_CMD_MASK;

        int i;
        s_force * forces = state->forces;

        if (cmd == FF_LG_CMD_STOP && slots == 0xf0)
        {
          // stop all forces, whatever their current states
          // this is useful at startup, where all forces are considered stopped
          for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
              forces[i].updated = 1;
              forces[i].playing = 0;
              memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
              s_cmd cmd = { forces[i].mask, 0x00 };
              ff_lg_fifo_push(state->fifo, cmd, 1);
          }
          return;
        }

        switch(cmd)
        {
        case FF_LG_CMD_DOWNLOAD:
        case FF_LG_CMD_DOWNLOAD_AND_PLAY:
        case FF_LG_CMD_PLAY:
        case FF_LG_CMD_STOP:
        case FF_LG_CMD_REFRESH_FORCE:
        {
            for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
                if (slots & forces[i].mask) {
                    if (cmd == FF_LG_CMD_DOWNLOAD) {
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                        continue; // don't send anything yet
                    } else if (cmd == FF_LG_CMD_DOWNLOAD_AND_PLAY || cmd == FF_LG_CMD_REFRESH_FORCE) {
                        if(forces[i].playing && !memcmp(forces[i].parameters, data + 1, sizeof(forces[i].parameters))) {
                            dprintf("> no change\n");
                            continue; // no change
                        }
                        forces[i].updated = 1;
                        forces[i].playing = 1;
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                    } else if (cmd == FF_LG_CMD_PLAY) {
                        if(forces[i].playing) {
                            dprintf("> already playing\n");
                            continue; // already playing
                        }
                        forces[i].updated = 1;
                        forces[i].playing = 1;
                    } else if (cmd == FF_LG_CMD_STOP) {
                        if(!forces[i].playing) {
                            dprintf("> already stopped\n");
                            continue; // already stopped
                        }
                        forces[i].updated = 1;
                        forces[i].playing = 0;
                        memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
                    } else {
                        continue;
                    }
                    s_cmd cmd = { forces[i].mask, 0x00 };
                    ff_lg_fifo_push(state->fifo, cmd, 1);
                }
            }
        }
        break;
        default:
        {
            static int warn = 1;
            if (warn == 1) {
                gwarn("skipping unsupported command %s\n", ff_lg_get_cmd_name(cmd));
                warn = 0;
            }
        }
        break;
        }
    }
    else {

        unsigned short range = 0;
        switch(data[1]) {
        case FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES:
            range = 200;
            break;
        case FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES:
            range = 900;
            break;
        case FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE:
            range = (data[3] << 8) | data[2];
            break;
        }

        if (range != 0) {
            set_wheel_range(state, range);
            return;
        }

        switch(data[1]) {
        case FF_LG_EXT_CMD_CHANGE_MODE_DFP:
        case FF_LG_EXT_CMD_CHANGE_MODE:
        case FF_LG_EXT_CMD_REVERT_IDENTITY:
        case FF_LG_EXT_CMD_CHANGE_MODE_G25:
        case FF_LG_EXT_CMD_CHANGE_MODE_G25_NO_DETACH:
          {
              static int warn = 1;
              if (warn == 1) {
                  gwarn("skipping unsupported change wheel mode commands\n");
                  warn = 0;
              }
          }
          return;
        default:
          break;
        }

        process_extended(state, data);
    }
}

static unsigned char slot_index[] = { [FF_LG_FSLOT_1 >> 4] = 0, [FF_LG_FSLOT_2 >> 4] = 1, [FF_LG_FSLOT_3 >> 4] = 2, [FF_LG_FSLOT_4 >> 4] = 3, };

static int haptic_source_lg_get(struct haptic_source_state * state, s_haptic_core_data * data) {

    s_cmd cmd = ff_lg_fifo_peek(state->fifo);
    if (cmd.cmd) {
        if(cmd.cmd != FF_LG_CMD_EXTENDED_COMMAND) {
            if(cmd.cmd & FF_LG_CMD_MASK)
            {
                // not a slot update
                return 0;
            }
            else
            {
                // slot update: cmd.cmd is in { FSLOT_1, FSLOT_2, FSLOT_3, FSLOT_4 }
                unsigned char index = slot_index[cmd.cmd >> 4];
                s_force * force = state->forces + index;
                force->updated = 0;
                return ff_lg_convert_force(state->caps, index, (s_ff_lg_command *)state->forces[index].parameters, state->forces[index].playing, data);
            }
        }
        else {
            int i;
            for (i = 0; i < FF_LG_EXT_CMD_NB; ++i) {
                s_ext_cmd * ext_cmd = state->ext_cmds + i;
                if(ext_cmd->cmd[1] == cmd.ext) {
                    ext_cmd->updated = 0;
                    return ff_lg_convert_extended((s_ff_lg_command *)(ext_cmd->cmd + 1), data);
                }
            }
        }
    }

    return 0;
}

static s_haptic_core_ids haptic_source_lg_ids[] = {
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP  },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE     },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL        },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DFP_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G25_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL        },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G27_WHEEL         },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2       },
        { .vid = USB_VENDOR_ID_LOGITECH,  .pid = USB_PRODUCT_ID_LOGITECH_G29_WHEEL         },
        { .vid = 0x0000,      .pid = 0x0000       }, // end of table
};

static s_haptic_source source_lg = {
        .name = "haptic_source_lg",
        .ids = haptic_source_lg_ids,
        .init = haptic_source_lg_init,
        .clean = haptic_source_lg_clean,
        .process = haptic_source_lg_process,
        .get = haptic_source_lg_get
};

void haptic_source_lg_constructor(void) __attribute__((constructor));
void haptic_source_lg_constructor(void) {

    if (haptic_source_register(&source_lg) == -1) {
        PRINT_ERROR_OTHER("failed to register source")
    }
}
