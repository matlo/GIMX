/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ffb_logitech.h>
#include <ghid.h>
#include <adapter.h>

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

#define EXT_CMD_CHANGE_MODE_DFP           0x01
#define EXT_CMD_WHEEL_RANGE_200_DEGREES   0x02
#define EXT_CMD_WHEEL_RANGE_900_DEGREES   0x03
#define EXT_CMD_CHANGE_MODE               0x09
#define EXT_CMD_REVERT_IDENTITY           0x0a
#define EXT_CMD_CHANGE_MODE_G25           0x10
#define EXT_CMD_CHANGE_MODE_G25_NO_DETACH 0x11
#define EXT_CMD_SET_RPM_LEDS              0x12
#define EXT_CMD_CHANGE_WHEEL_RANGE        0x81

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

static const char * get_cmd_name(unsigned char header) {
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

static struct {
    unsigned char value;
    const char * name;
} ext_cmd_names[] = {
    { EXT_CMD_CHANGE_MODE_DFP,           "CHANGE_MODE_DFP" },
    { EXT_CMD_WHEEL_RANGE_200_DEGREES,   "WHEEL_RANGE_200_DEGREES" },
    { EXT_CMD_WHEEL_RANGE_900_DEGREES,   "WHEEL_RANGE_900_DEGREES" },
    { EXT_CMD_CHANGE_MODE,               "CHANGE_MODE" },
    { EXT_CMD_REVERT_IDENTITY,           "REVERT_IDENTITY" },
    { EXT_CMD_CHANGE_MODE_G25,           "CHANGE_MODE_G25" },
    { EXT_CMD_CHANGE_MODE_G25_NO_DETACH, "CHANGE_MODE_G25_NO_DETACH" },
    { EXT_CMD_SET_RPM_LEDS,              "SET_RPM_LEDS" },
    { EXT_CMD_CHANGE_WHEEL_RANGE,        "CHANGE_WHEEL_RANGE" },
};

static const char * get_ext_cmd_name(unsigned char ext) {
    unsigned int i;
    for (i = 0; i < sizeof(ext_cmd_names) / sizeof(*ext_cmd_names); ++i) {
        if(ext_cmd_names[i].value == ext) {
            return ext_cmd_names[i].name;
        }
    }
    static char unknown[] = "UNKNOWN (255)";
    snprintf(unknown, sizeof(unknown), "UNKNOWN %hhu", ext);
    return unknown;
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
    unsigned char parameters[FFB_LOGITECH_OUTPUT_REPORT_SIZE - 1];
} s_force;

typedef struct {
    unsigned char updated;
    unsigned char cmd[FFB_LOGITECH_OUTPUT_REPORT_SIZE];
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

void ffb_lg_init_static(void) __attribute__((constructor (101)));
void ffb_lg_init_static(void)
{
    unsigned int i, j;
    for (i = 0; i < MAX_CONTROLLERS; ++i) {
        for(j = 0; j < FORCES_NB; ++j) {
            ffb_lg_device[i].forces[j].mask = 0x10 << j;
        }
    }
}

static const int debug = 0;

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

static unsigned char slot_index[] = { [FSLOT_1 >> 4] = 0, [FSLOT_2 >> 4] = 1, [FSLOT_3 >> 4] = 2, [FSLOT_4 >> 4] = 3, };

static inline int compare_cmd(s_cmd cmd1, s_cmd cmd2) {
    return cmd1.cmd == cmd2.cmd && (cmd1.cmd != CMD_EXTENDED_COMMAND || cmd1.ext == cmd2.ext);
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
    if(cmd.cmd == CMD_EXTENDED_COMMAND) {
        dprintf(" %02x", cmd.ext);
    }
    dprintf("\n");
}

static inline s_cmd fifo_peek(int device) {
    s_cmd * fifo = ffb_lg_device[device].fifo;
    s_cmd cmd = fifo[0];
    if (cmd.cmd) {
        dprintf("peek: %02x", cmd.cmd);
        if(cmd.cmd == CMD_EXTENDED_COMMAND) {
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
            if(cmd.cmd == CMD_EXTENDED_COMMAND) {
                dprintf(" %02x", cmd.ext);
            }
            dprintf("\n");
            memmove(fifo + i, fifo + i + 1, (FORCES_NB - i - 1) * sizeof(*fifo));
            memset(fifo + FORCES_NB - i - 1, 0x00, sizeof(*fifo));
            break;
        }
    }
}

static void decode_extended(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]) {

    dprintf("%s %s", get_cmd_name(data[0]), get_ext_cmd_name(data[1]));

    switch(data[1]) {
    case EXT_CMD_CHANGE_MODE_DFP:
    case EXT_CMD_WHEEL_RANGE_200_DEGREES:
    case EXT_CMD_WHEEL_RANGE_900_DEGREES:
    case EXT_CMD_CHANGE_MODE_G25:
    case EXT_CMD_CHANGE_MODE_G25_NO_DETACH:
      break;
    case EXT_CMD_CHANGE_MODE:
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
    case EXT_CMD_REVERT_IDENTITY:
      dprintf(" - %s", data[2] ? "REVERT" : "DO NOT REVERT");
      break;
    case EXT_CMD_SET_RPM_LEDS:
      dprintf(" - 0x%02x", data[2]);
      break;
    case EXT_CMD_CHANGE_WHEEL_RANGE:
      dprintf(" - %hu", (data[3] << 8) | data[2]);
      break;
    default:
      dprintf(" - ");
      dump(data + 2, FFB_LOGITECH_OUTPUT_REPORT_SIZE - 2);
      break;
    }
    dprintf("\n");
}

static void decode_command(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]) {

    dprintf("%s ", get_cmd_name(data[0]));

    switch(data[0] & 0x0f) {
    case CMD_PLAY:
    case CMD_STOP:
        dprintf(" 0x%02x", data[0]);
        break;
    case CMD_DOWNLOAD:
    case CMD_DOWNLOAD_AND_PLAY:
    case CMD_REFRESH_FORCE:
        dprintf(" - %s", get_ftype_name(data[1]));
        dprintf(" - ");
        dump(data + 2, FFB_LOGITECH_OUTPUT_REPORT_SIZE - 2);
        break;
    case CMD_DEFAULT_SPRING_ON:
    case CMD_DEFAULT_SPRING_OFF:
    case CMD_NORMAL_MODE:
    case CMD_RAW_MODE:
        break;
    case CMD_SET_LED:
        dprintf(" - 0x%02x", data[1]);
        break;
    case CMD_SET_WATCHDOG:
        dprintf(" - 0x%02x", data[1]);
        break;
    case CMD_FIXED_TIME_LOOP:
        dprintf(" - %s", data[1] ? "ON" : "OFF");
        break;
    case CMD_SET_DEFAULT_SPRING:
        dprintf(" - ");
        dump(data + 1, FFB_LOGITECH_OUTPUT_REPORT_SIZE - 1);
        break;
    case CMD_SET_DEAD_BAND:
        dprintf(" - %s", data[1] ? "ON" : "OFF");
        break;
    }
    dprintf("\n");
}

