/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <haptic/ff_lg.h>
#include <haptic/ff_common.h>
#include <ghid.h>
#include <adapter.h>
#include <gimx.h>
#include <limits.h>

#undef dprintf
#define dprintf(...) if(gimx_params.debug.ff_lg) printf(__VA_ARGS__)

#define CLAMP(MIN,VALUE,MAX) (((VALUE) < MIN) ? (MIN) : (((VALUE) > MAX) ? (MAX) : (VALUE)))

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
        unsigned char cmd = header & FF_LG_CMD_MASK;
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

const char * ff_lg_get_ftype_name(unsigned char ftype) {
    if (ftype < sizeof(ftype_names) / sizeof(*ftype_names)) {
        return ftype_names[ftype];
    } else {
        return "UNKNOWN";
    }
}

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

static struct
{
    struct {
        unsigned short pid;
        unsigned short range; // the current wheel range (0 means unknown)
    } src; // force feedback commands come from this wheel
    struct {
        unsigned short pid;
        unsigned short range; // 0 means the wheel supports adjusting the range
    } dst; // force feedback commands go to this wheel
    int convert_lr_coef; // convert 'old' to 'new' or 'new' to 'old' low-res spring/damper coefficients
    int convert_hr2lr; // convert high-res to low-res spring/damper effects
    int skip_leds; // skip FF_LG_CMD_SET_LED commands
    s_force forces[FF_LG_FSLOTS_NB];
    s_ext_cmd ext_cmds[FF_LG_EXT_CMD_NB];
    s_cmd fifo[FIFO_SIZE];
    s_ff_lg_report last_report;
} ff_lg_device[MAX_CONTROLLERS] = {};

static inline int check_device(int device, const char * file, unsigned int line, const char * func) {
  if(device < 0 || device >= MAX_CONTROLLERS) {
      fprintf(stderr, "%s:%d %s: invalid device (%d)\n", file, line, func, device);
      return -1;
  }
  return 0;
}
#define CHECK_DEVICE(device,retValue) \
  if(check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

void ff_lg_init_static(void) __attribute__((constructor));
void ff_lg_init_static(void)
{
    unsigned int i, j;
    for (i = 0; i < MAX_CONTROLLERS; ++i) {
        for(j = 0; j < FF_LG_FSLOTS_NB; ++j) {
            ff_lg_device[i].forces[j].mask = 0x10 << j;
        }
    }
}

/*
 * Low-res spring/damper coefficients 5 and 6 are swapped for these wheels.
 */
static inline int is_old_lr_coef_wheel(unsigned short pid) {

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
    case USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE:
        return 1;
    default:
        return 0;
    }
}

/*
 * High-res spring/damper forces are supported since Driving Force,
 * since Driving Force Pro introduced variations of these forces.
 */
static inline int is_hr_wheel(unsigned short pid) {

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
        return 0;
    default:
        return 1;
    }
}

/*
 * Tell if FF_LG_CMD_SET_LED commands should be skipped.
 */
static inline int skip_leds(unsigned short pid) {

    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_G27_WHEEL:
    case USB_PRODUCT_ID_LOGITECH_G29_WHEEL:
        return 0;
    default:
        return 1;
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

static void process_extended(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE], int force);

