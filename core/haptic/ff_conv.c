/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include <haptic/ff_lg.h>
#include <haptic/ff_common.h>
#include <gimx.h>
#include <string.h>

#undef dprintf
#define dprintf(...) if(gimx_params.debug.ff_conv) printf(__VA_ARGS__)

#define CLAMP(MIN,VALUE,MAX) (((VALUE) < MIN) ? (MIN) : (((VALUE) > MAX) ? (MAX) : (VALUE)))

static inline short u8_to_s16(unsigned char c) {
    int value = (c + CHAR_MIN) * SHRT_MAX / CHAR_MAX;
    return CLAMP(-SHRT_MAX, value, SHRT_MAX);
    }

static inline unsigned short u8_to_u16(unsigned char c) {
    return c * USHRT_MAX / UCHAR_MAX;
}

static inline short u16_to_s16(unsigned short s) {
    return s + SHRT_MIN;
}

typedef struct {
    unsigned char mask;
    unsigned char active;
    s_ff_lg_force ff_lg_force;
} s_slot;

static struct
{
    unsigned short pid;
    s_slot slots[FF_LG_FSLOTS_NB];
    s_cmd fifo[FIFO_SIZE];
    unsigned short range;
    GE_Event last_event;
    int invert;
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

void ff_conv_init_static(void) __attribute__((constructor));
void ff_conv_init_static(void)
{
    unsigned int i, j;
    for (i = 0; i < MAX_CONTROLLERS; ++i) {
        for(j = 0; j < FF_LG_FSLOTS_NB; ++j) {
            ff_lg_device[i].slots[j].mask = 0x10 << j;
        }
    }
}

int ff_conv_init(int device, unsigned short pid) {

    CHECK_DEVICE(device, -1)

    ff_lg_device[device].pid = pid;

    ff_lg_device[device].invert = 0;

    unsigned short range = ff_lg_get_wheel_range(pid);
    if (range != 0) {
        ginfo("adjust your wheel range to %u degrees\n", range);
    }

    return 0;
}

static void dump_event(const GE_Event * event) {

    switch (event->type) {
    case GE_JOYCONSTANTFORCE:
        dprintf("< CONSTANT, level: %d\n", event->jconstant.level);
        fflush(stdout);
        break;
    case GE_JOYSPRINGFORCE:
        dprintf("< SPRING, saturation: %u %u, coefficient: %u %u, center: %d, deadband: %u\n",
                event->jcondition.saturation.left, event->jcondition.saturation.right,
                event->jcondition.coefficient.left, event->jcondition.coefficient.right,
                event->jcondition.center, event->jcondition.deadband);
        fflush(stdout);
        break;
    case GE_JOYDAMPERFORCE:
        dprintf("< DAMPER, saturation: %u %u, coefficient: %d %d\n",
                event->jcondition.saturation.left, event->jcondition.saturation.right,
                event->jcondition.coefficient.left, event->jcondition.coefficient.right);
        fflush(stdout);
        break;
    default:
        dprintf("< UNKNOWN\n");
        fflush(stdout);
        break;
    }
}

#define SWAP(TYPE, V1, V2) \
        TYPE tmp = V1; \
        V1 = V2; \
        V2 = tmp;

static int ff_conv_lg_force(int device, unsigned int index, GE_Event * event) {

    int ret = 0;

    s_ff_lg_force * force = &ff_lg_device[device].slots[index].ff_lg_force;

    memset(event, 0x00, sizeof(*event));

    switch (force->type) {
    case FF_LG_FTYPE_CONSTANT:
        event->type = GE_JOYCONSTANTFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jconstant.level = u8_to_s16(FF_LG_CONSTANT_LEVEL(force, index));
            if (ff_lg_device[device].invert) {
                event->jconstant.level = -event->jconstant.level;
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_VARIABLE:
        event->type = GE_JOYCONSTANTFORCE;
        if (ff_lg_device[device].slots[index].active) {
            static int warned = 0;
            if (index == 0) {
                if (FF_LG_VARIABLE_T1(force) && FF_LG_VARIABLE_S1(force)) {
                    if (warned == 0) {
                        gwarn("variable force cannot be converted to constant force (l1=%hu, t1=%hu, s1=%hu, d1=%hu\n",
                            FF_LG_VARIABLE_L1(force), FF_LG_VARIABLE_T1(force), FF_LG_VARIABLE_S1(force), FF_LG_VARIABLE_D1(force));
                        warned = 1;
                    }
                } else {
                    event->jconstant.level = u8_to_s16(FF_LG_VARIABLE_L1(force));
                    if (ff_lg_device[device].invert) {
                        event->jconstant.level = -event->jconstant.level;
                    }
                }
            } else if (index == 2) {
                if (FF_LG_VARIABLE_T2(force) && FF_LG_VARIABLE_S2(force)) {
                    if (warned == 0) {
                        gwarn("variable force cannot be converted to constant force (l2=%hu, t2=%hu, s2=%hu, d2=%hu\n",
                            FF_LG_VARIABLE_L2(force), FF_LG_VARIABLE_T2(force), FF_LG_VARIABLE_S2(force), FF_LG_VARIABLE_D2(force));
                        warned = 1;
                    }
                } else {
                    event->jconstant.level = u8_to_s16(FF_LG_VARIABLE_L2(force));
                    if (ff_lg_device[device].invert) {
                        event->jconstant.level = -event->jconstant.level;
                    }
                }
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_SPRING:
        event->type = GE_JOYSPRINGFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = u8_to_u16(FF_LG_SPRING_CLIP(force));
            event->jcondition.saturation.right = u8_to_u16(FF_LG_SPRING_CLIP(force));
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, FF_LG_SPRING_K1(force), FF_LG_SPRING_S1(force));
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, FF_LG_SPRING_K2(force), FF_LG_SPRING_S2(force));
            event->jcondition.center = u8_to_s16((FF_LG_SPRING_D1(force) + FF_LG_SPRING_D2(force)) / 2);
            event->jcondition.deadband = u8_to_u16(FF_LG_SPRING_D2(force) - FF_LG_SPRING_D1(force));
            if (ff_lg_device[device].invert) {
                SWAP(int16_t, event->jcondition.coefficient.left, event->jcondition.coefficient.right)
                event->jcondition.center = -event->jcondition.center;
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_DAMPER:
        event->type = GE_JOYDAMPERFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, FF_LG_DAMPER_K1(force), FF_LG_DAMPER_S1(force));
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, FF_LG_DAMPER_K2(force), FF_LG_DAMPER_S2(force));
            if (ff_lg_device[device].invert) {
                SWAP(int16_t, event->jcondition.coefficient.left, event->jcondition.coefficient.right)
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_SPRING:
        event->type = GE_JOYSPRINGFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = u8_to_u16(FF_LG_HIGHRES_SPRING_CLIP(force));
            event->jcondition.saturation.right = u8_to_u16(FF_LG_HIGHRES_SPRING_CLIP(force));
            event->jcondition.coefficient.left = 
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, FF_LG_HIGHRES_SPRING_K1(force), FF_LG_HIGHRES_SPRING_S1(force));
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, FF_LG_HIGHRES_SPRING_K2(force), FF_LG_HIGHRES_SPRING_S2(force));
            uint16_t d2 = ff_lg_get_spring_deadband(ff_lg_device[device].pid, FF_LG_HIGHRES_SPRING_D2(force), FF_LG_HIGHRES_SPRING_D2L(force));
            uint16_t d1 = ff_lg_get_spring_deadband(ff_lg_device[device].pid, FF_LG_HIGHRES_SPRING_D1(force), FF_LG_HIGHRES_SPRING_D1L(force));
            event->jcondition.center = u16_to_s16((d1 + d2) / 2);
            event->jcondition.deadband = d2 - d1;
            if (ff_lg_device[device].invert) {
                SWAP(int16_t, event->jcondition.coefficient.left, event->jcondition.coefficient.right)
                event->jcondition.center = -event->jcondition.center;
            }
        }
        ret = 1;
        break;
    case FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER:
        event->type = GE_JOYDAMPERFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = ff_lg_get_damper_clip(ff_lg_device[device].pid, FF_LG_HIGHRES_DAMPER_CLIP(force));
            event->jcondition.saturation.right = ff_lg_get_damper_clip(ff_lg_device[device].pid, FF_LG_HIGHRES_DAMPER_CLIP(force));
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, FF_LG_HIGHRES_DAMPER_K1(force), FF_LG_HIGHRES_DAMPER_S1(force));
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, FF_LG_HIGHRES_DAMPER_K2(force), FF_LG_HIGHRES_DAMPER_S2(force));
            event->jcondition.center = 0;
            event->jcondition.deadband = 0;
            if (ff_lg_device[device].invert) {
                SWAP(int16_t, event->jcondition.coefficient.left, event->jcondition.coefficient.right)
            }
        }
        ret = 1;
        break;
    default:
        //TODO MLA: other force types
        {
            static int warned[0x0F] = {};
            if (force->type < sizeof(warned) / sizeof(*warned) && warned[force->type] == 0) {
                gwarn("unsupported force type: %s\n", ff_lg_get_ftype_name(force->type));
                fflush(stdout);
                warned[force->type] = 1;
            }
        }
        break;
    }

    if(gimx_params.debug.ff_conv && ret != 0) {
        dump_event(event);
    }

    return ret;
}

