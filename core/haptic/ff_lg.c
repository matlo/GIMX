/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <haptic/ff_lg.h>
#include <ghid.h>
#include <adapter.h>

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

static const char * get_cmd_name(unsigned char header) {
    if (header == FF_LG_CMD_EXTENDED_COMMAND) {
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

static const char * get_ext_cmd_name(unsigned char ext) {
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

static const char * get_ftype_name(unsigned char ftype) {
    if (ftype < sizeof(ftype_names) / sizeof(*ftype_names)) {
        return ftype_names[ftype];
    } else {
        return "UNKNOWN";
    }
}

#define FORCES_NB 4
#define EXT_CMD_NB 16
#define FIFO_SIZE 16

typedef struct {
    unsigned char mask;
    unsigned char active;
    unsigned char updated;
    unsigned char parameters[FF_LG_OUTPUT_REPORT_SIZE - 1];
} s_force;

typedef struct {
    unsigned char updated;
    unsigned char cmd[FF_LG_OUTPUT_REPORT_SIZE];
} s_ext_cmd;

typedef struct {
    unsigned char cmd;
    unsigned char ext;
} s_cmd;

static struct
{
    s_force forces[FORCES_NB];
    s_ext_cmd ext_cmds[EXT_CMD_NB];
    s_cmd fifo[FIFO_SIZE];
    s_ffb_report last_report;
} ffb_lg_device[MAX_CONTROLLERS] = {};

void ffb_lg_init_static(void) __attribute__((constructor));
void ffb_lg_init_static(void)
{
    unsigned int i, j;
    for (i = 0; i < MAX_CONTROLLERS; ++i) {
        for(j = 0; j < FORCES_NB; ++j) {
            ffb_lg_device[i].forces[j].mask = 0x10 << j;
        }
    }
}

static const int debug = 1;

#define dprintf(...) if(debug) printf(__VA_ARGS__)

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

static void dump(unsigned char* packet, unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i)
  {
    printf("0x%02x ", packet[i]);
  }
}

static unsigned char slot_index[] = { [FF_LG_FSLOT_1 >> 4] = 0, [FF_LG_FSLOT_2 >> 4] = 1, [FF_LG_FSLOT_3 >> 4] = 2, [FF_LG_FSLOT_4 >> 4] = 3, };

static inline int compare_cmd(s_cmd cmd1, s_cmd cmd2) {
    return cmd1.cmd == cmd2.cmd && (cmd1.cmd != FF_LG_CMD_EXTENDED_COMMAND || cmd1.ext == cmd2.ext);
}

static inline void fifo_push(int device, s_cmd cmd, int replace) {
    s_cmd * fifo = ffb_lg_device[device].fifo;
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

static inline s_cmd fifo_peek(int device) {
    s_cmd * fifo = ffb_lg_device[device].fifo;
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

static inline void fifo_remove(int device, s_cmd cmd) {
    s_cmd * fifo = ffb_lg_device[device].fifo;
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
            memmove(fifo + i, fifo + i + 1, (FORCES_NB - i - 1) * sizeof(*fifo));
            memset(fifo + FORCES_NB - i - 1, 0x00, sizeof(*fifo));
            break;
        }
    }
}

static void decode_extended(unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    dprintf("%s %s", get_cmd_name(data[0]), get_ext_cmd_name(data[1]));

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

static void decode_command(unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    dprintf("%s ", get_cmd_name(data[0]));

    switch(data[0] & 0x0f) {
    case FF_LG_CMD_PLAY:
    case FF_LG_CMD_STOP:
        dprintf(" 0x%02x", data[0]);
        break;
    case FF_LG_CMD_DOWNLOAD:
    case FF_LG_CMD_DOWNLOAD_AND_PLAY:
    case FF_LG_CMD_REFRESH_FORCE:
        dprintf(" - %s", get_ftype_name(data[1]));
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

void ffb_logitech_process_report(int device, unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return;
    }

    if(debug) {
        dprintf("> ");
        if(data[0] == FF_LG_CMD_EXTENDED_COMMAND) {
            decode_extended(data);
        }
        else {
            decode_command(data);
        }
    }

    if(data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

        unsigned char slots = data[0] & 0xf0;
        unsigned char cmd = data[0] & 0x0f;

        int i;
        s_force * forces = ffb_lg_device[device].forces;

        if (cmd == FF_LG_CMD_STOP && slots == 0xf0)
        {
          // stop all forces, whatever their current states
          // this is useful at startup, where all forces are considered stopped
          for (i = 0; i < FORCES_NB; ++i) {
              forces[i].updated = 1;
              forces[i].active = 0;
              memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
              s_cmd cmd = { forces[i].mask, 0x00 };
              fifo_push(device, cmd, 1);
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
            for (i = 0; i < FORCES_NB; ++i) {
                if (slots & forces[i].mask) {
                    if (cmd == FF_LG_CMD_DOWNLOAD) {
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                        continue; // don't send anything yet
                    } else if (cmd == FF_LG_CMD_DOWNLOAD_AND_PLAY || cmd == FF_LG_CMD_REFRESH_FORCE) {
                        if(forces[i].active && !memcmp(forces[i].parameters, data + 1, sizeof(forces[i].parameters))) {
                            dprintf("> no change\n");
                            continue; // no change
                        }
                        forces[i].updated = 1;
                        forces[i].active = 1;
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                    } else if (cmd == FF_LG_CMD_PLAY) {
                        if(forces[i].active) {
                            dprintf("> already playing\n");
                            continue; // already playing
                        }
                        forces[i].updated = 1;
                        forces[i].active = 1;
                    } else if (cmd == FF_LG_CMD_STOP) {
                        if(!forces[i].active) {
                            dprintf("> already stopped\n");
                            continue; // already stopped
                        }
                        forces[i].updated = 1;
                        forces[i].active = 0;
                        memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
                    } else {
                        continue;
                    }
                    s_cmd cmd = { forces[i].mask, 0x00 };
                    fifo_push(device, cmd, 1);
                }
            }
        }
        break;
        default:
        {
            s_cmd cmd = { data[0], 0x00 };
            fifo_push(device, cmd, 0);
        }
        break;
        }
    }
    else {
        s_cmd cmd = { FF_LG_CMD_EXTENDED_COMMAND, data[1] };

        switch(data[1]) {
        case FF_LG_EXT_CMD_CHANGE_MODE_DFP:
        case FF_LG_EXT_CMD_CHANGE_MODE:
        case FF_LG_EXT_CMD_REVERT_IDENTITY:
        case FF_LG_EXT_CMD_CHANGE_MODE_G25:
        case FF_LG_EXT_CMD_CHANGE_MODE_G25_NO_DETACH:
          return;
        }

        int i;
        for (i = 0; i < EXT_CMD_NB; ++i) {
            s_ext_cmd * ext_cmd = ffb_lg_device[device].ext_cmds + i;
            if(!ext_cmd->cmd[0]) {
                memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
                ext_cmd->updated = 1;
                fifo_push(device, cmd, 1);
                break;
            } else if(ext_cmd->cmd[1] == data[1]) {
                if(memcmp(ext_cmd->cmd, data, sizeof(ext_cmd->cmd))) {
                    memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
                    ext_cmd->updated = 1;
                    fifo_push(device, cmd, 1);
                } else {
                    dprintf("> no change\n");
                }
                break;
            }
        }
    }
}

void ffb_logitech_ack(int device) {

  if (device < 0 || device >= MAX_CONTROLLERS) {
      fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
      return;
  }

  dprintf("> ack\n");

  unsigned char * data = ffb_lg_device[device].last_report.data + 1;

  if(data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

      unsigned char slots = data[0] & 0xf0;
      unsigned char cmd = data[0] & 0x0f;

      int i;
      s_force * forces = ffb_lg_device[device].forces;

      switch(cmd)
      {
      case FF_LG_CMD_DOWNLOAD:
      case FF_LG_CMD_DOWNLOAD_AND_PLAY:
      case FF_LG_CMD_PLAY:
      case FF_LG_CMD_STOP:
      case FF_LG_CMD_REFRESH_FORCE:
      {
          for (i = 0; i < FORCES_NB; ++i) {
              if (slots & forces[i].mask) {
                  s_cmd cmd = { forces[i].mask, 0x00 };
                  if(!forces[i].updated) {
                      fifo_remove(device, cmd);
                  } else {
                      dprintf("do not remove %02x\n", cmd.cmd);
                  }
              }
          }
      }
      break;
      default:
      {
          s_cmd cmd = { data[0], 0x00 };
          fifo_remove(device, cmd);
      }
      break;
      }

  } else {

      s_ext_cmd * ext_cmds = ffb_lg_device[device].ext_cmds;

      int i;
      for (i = 0; i < EXT_CMD_NB; ++i) {
          if(ext_cmds[i].cmd[1] == data[1]) {
              s_cmd cmd = { FF_LG_CMD_EXTENDED_COMMAND, data[1] };
              if(!ext_cmds[i].updated) {
                  fifo_remove(device, cmd);
              } else {
                  dprintf("do not remove %02x", cmd.cmd);
                  if(cmd.cmd == FF_LG_CMD_EXTENDED_COMMAND) {
                      dprintf(" %02x", cmd.ext);
                  }
                  dprintf("\n");
              }
              break;
          }
      }
  }
}

static inline void clear_report(int device) {

  memset(ffb_lg_device[device].last_report.data, 0x00, sizeof(ffb_lg_device[device].last_report.data));
}

s_ffb_report * ffb_logitech_get_report(int device) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return NULL;
    }

    int i;
    s_force * forces = ffb_lg_device[device].forces;
    unsigned char * data = ffb_lg_device[device].last_report.data + 1;

    unsigned char mask = 0;
    // look for forces to stop
    for (i = 0; i < FORCES_NB; ++i) {
        if (forces[i].updated && !forces[i].active) {
            mask |= forces[i].mask;
        }
    }

    // if multiple forces have to be stopped, then stop them
    if (fslot_nbits[mask >> 4] > 1) {
        clear_report(device);
        data[0] = mask | FF_LG_CMD_STOP;
        for (i = 0; i < FORCES_NB; ++i) {
            if (mask & forces[i].mask) {
                forces[i].updated = 0;
            }
        }
        dprintf("< %s %02x\n", get_cmd_name(data[0]), data[0]);
        return &ffb_lg_device[device].last_report;
    }

    s_cmd cmd = fifo_peek(device);
    if (cmd.cmd) {
        clear_report(device);
        dprintf("< ");
        if(cmd.cmd != FF_LG_CMD_EXTENDED_COMMAND) {
            if(cmd.cmd & 0x0f)
            {
                // not a slot update
                data[0] = cmd.cmd;
                if(debug) {
                    decode_command(data);
                }
                return &ffb_lg_device[device].last_report;
            }
            else
            {
                // slot update: cmd.cmd is in { FSLOT_1, FSLOT_2, FSLOT_3, FSLOT_4 }
                unsigned char index = slot_index[cmd.cmd >> 4];
                if (forces[index].active) {
                    data[0] = cmd.cmd | FF_LG_CMD_DOWNLOAD_AND_PLAY;
                    memcpy(data + 1, forces[index].parameters, sizeof(forces[index].parameters));
                } else {
                    data[0] = cmd.cmd | FF_LG_CMD_STOP;
                }
                forces[index].updated = 0;
                if(debug) {
                    decode_command(data);
                }
                return &ffb_lg_device[device].last_report;
            }
        }
        else {
            int i;
            for (i = 0; i < EXT_CMD_NB; ++i) {
                s_ext_cmd * ext_cmd = ffb_lg_device[device].ext_cmds + i;
                if(ext_cmd->cmd[1] == cmd.ext) {
                    memcpy(data, ext_cmd->cmd, sizeof(ext_cmd->cmd));
                    if(debug) {
                        decode_extended(data);
                    }
                    ext_cmd->updated = 0;
                    return &ffb_lg_device[device].last_report;
                }
            }
        }
    }

    return NULL;
}

static void get_slot(int device, unsigned char index, GE_Event * haptic) {

    unsigned char type = ffb_lg_device[device].forces[index].parameters[0];

    printf("%02x %02x", index, type);

    switch(type) {
    case FF_LG_FTYPE_CONSTANT:
        haptic->type = GE_JOYCONSTANTFORCE;
        if (ffb_lg_device[device].forces[index].active) {
            unsigned char level = ffb_lg_device[device].forces[index].parameters[1];
            haptic->jconstant.level = level << 8;
        } else {
            haptic->jconstant.level = 0;
        }
        printf("level: %d\n", haptic->jconstant.level);
        break;
    case FF_LG_FTYPE_SPRING:
        haptic->type = GE_JOYSPRINGFORCE;
        break;
    case FF_LG_FTYPE_DAMPER:
        haptic->type = GE_JOYDAMPERFORCE;
        break;
    default:
        return;
    }

}

GE_Event * ffb_logitech_convert_report(int device, s_ffb_report * report) {

    static GE_Event haptic = {};

    unsigned char slots = report->data[1] & 0xf0;
    unsigned char cmd = report->data[1] & 0x0f;

    unsigned char index;
    s_force * forces = ffb_lg_device[device].forces;

    switch(cmd)
    {
    case FF_LG_CMD_DOWNLOAD_AND_PLAY:
        index = slot_index[slots >> 4];
        get_slot(device, index, &haptic);
        report->data[1] = 0;
        return &haptic;
    case FF_LG_CMD_STOP:
        for (index = 0; index < FORCES_NB; ++index) {
            if (slots & forces[index].mask) {
                get_slot(device, index, &haptic);
                report->data[1] &= ~forces[index].mask;
                return &haptic;
            }
        }
        break;
    default:
    break;
    }
    return NULL;
}

static unsigned short lg_wheel_products[] = {
        USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE,
        USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
        USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE,
        USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL,
        USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
        USB_PRODUCT_ID_LOGITECH_G25_WHEEL,
        USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL,
        USB_PRODUCT_ID_LOGITECH_G27_WHEEL,
        USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2,
        USB_PRODUCT_ID_LOGITECH_G920_WHEEL,
        USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
};

int ffb_logitech_is_logitech_wheel(unsigned short vendor, unsigned short product) {

    if(vendor != USB_VENDOR_ID_LOGITECH) {
        return 0;
    }
    unsigned int i;
    for(i = 0; i < sizeof(lg_wheel_products) / sizeof(*lg_wheel_products); ++i) {
        if(lg_wheel_products[i] == product) {
            return 1;
        }
    }
    return 0;
}