static void set_wheel_range(int device, unsigned short range) {

    if (ff_lg_device[device].dst.range != 0) {
        static int warn = 1;
        if (warn == 1) {
            ncprintf("skipping unsupported change wheel range commands\n");
            warn = 0;
        }
        return;
    }

    if (range == ff_lg_device[device].src.range) {
        return;
    }

    ff_lg_device[device].src.range = range;

    if (ff_lg_device[device].dst.pid == USB_PRODUCT_ID_LOGITECH_DFP_WHEEL) {
        unsigned char report1[FF_LG_OUTPUT_REPORT_SIZE] = {
                FF_LG_CMD_EXTENDED_COMMAND,
        };
        unsigned short full_range;
        if (range > 200) {
            report1[1] = FF_LG_EXT_CMD_WHEEL_RANGE_900_DEGREES;
            full_range = 900;
        } else {
            report1[1] = FF_LG_EXT_CMD_WHEEL_RANGE_200_DEGREES;
            full_range = 200;
        }
        process_extended(device, report1, 1);
        ncprintf("wheel range adjusted to %hu degrees\n", full_range);
        if (range != full_range) {
            static int warn = 1;
            if (warn == 1) {
                ncprintf("Driving Force Pro currently only supports 200 and 900 degree ranges\n");
                warn = 0;
            }
            // division by 2 is performed when computing high and low order bits
            /*unsigned short d1 = (full_range - range + 1) * 0x7FF / full_range;
            unsigned short d2 = 0xFFF - d1;
            unsigned char report2[FF_LG_OUTPUT_REPORT_SIZE] = {
                    FF_LG_FSLOT_4 | FF_LG_CMD_DOWNLOAD_AND_PLAY,
                    FF_LG_FTYPE_HIGH_RESOLUTION_SPRING,
                    d1 >> 4,
                    d2 >> 4,
                    0xff,
                    (d2 & 0x0e) << 4 | (d1 & 0x0e),
                    0xff
            };
            process_extended(device, report2, 1);*/
        }
    } else {
        unsigned char change_wheel_range[FF_LG_OUTPUT_REPORT_SIZE] = {
                FF_LG_CMD_EXTENDED_COMMAND,
                FF_LG_EXT_CMD_CHANGE_WHEEL_RANGE,
                range & 0xFF,
                range >> 8
        };
        process_extended(device, change_wheel_range, 0);
        ncprintf("wheel range adjusted to %hu degrees\n", range);
    }
}

int ff_lg_init(int device, unsigned short src_pid, unsigned short dst_pid) {

    CHECK_DEVICE(device, -1)

    ff_lg_device[device].dst.pid = dst_pid;
    ff_lg_device[device].src.pid = src_pid;

    ff_lg_device[device].convert_lr_coef = is_old_lr_coef_wheel(src_pid) ^ is_old_lr_coef_wheel(dst_pid);
    ff_lg_device[device].convert_hr2lr = is_hr_wheel(src_pid) && !is_hr_wheel(dst_pid);
    ff_lg_device[device].skip_leds = skip_leds(dst_pid) || gimx_params.skip_leds;
    ff_lg_device[device].dst.range = ff_lg_get_wheel_range(dst_pid);

    unsigned short src_range = ff_lg_get_wheel_range(ff_lg_device[device].src.pid);
    if (src_range != 0 && ff_lg_device[device].dst.range == 0) {
        // source wheel range is fixed and dest wheel supports adjusting the range
        set_wheel_range(device, src_range);
    }

    return 0;
}

static void dump(const unsigned char* packet, unsigned char length)
{
  int i;
  for(i = 0; i < length; ++i)
  {
    printf("0x%02x ", packet[i]);
  }
}

static unsigned char slot_index[] = { [FF_LG_FSLOT_1 >> 4] = 0, [FF_LG_FSLOT_2 >> 4] = 1, [FF_LG_FSLOT_3 >> 4] = 2, [FF_LG_FSLOT_4 >> 4] = 3, };

void ff_lg_decode_extended(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

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

void ff_lg_decode_command(const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    dprintf("%s ", get_cmd_name(data[0]));

    switch(data[0] & FF_LG_CMD_MASK) {
    case FF_LG_CMD_PLAY:
    case FF_LG_CMD_STOP:
        dprintf(" 0x%02x", data[0]);
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

static void process_extended(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE], int force) {

    s_cmd cmd = { FF_LG_CMD_EXTENDED_COMMAND, data[1] };

    int i;
    for (i = 0; i < FF_LG_EXT_CMD_NB; ++i) {
        s_ext_cmd * ext_cmd = ff_lg_device[device].ext_cmds + i;
        if(!ext_cmd->cmd[0]) {
            memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
            ext_cmd->updated = 1;
            fifo_push(ff_lg_device[device].fifo, cmd, 1);
            break;
        } else if(ext_cmd->cmd[1] == data[1]) {
            if(force != 0 || memcmp(ext_cmd->cmd, data, sizeof(ext_cmd->cmd))) {
                memcpy(ext_cmd->cmd, data, sizeof(ext_cmd->cmd));
                ext_cmd->updated = 1;
                fifo_push(ff_lg_device[device].fifo, cmd, 1);
            } else {
                dprintf("> no change\n");
            }
            break;
        }
    }
}

void ff_lg_process_report(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return;
    }

    if(gimx_params.debug.ff_lg) {
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
        s_force * forces = ff_lg_device[device].forces;

        if (cmd == FF_LG_CMD_STOP && slots == 0xf0)
        {
          // stop all forces, whatever their current states
          // this is useful at startup, where all forces are considered stopped
          for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
              forces[i].updated = 1;
              forces[i].active = 0;
              memset(forces[i].parameters, 0x00, sizeof(forces[i].parameters));
              s_cmd cmd = { forces[i].mask, 0x00 };
              fifo_push(ff_lg_device[device].fifo, cmd, 1);
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
                    fifo_push(ff_lg_device[device].fifo, cmd, 1);
                }
            }
        }
        break;
        default:
        {
            s_cmd cmd = { data[0], 0x00 };
            fifo_push(ff_lg_device[device].fifo, cmd, 0);
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
            set_wheel_range(device, range);
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
                  ncprintf("skipping unsupported change wheel mode commands\n");
                  warn = 0;
              }
          }
          return;
        case FF_LG_EXT_CMD_SET_RPM_LEDS:
          if (ff_lg_device[device].skip_leds) {
              return;
          }
          break;
        default:
          return;
        }

        process_extended(device, data, 0);
    }
}