void ff_conv_process_report(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    CHECK_DEVICE(device,)

    if(gimx_params.debug.ff_conv) {
        dprintf("> ");
        if(data[0] == FF_LG_CMD_EXTENDED_COMMAND) {
            ff_lg_decode_extended(data);
        }
        else {
            ff_lg_decode_command(data);
        }
    }

    uint8_t fslots = data[0] & FF_LG_FSLOT_MASK;
    uint8_t cmd = data[0] & FF_LG_CMD_MASK;

    unsigned int i;

    s_slot * slots = ff_lg_device[device].slots;

    if (data[0] != FF_LG_CMD_EXTENDED_COMMAND) {

        switch(cmd)
        {
        case FF_LG_CMD_DOWNLOAD:
        case FF_LG_CMD_DOWNLOAD_AND_PLAY:
        case FF_LG_CMD_PLAY:
        case FF_LG_CMD_STOP:
        case FF_LG_CMD_REFRESH_FORCE:
        {
            for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
                if (fslots & slots[i].mask) {
                    if (cmd == FF_LG_CMD_DOWNLOAD) {
                        memcpy(&slots[i].ff_lg_force, data + 1, sizeof(slots[i].ff_lg_force));
                        continue; // don't send anything yet
                    } else if (cmd == FF_LG_CMD_DOWNLOAD_AND_PLAY || cmd == FF_LG_CMD_REFRESH_FORCE) {
                        if(slots[i].active && !memcmp(&slots[i].ff_lg_force, data + 1, sizeof(slots[i].ff_lg_force))) {
                            dprintf("> no change\n");
                            continue; // no change
                        }
                        slots[i].active = 1;
                        memcpy(&slots[i].ff_lg_force, data + 1, sizeof(slots[i].ff_lg_force));
                    } else if (cmd == FF_LG_CMD_PLAY) {
                        if(slots[i].active) {
                            dprintf("> already playing\n");
                            continue; // already playing
                        }
                        slots[i].active = 1;
                    } else if (cmd == FF_LG_CMD_STOP) {
                        if(!slots[i].active) {
                            dprintf("> already stopped\n");
                            continue; // already stopped
                        }
                        slots[i].active = 0;
                        memset(&slots[i].ff_lg_force.parameters, 0x00, sizeof(slots[i].ff_lg_force.parameters));
                    } else {
                        continue;
                    }
                    s_cmd cmd = { slots[i].mask, 0x00 };
                    fifo_push(ff_lg_device[device].fifo, cmd, 1);
                }
            }
        }
        break;
        }
    } else {
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
        if (range > 0 && range != ff_lg_device[device].range) {
            ginfo("adjust your wheel range to %u degrees\n", range);
            ff_lg_device[device].range = range;
        }
    }
}

