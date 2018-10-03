/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <haptic/common/ff_lg.h>
#include <haptic/haptic_common.h>
#include <haptic/haptic_source.h>

struct haptic_source_state {
    uint8_t caps;
    uint8_t cmd_offset;
    struct {
        int updated;
        uint16_t value; // the current wheel range (0 means unknown)
    } range;

    // TODO MLA
    struct {
        int updated;
        int16_t level;
    } constant;
};

struct haptic_source_state * init(s_haptic_core_ids ids __attribute__((unused))) {

    void * ptr = calloc(1, sizeof(struct haptic_source_state));

    if (ptr == NULL) {
        PRINT_ERROR_ALLOC_FAILED("calloc")
        return NULL;
    }

    struct haptic_source_state * state = (struct haptic_source_state *) ptr;

    // TODO MLA

    return state;
}

static void clean(struct haptic_source_state * state) {

    free(state);
}

static void dump2(const unsigned char * packet, unsigned char length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    if (i > 0) {
      ginfo(":");
    }
    ginfo("%02x", packet[i]);
  }
  ginfo("\n");
}

#define DEBUG_PACKET2(STR, DATA, LENGTH) \
  if(gimx_params.debug.haptic) \
  { \
    ginfo("%s", STR); \
    dump2(DATA, LENGTH); \
  }

static void process(struct haptic_source_state * state, size_t size, const unsigned char * data) {

    const unsigned char * ptr = data + state->cmd_offset;

    if(ptr[4] == 0x02) {
        DEBUG_PACKET2("> ", data, size)
    }

    if (ptr[0] == 0x0a && ptr[1] == 0x00 && ptr[4] == 0x01) {
        int16_t value = ptr[6] << 8 | ptr[5];
        if (value != state->range.value) {
            state->range.value = value;
            state->range.updated = 1;
            dprintf("> range: %hu\n", value);
        }
    } else if (ptr[0] == 0x0b && ptr[1] == 0x00) {
        ptr += 6;
        while (ptr + 6 < data + size) {
            if (ptr[1] == 0x01) {
                int16_t level = (ptr[5] & 0x80 ? 1 : -1) * ptr[4];
                if (level != state->constant.level) {
                    state->constant.level = level;
                    state->constant.updated = 1;
                    dprintf("> constant: %d\n", state->constant.level);
                }
            }
            ptr += 6;
        }
    }

    // TODO MLA
}

static int get(struct haptic_source_state * state __attribute__((unused)), s_haptic_core_data * data __attribute__((unused))) {

    if (state->range.updated) {
        data->type = E_DATA_TYPE_RANGE;
        data->range.value = state->range.value;
        state->range.updated = 0;
        return 1;
    }

    // TODO MLA
    if (state->constant.updated) {
        data->type = E_DATA_TYPE_CONSTANT;
        data->constant.level = state->constant.level;
        state->constant.updated = 0;
        return 1;
    }

    return 0;
}

static s_haptic_core_ids ids[] = {
        { .vid = USB_VENDOR_ID_LOGITECH, .pid = USB_PRODUCT_ID_LOGITECH_G920_XONE_WHEEL },
        { .vid = 0x0000,                 .pid = 0x0000                                  }, // end of table
};

static s_haptic_source source = {
        .name = "haptic_source_g920",
        .ids = ids,
        .init = init,
        .clean = clean,
        .process = process,
        .get = get
};

void haptic_source_g920_constructor(void) __attribute__((constructor));
void haptic_source_g920_constructor(void) {

    if (haptic_source_register(&source) == -1) {
        PRINT_ERROR_OTHER("failed to register source")
    }
}