void ff_lg_ack(int device) {

  if (device < 0 || device >= MAX_CONTROLLERS) {
      fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
      return;
  }

  dprintf("> ack\n");

  unsigned char * data = ff_lg_device[device].last_report.data + 1;

  if(data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

      unsigned char slots = data[0] & FF_LG_FSLOT_MASK;
      unsigned char cmd = data[0] & FF_LG_CMD_MASK;

      int i;
      s_force * forces = ff_lg_device[device].forces;

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
                  s_cmd cmd = { forces[i].mask, 0x00 };
                  if(!forces[i].updated) {
                      fifo_remove(ff_lg_device[device].fifo, cmd);
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
          fifo_remove(ff_lg_device[device].fifo, cmd);
      }
      break;
      }

  } else {

      s_ext_cmd * ext_cmds = ff_lg_device[device].ext_cmds;

      int i;
      for (i = 0; i < FF_LG_EXT_CMD_NB; ++i) {
          if(ext_cmds[i].cmd[1] == data[1]) {
              s_cmd cmd = { FF_LG_CMD_EXTENDED_COMMAND, data[1] };
              if(!ext_cmds[i].updated) {
                  fifo_remove(ff_lg_device[device].fifo, cmd);
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

  memset(ff_lg_device[device].last_report.data, 0x00, sizeof(ff_lg_device[device].last_report.data));
}

/*
 * Convert low-res spring/damper coefficients.
 */
static inline unsigned char convert_coef_lr2lr(unsigned char k) {

    static const unsigned char map[] = { 0, 1, 2, 3, 4, 6, 5, 7 };
    return map[k];
}

/*
 * Convert high-res spring/damper effects to low-res spring/damper effects (with 'old' coefficients).
 */
static inline unsigned char convert_coef_hr2lr(unsigned char k) {

    static const unsigned char hr2lr[16] = {
            [0]  = 0, // 0 => stop effect
            [1]  = 0, // 0.26
            [2]  = 1, // 0.53
            [3]  = 2, // 0.8
            [4]  = 3, // 1.06
            [5]  = 3, // 1.33
            [6]  = 4, // 1.6
            [7]  = 4, // 1.86
            [8]  = 6, // 2.13
            [9]  = 6, // 2.4
            [10] = 6, // 2.66
            [11] = 6, // 2.93
            [12] = 5, // 3.2
            [13] = 5, // 3.46
            [14] = 5, // 3.73
            [15] = 7, // 4
    };
    return hr2lr[k];
}

static int convert_force(int device, s_ff_lg_report * to) {

    *to = ff_lg_device[device].last_report;

    if ((ff_lg_device[device].last_report.data[1] & FF_LG_CMD_MASK) == FF_LG_CMD_STOP) {
        return 1;
    }

    const s_ff_lg_force * force = (s_ff_lg_force *)(ff_lg_device[device].last_report.data + 2);
    s_ff_lg_force * out_force = (s_ff_lg_force *)(to->data + 2);

    switch (force->type) {
    case FF_LG_FTYPE_HIGH_RESOLUTION_SPRING:
        if (ff_lg_device[device].convert_hr2lr) {
            if (FF_LG_HIGHRES_SPRING_K1(force) != 0 || FF_LG_HIGHRES_SPRING_K2(force) != 0) {
                out_force->type = FF_LG_FTYPE_SPRING;
                out_force->parameters[0] = FF_LG_HIGHRES_SPRING_D1(force);
                out_force->parameters[1] = FF_LG_HIGHRES_SPRING_D2(force);
                out_force->parameters[2] = (convert_coef_hr2lr(FF_LG_HIGHRES_SPRING_K2(force)) << 4) | convert_coef_hr2lr(FF_LG_HIGHRES_SPRING_K1(force));
                out_force->parameters[3] = (FF_LG_HIGHRES_SPRING_S2(force) << 4) | FF_LG_HIGHRES_SPRING_S1(force);
                out_force->parameters[4] = FF_LG_HIGHRES_SPRING_CLIP(force);
            } else {
                to->data[1] = (to->data[1] & FF_LG_FSLOT_MASK) | FF_LG_CMD_STOP;
                memset(to->data + 2, 0x00, sizeof(to->data) - 2);
            }
        }
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER:
        if (ff_lg_device[device].convert_hr2lr) {
            if (FF_LG_HIGHRES_DAMPER_K1(force) != 0 || FF_LG_HIGHRES_DAMPER_K2(force) != 0) {
                out_force->type = FF_LG_FTYPE_DAMPER;
                out_force->parameters[0] = convert_coef_hr2lr(FF_LG_HIGHRES_DAMPER_K1(force));
                out_force->parameters[1] = convert_coef_hr2lr(FF_LG_HIGHRES_DAMPER_K2(force));
                out_force->parameters[2] = FF_LG_HIGHRES_DAMPER_S1(force);
                out_force->parameters[3] = FF_LG_HIGHRES_DAMPER_S2(force);
                out_force->parameters[4] = 0;
            } else {
                to->data[1] = (to->data[1] & FF_LG_FSLOT_MASK) | FF_LG_CMD_STOP;
                memset(to->data + 2, 0x00, sizeof(to->data) - 2);
            }
        }
        break;
    case FF_LG_FTYPE_SPRING:
        if (ff_lg_device[device].convert_lr_coef) {
            out_force->type = FF_LG_FTYPE_SPRING;
            out_force->parameters[0] = FF_LG_SPRING_D1(force);
            out_force->parameters[1] = FF_LG_SPRING_D2(force);
            out_force->parameters[2] = (convert_coef_lr2lr(FF_LG_SPRING_K2(force)) << 4) | convert_coef_lr2lr(FF_LG_SPRING_K1(force));
            out_force->parameters[3] = (FF_LG_SPRING_S2(force) << 4) | FF_LG_SPRING_S1(force);
            out_force->parameters[4] = FF_LG_SPRING_CLIP(force);
        }
        break;
    case FF_LG_FTYPE_DAMPER:
        if (ff_lg_device[device].convert_lr_coef) {
            out_force->type = FF_LG_FTYPE_DAMPER;
            out_force->parameters[0] = convert_coef_lr2lr(FF_LG_DAMPER_K1(force));
            out_force->parameters[1] = convert_coef_lr2lr(FF_LG_DAMPER_K2(force));
            out_force->parameters[2] = FF_LG_DAMPER_S1(force);
            out_force->parameters[3] = FF_LG_DAMPER_S2(force);
            out_force->parameters[4] = 0;
        }
        break;
    }

    return 1;
}

int ff_lg_get_report(int device, s_ff_lg_report * report) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return 0;
    }

    int i;
    s_force * forces = ff_lg_device[device].forces;
    unsigned char * data = ff_lg_device[device].last_report.data + 1;

    unsigned char mask = 0;
    // look for forces to stop
    for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
        if (forces[i].updated && !forces[i].active) {
            mask |= forces[i].mask;
        }
    }

    // if multiple forces have to be stopped, then stop them
    if (fslot_nbits[mask >> 4] > 1) {
        clear_report(device);
        data[0] = mask | FF_LG_CMD_STOP;
        for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
            if (mask & forces[i].mask) {
                forces[i].updated = 0;
            }
        }
        dprintf("< %s %02x\n", get_cmd_name(data[0]), data[0]);
        *report = ff_lg_device[device].last_report;
        return 1;
    }

    s_cmd cmd = fifo_peek(ff_lg_device[device].fifo);
    if (cmd.cmd) {
        clear_report(device);
        dprintf("< ");
        if(cmd.cmd != FF_LG_CMD_EXTENDED_COMMAND) {
            if(cmd.cmd & FF_LG_CMD_MASK)
            {
                // not a slot update
                data[0] = cmd.cmd;
                if(gimx_params.debug.ff_lg) {
                    ff_lg_decode_command(data);
                }
                *report = ff_lg_device[device].last_report;
                return 1;
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
                if(gimx_params.debug.ff_lg) {
                    ff_lg_decode_command(data);
                }
                return convert_force(device, report);
            }
        }
        else {
            int i;
            for (i = 0; i < FF_LG_EXT_CMD_NB; ++i) {
                s_ext_cmd * ext_cmd = ff_lg_device[device].ext_cmds + i;
                if(ext_cmd->cmd[1] == cmd.ext) {
                    memcpy(data, ext_cmd->cmd, sizeof(ext_cmd->cmd));
                    if(gimx_params.debug.ff_lg) {
                        ff_lg_decode_extended(data);
                    }
                    ext_cmd->updated = 0;
                    *report = ff_lg_device[device].last_report;
                    return 1;
                }
            }
        }
    }