static unsigned char slot_index[] = { [FF_LG_FSLOT_1 >> 4] = 0, [FF_LG_FSLOT_2 >> 4] = 1, [FF_LG_FSLOT_3 >> 4] = 2, [FF_LG_FSLOT_4 >> 4] = 3, };

int ff_conv_get_event(int device, GE_Event * event) {

    if (device < 0 || device >= MAX_CONTROLLERS) {
        fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
        return -1;
    }

    s_cmd cmd = fifo_peek(ff_lg_device[device].fifo);
    if (cmd.cmd) {
        unsigned char index = slot_index[cmd.cmd >> 4];
        int ret = ff_conv_lg_force(device, index, event);
        if (ret == 1) {
            ff_lg_device[device].last_event = *event;
            return 0;
        }
    }

#ifndef WIN32
    if (ff_lg_device[device].last_event.type != GE_NOEVENT && event->type != ff_lg_device[device].last_event.type) {
        // keep sending something to ensure bandwidth reservation
        int i;
        for (i = 0; i < FF_LG_FSLOTS_NB; ++i) {
            // check if at least one force is running
            if (ff_lg_device[device].slots[i].active) {
                dprintf("keep sending last event\n");
                *event = ff_lg_device[device].last_event;
                return 0;
            }
        }
    }
#endif

    return -1;
}

void ff_conv_ack(int device) {

  if (device < 0 || device >= MAX_CONTROLLERS) {
      fprintf(stderr, "%s:%d %s: invalid device (%d)", __FILE__, __LINE__, __func__, device);
      return;
  }

  dprintf("> ack\n");

  s_cmd cmd = fifo_peek(ff_lg_device[device].fifo);
  fifo_remove(ff_lg_device[device].fifo, cmd);
}

int ff_conv_set_tweaks(int device, int invert) {

    CHECK_DEVICE(device, -1)

    ginfo("FFB invert: %s\n", invert ? "yes" : "no");

    ff_lg_device[device].invert = invert;

    return 0;
}