void ffb_logitech_process_report(int device, unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return;
    }

    if(debug) {
        dprintf("> ");
        if(data[0] == CMD_EXTENDED_COMMAND) {
            decode_extended(data);
        }
        else {
            decode_command(data);
        }
    }

    if(data[0] != CMD_EXTENDED_COMMAND) {

        unsigned char slots = data[0] & 0xf0;
        unsigned char cmd = data[0] & 0x0f;

        int i;
        s_force * forces = ffb_lg_device[device].forces;

        if (cmd == CMD_STOP && slots == 0xf0)
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
        case CMD_DOWNLOAD:
        case CMD_DOWNLOAD_AND_PLAY:
        case CMD_PLAY:
        case CMD_STOP:
        case CMD_REFRESH_FORCE:
        {
            for (i = 0; i < FORCES_NB; ++i) {
                if (slots & forces[i].mask) {
                    if (cmd == CMD_DOWNLOAD) {
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                        continue; // don't send anything yet
                    } else if (cmd == CMD_DOWNLOAD_AND_PLAY || cmd == CMD_REFRESH_FORCE) {
                        if(forces[i].active && !memcmp(forces[i].parameters, data + 1, sizeof(forces[i].parameters))) {
                            dprintf("> no change\n");
                            continue; // no change
                        }
                        forces[i].updated = 1;
                        forces[i].active = 1;
                        memcpy(forces[i].parameters, data + 1, sizeof(forces[i].parameters));
                    } else if (cmd == CMD_PLAY) {
                        if(forces[i].active) {
                            dprintf("> already playing\n");
                            continue; // already playing
                        }
                        forces[i].updated = 1;
                        forces[i].active = 1;
                    } else if (cmd == CMD_STOP) {
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
        s_cmd cmd = { CMD_EXTENDED_COMMAND, data[1] };

        switch(data[1]) {
        case EXT_CMD_CHANGE_MODE_DFP:
        case EXT_CMD_CHANGE_MODE:
        case EXT_CMD_REVERT_IDENTITY:
        case EXT_CMD_CHANGE_MODE_G25:
        case EXT_CMD_CHANGE_MODE_G25_NO_DETACH:
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

  if(data[0] != CMD_EXTENDED_COMMAND) {

      unsigned char slots = data[0] & 0xf0;
      unsigned char cmd = data[0] & 0x0f;

      int i;
      s_force * forces = ffb_lg_device[device].forces;

      switch(cmd)
      {
      case CMD_DOWNLOAD:
      case CMD_DOWNLOAD_AND_PLAY:
      case CMD_PLAY:
      case CMD_STOP:
      case CMD_REFRESH_FORCE:
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
              s_cmd cmd = { CMD_EXTENDED_COMMAND, data[1] };
              if(!ext_cmds[i].updated) {
                  fifo_remove(device, cmd);
              } else {
                  dprintf("do not remove %02x", cmd.cmd);
                  if(cmd.cmd == CMD_EXTENDED_COMMAND) {
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
        data[0] = mask | CMD_STOP;
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
        if(cmd.cmd != CMD_EXTENDED_COMMAND) {
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
                    data[0] = cmd.cmd | CMD_DOWNLOAD_AND_PLAY;
                    memcpy(data + 1, forces[index].parameters, sizeof(forces[index].parameters));
                } else {
                    data[0] = cmd.cmd | CMD_STOP;
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

static unsigned short lg_wheel_products[] = {
    USB_DEVICE_ID_LOGITECH_WINGMAN_FFG,
    USB_DEVICE_ID_LOGITECH_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFP_WHEEL,
    USB_DEVICE_ID_LOGITECH_G25_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFGT_WHEEL,
    USB_DEVICE_ID_LOGITECH_G27_WHEEL,
    USB_DEVICE_ID_LOGITECH_WII_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL2,
    USB_DEVICE_ID_LOGITECH_VIBRATION_WHEEL,
    USB_DEVICE_ID_LOGITECH_G920_WHEEL,
    USB_DEVICE_ID_LOGITECH_G29_WHEEL,
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

typedef struct
{
  unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE];
} s_native_mode;

static struct
{
  unsigned short product;
  s_native_mode command;
} native_modes[] =
{
    { USB_DEVICE_ID_LOGITECH_DFGT_WHEEL, { { 0x00, 0xf8, 0x09, 0x03, 0x01 } } },
    { USB_DEVICE_ID_LOGITECH_G27_WHEEL,  { { 0x00, 0xf8, 0x09, 0x04, 0x01 } } },
    { USB_DEVICE_ID_LOGITECH_G25_WHEEL,  { { 0x00, 0xf8, 0x10 } } },
    { USB_DEVICE_ID_LOGITECH_DFP_WHEEL,  { { 0x00, 0xf8, 0x01 } } },
};

static s_native_mode * get_native_mode_command(unsigned short product, unsigned short bcdDevice)
{
  unsigned short native = 0x0000;

  if(((USB_DEVICE_ID_LOGITECH_WHEEL == product) || (USB_DEVICE_ID_LOGITECH_DFP_WHEEL == product))
      && (0x1300 == (bcdDevice & 0xff00))) {
    native = USB_DEVICE_ID_LOGITECH_DFGT_WHEEL;
  } else if(((USB_DEVICE_ID_LOGITECH_WHEEL == product) || (USB_DEVICE_ID_LOGITECH_DFP_WHEEL == product) || (USB_DEVICE_ID_LOGITECH_G25_WHEEL == product))
      && (0x1230 == (bcdDevice & 0xfff0))) {
    native = USB_DEVICE_ID_LOGITECH_G27_WHEEL;
  } else if(((USB_DEVICE_ID_LOGITECH_WHEEL == product) || (USB_DEVICE_ID_LOGITECH_DFP_WHEEL == product))
      && (0x1200 == (bcdDevice & 0xff00))) {
    native = USB_DEVICE_ID_LOGITECH_G25_WHEEL;
  } else if((USB_DEVICE_ID_LOGITECH_WHEEL == product)
      && (0x1000 == (bcdDevice & 0xf000))) {
    native = USB_DEVICE_ID_LOGITECH_DFP_WHEEL;
  }

  unsigned int i;
  for (i = 0; i < sizeof(native_modes) / sizeof(*native_modes); ++i) {
    if (native_modes[i].product == native) {
      return &native_modes[i].command;
    }
  }

  return NULL;
}

void ffb_logitech_set_native_mode() {
    static int done = 0;
    if(done) {
      return;
    }
    done = 1;
    s_hid_dev * hid_devs = ghid_enumerate(USB_VENDOR_ID_LOGITECH, 0x0000);
    s_hid_dev * current;
    for(current = hid_devs; current != NULL; ++current) {
        if(ffb_logitech_is_logitech_wheel(current->vendor_id, current->product_id)) {
            int hid = ghid_open_path(current->path);
            if(hid >= 0) {
                unsigned char reset = 0;
                const s_hid_info * hid_info = ghid_get_hid_info(hid);
                if(hid_info != NULL) {
                  s_native_mode * command = get_native_mode_command(hid_info->product_id, hid_info->bcdDevice);
                  if(command) {
                    reset = 1;
                    // send the native mode command
                    int ret = ghid_write_timeout(hid, command->data, sizeof(command->data), 1);
                    if(ret == 0) {
                      fprintf(stderr, "failed to send native mode command for HID device %s (PID=%04x)\n", current->path, current->product_id);
                    } else {
                      printf("native mode command sent to HID device %s (PID=%04x)\n", current->path, current->product_id);
                    }
                  } else {
                    printf("native mode is already enabled for HID device %s (PID=%04x)\n", current->path, current->product_id);
                  }
                }
                ghid_close(hid);
                if(reset) {
                  // wait up to 5 seconds for the device to reset
                  int cpt = 0;
                  do
                  {
                    // sleep 1 second between each retry
                    int i;
                    for (i = 0; i < 10; ++i)
                    {
                      usleep(100000);
                    }
                    ++cpt;
                  } while((hid = ghid_open_path(current->path)) < 0 && cpt < 5);
                  if(hid >= 0) {
                    const s_hid_info * hid_info = ghid_get_hid_info(hid);
                    // verify that the native mode is enabled
                    s_native_mode * command = get_native_mode_command(hid_info->product_id, hid_info->bcdDevice);
                    if(command) {
                      fprintf(stderr, "failed to enable native mode for HID device %s\n", current->path);
                    } else {
                      printf("native mode enabled for HID device %s (PID=%04x)\n", current->path, hid_info->product_id);
                    }
                    ghid_close(hid);
                  } else {
                    fprintf(stderr, "HID device %s not found\n", current->path);
                  }
                }
            }
        }
        if(current->next == 0) {
            break;
        }
    }

    ghid_free_enumeration(hid_devs);
}