#ifndef WIN32
      if (ff_lg_device[device].last_report.data[1] != 0) {
          // keep sending something to ensure bandwidth reservation
          for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
              // check if at least one force is running
              if (forces[i].active) {
                  dprintf("keep sending last command\n");
                  *report = ff_lg_device[device].last_report;
                  return 1;
              }
          }
      }
#endif

    return 0;
}

// this table lists the devices that support the "classic format" protocol
static unsigned short ff_lg_wheel_products[] = {
        USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP,
        USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE,
        USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL,
        USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,
        USB_PRODUCT_ID_LOGITECH_G25_WHEEL,
        USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL,
        USB_PRODUCT_ID_LOGITECH_G27_WHEEL,
        USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2,
        USB_PRODUCT_ID_LOGITECH_G29_WHEEL,
};

int ff_lg_is_logitech_wheel(unsigned short vendor, unsigned short product) {

    if(vendor != USB_VENDOR_ID_LOGITECH) {
        return 0;
    }
    unsigned int i;
    for(i = 0; i < sizeof(ff_lg_wheel_products) / sizeof(*ff_lg_wheel_products); ++i) {
        if(ff_lg_wheel_products[i] == product) {
            return 1;
        }
    }
    return 0;
}

typedef struct {
    unsigned char num;
    unsigned char den;
} s_coef;

