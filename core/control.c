/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "control.h"
#include "gimx.h"
#include <gimxinput/include/ginput.h>
#include "mainloop.h"

static int lshift = 0;
static int rshift = 0;
static int lalt = 0;
static int ralt = 0;

/*
 * \brief Use special keys to control GIMX.
 *
 * \param sym  the key
 * \param down the key status, 1 for down, 0 for up
 */
void control_key(int sym, int down) {

    switch (sym) {
    case GE_KEY_ESC:
        if (lshift && gimx_params.config_file != NULL) {
            set_done();
        }
        break;
    case GE_KEY_LEFTSHIFT:
        lshift = down ? 1 : 0;
        break;
    case GE_KEY_RIGHTSHIFT:
        rshift = down ? 1 : 0;
        break;
    case GE_KEY_LEFTALT:
        lalt = down ? 1 : 0;
        break;
    case GE_KEY_RIGHTALT:
        ralt = down ? 1 : 0;
        break;
    }

    if (lshift && rshift) {
        if (gimx_params.status) {
            gimx_params.status = 0;
        } else {
            if (!gimx_params.curses) {
                gimx_params.status = 1;
            }
        }
    }

    if (lalt && ralt && gimx_params.config_file) {
        gimx_params.grab = ginput_grab_toggle();
    }
}
