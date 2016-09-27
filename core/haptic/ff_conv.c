/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include <haptic/ff_lg.h>
#include <gimx.h>
#include <string.h>

#define CLAMP(MIN,VALUE,MAX) (((VALUE) < MIN) ? (MIN) : (((VALUE) > MAX) ? (MAX) : (VALUE)))

static inline short u8_to_s16(unsigned char c) {
    int value = (c + CHAR_MIN) * SHRT_MAX / CHAR_MAX;
    return CLAMP(SHRT_MIN, value, SHRT_MAX);
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

    return 0;
}

static int ff_conv_lg_force(int device, unsigned int index, GE_Event * event) {

    s_ff_lg_force * force = &ff_lg_device[device].slots[index].ff_lg_force;

    switch (force->type) {
    case FF_LG_FTYPE_CONSTANT:
        event->type = GE_JOYCONSTANTFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jconstant.level = u8_to_s16(force->constant.levels[index]);
        }
        return 1;
    case FF_LG_FTYPE_VARIABLE:
        event->type = GE_JOYCONSTANTFORCE;
        if (ff_lg_device[device].slots[index].active) {
            static int warned = 0;
            if (index == 0) {
                if (warned == 0 && force->variable.t1 && force->variable.s1) {
                    gprintf("warning: variable force cannot be converted to constant force (l1=%hu, t1=%hu, s1=%hu, d1=%hu\n",
                        force->variable.l1, force->variable.t1, force->variable.s1, force->variable.d1);
                    warned = 1;
                } else {
                    event->jconstant.level = u8_to_s16(force->variable.l1);
                }
            } else if (index == 2) {
                if (warned == 0 && force->variable.t2 && force->variable.s2) {
                    gprintf("warning: variable force cannot be converted to constant force (l2=%hu, t2=%hu, s2=%hu, d2=%hu\n",
                        force->variable.l2, force->variable.t2, force->variable.s2, force->variable.d2);
                    warned = 1;
                } else {
                    event->jconstant.level = u8_to_s16(force->variable.l2);
                }
            }
        }
        return 1;
    case FF_LG_FTYPE_SPRING:
        event->type = GE_JOYSPRINGFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = u8_to_u16(force->spring.clip);
            event->jcondition.saturation.right = u8_to_u16(force->spring.clip);
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, force->spring.k1, force->spring.s1);
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, force->spring.k2, force->spring.s2);
            event->jcondition.center = u8_to_s16((force->spring.d1 + force->spring.d2) / 2);
            event->jcondition.deadband = u8_to_u16(force->spring.d2 - force->spring.d1);
        }
        return 1;
    case FF_LG_FTYPE_DAMPER:
        event->type = GE_JOYDAMPERFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, force->damper.k1, force->damper.s1);
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 0, force->damper.k2, force->damper.s2);
        }
        return 1;
    case FF_LG_FTYPE_HIGH_RESOLUTION_SPRING:
        event->type = GE_JOYSPRINGFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = u8_to_u16(force->hr_spring.clip);
            event->jcondition.saturation.right = u8_to_u16(force->hr_spring.clip);
            event->jcondition.coefficient.left = 
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, force->hr_spring.k1, force->hr_spring.s1);
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, force->hr_spring.k2, force->hr_spring.s2);
            uint16_t d2 = ff_lg_get_spring_deadband(ff_lg_device[device].pid, force->hr_spring.d2, force->hr_spring.d2L);
            uint16_t d1 = ff_lg_get_spring_deadband(ff_lg_device[device].pid, force->hr_spring.d1, force->hr_spring.d1L);
            event->jcondition.center = u16_to_s16((d1 + d2) / 2);
            event->jcondition.deadband = d2 - d1;
        }
        return 1;
    case FF_LG_FTYPE_HIGH_RESOLUTION_DAMPER:
        event->type = GE_JOYDAMPERFORCE;
        if (ff_lg_device[device].slots[index].active) {
            event->jcondition.saturation.left = ff_lg_get_damper_clip(ff_lg_device[device].pid, force->hr_damper.clip);
            event->jcondition.saturation.right = ff_lg_get_damper_clip(ff_lg_device[device].pid, force->hr_damper.clip);
            event->jcondition.coefficient.left =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, force->hr_damper.k1, force->hr_damper.s1);
            event->jcondition.coefficient.right =
                ff_lg_get_condition_coef(ff_lg_device[device].pid, 1, force->hr_damper.k2, force->hr_damper.s2);
        }
        return 1;
    default:
        //TODO MLA: other force types
        return 0;
    }
}

int ff_conv(int device, const unsigned char data[FF_LG_OUTPUT_REPORT_SIZE], GE_Event events[FF_LG_FSLOTS_NB]) {

    CHECK_DEVICE(device, -1)

    if(gimx_params.debug) {
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

    int event_nb = 0;
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
                        memset(&slots[i].ff_lg_force, 0x00, sizeof(slots[i].ff_lg_force));
                    } else {
                        continue;
                    }
                    event_nb += ff_conv_lg_force(device, i, events + event_nb);
                }
            }
        }
        break;
        }
    }

    return event_nb;
 }