/*
 * \brief Get the spring or damper force coefficient, normalized to [0..1].
 *
 * \param pid  the pid of the device (needed for some old devices)
 * \param hr   indicates if this is a high res spring or damper
 * \param k    the constant selector
 *
 * \return the force coefficient
 */
static s_coef get_force_coefficient(unsigned short pid, unsigned char hr, unsigned char k) {

    s_coef coef;

    if (hr == 1) {
        coef.num = k;
        coef.den = 0x0F;
    } else {
        if (is_old_lr_coef_wheel(pid)) {
            static const s_coef old_coefs[] = { { 1, 16 }, { 1, 8 }, { 3, 16 }, { 1, 4 }, { 3, 8 }, { 3, 4 }, { 2, 4 }, { 4, 4 } };
            coef = old_coefs[k];
        } else {
            static const s_coef coefs[] = { { 1, 16 }, { 1, 8 }, { 3, 16 }, { 1, 4 }, { 3, 8 }, { 2, 4 }, { 3, 4 }, { 4, 4 } };
            coef = coefs[k];
        }
    }
    return coef;
}

int16_t ff_lg_get_condition_coef(unsigned short pid, unsigned char hr, unsigned char k, unsigned char s) {

    s_coef coef = get_force_coefficient(pid, hr, k);
    int value = (s ? SHRT_MIN : SHRT_MAX) * coef.num / coef.den;
    return CLAMP(SHRT_MIN, value, SHRT_MAX);
}

uint16_t ff_lg_get_spring_deadband(unsigned short pid, unsigned char d, unsigned char dL) {

    uint16_t deadband;
    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP:
    case USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE:
    case USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL:
    case USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2:
        // older than Driving Force Pro
        deadband = d * USHRT_MAX / UCHAR_MAX;
        break;
    default:
        deadband = ((d << 3) | dL) * USHRT_MAX / 0x7FF;
        break;
    }
    return deadband;
}

uint16_t ff_lg_get_damper_clip(unsigned short pid, unsigned char c) {

    uint16_t clip;
    switch(pid) {
    case USB_PRODUCT_ID_LOGITECH_DFP_WHEEL:
        clip = c * USHRT_MAX / UCHAR_MAX;
        break;
    default:
        clip = USHRT_MAX;
        break;
    }
    return clip;
}
