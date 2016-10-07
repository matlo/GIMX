/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include "common.h"

#define PERIOD 10000//microseconds
#define HAPTIC_DURATION 1000//periods
#define HAPTIC_DELAY 100//periods

static int nb_joystick = 0;
static int joystick = -1;
static int haptic = -1;

void display_haptic() {

    const char * name;
    int i = 0;
    printf("Joysticks:\n");
    while ((name = ginput_joystick_name(i))) {
        printf("  %d %s(%d) and has haptic effects: {", i, name, ginput_joystick_virtual_id(i));
        int effects = ginput_joystick_get_haptic(i);
        if (effects == GE_HAPTIC_NONE) {
            printf(" none");
        }
        if (effects & GE_HAPTIC_RUMBLE) {
            printf(" rumble,");
        }
        if (effects & GE_HAPTIC_CONSTANT) {
            printf(" constant,");
        }
        if (effects & GE_HAPTIC_SPRING) {
            printf(" spring,");
        }
        if (effects & GE_HAPTIC_DAMPER) {
            printf(" damper,");
        }
        printf(" }\n");
        ++i;
    }

    nb_joystick = i;

    if (!i) {
        printf("  none\n");
    }

    fflush(stdout);
}

int js_select() {

    printf("Select the joystick to test: ");
    fflush(stdout);

    if (scanf("%d", &joystick) != 1 || joystick < 0 || joystick >= nb_joystick) {
        fprintf(stderr, "Invalid choice.\n");
        return -1;
    }

    haptic = ginput_joystick_get_haptic(joystick);
    if (haptic < 0 || haptic == GE_HAPTIC_NONE) {
        fprintf(stderr, "Joystick has no haptic capabilities.\n");
        return -1;
    }

    return 0;
}

#define START(MSG,EVT) \
        printf(MSG); \
        fflush(stdout); \
        counter_max = HAPTIC_DURATION; \
        ++step; \
        ginput_joystick_set_haptic(EVT);

#define STOP(MSG,HAPTIC,EVT) \
        printf(MSG); \
        fflush(stdout); \
        haptic &= ~HAPTIC; \
        counter_max = HAPTIC_DELAY; \
        step = 0; \
        ginput_joystick_set_haptic(EVT);

void haptic_task() {

    static int step = 0;
    static int counter = 0;
    static int counter_max = 0;

    if (counter_max != 0) {
        ++counter;
        if (counter < counter_max) {
            return;
        }
        counter = 0;
        counter_max = 0;
    }

    GE_Event event = {
            .which = joystick,
    };

    if (haptic & GE_HAPTIC_RUMBLE) {
        event.jrumble.type = GE_JOYRUMBLE;
        switch (step) {
        case 0:
            event.jrumble.weak = USHRT_MAX / 2;
            START("Playing weak rumble effect.\n", &event)
            break;
        case 1:
            event.jrumble.strong = USHRT_MAX / 2;
            START("Playing strong rumble effect.\n", &event)
            break;
        case 2:
            STOP("Stop rumble effect.\n", GE_HAPTIC_RUMBLE, &event)
            break;
        }
        return;
    }
    if (haptic & GE_HAPTIC_CONSTANT) {
        event.jconstant.type = GE_JOYCONSTANTFORCE;
        switch (step) {
        case 0:
            event.jconstant.level = SHRT_MAX / 4;
            START("Playing positive constant force.\n", &event)
            break;
        case 1:
            event.jconstant.level = SHRT_MIN / 4;
            START("Playing negative constant force.\n", &event)
            break;
        case 2:
            STOP("Stop constant force.\n", GE_HAPTIC_CONSTANT, &event)
            break;
        }
        return;
    }
    if (haptic & GE_HAPTIC_SPRING) {
        event.jcondition.type = GE_JOYSPRINGFORCE;
        event.jcondition.saturation.left = USHRT_MAX;
        event.jcondition.saturation.right = USHRT_MAX;
        switch (step) {
        case 0:
            event.jcondition.coefficient.right = SHRT_MAX / 2;
            START("Playing right spring force.\n", &event)
            break;
        case 1:
            event.jcondition.coefficient.left = SHRT_MAX / 2;
            START("Playing left spring force.\n", &event)
            break;
        case 2:
            STOP("Stop spring force.\n", GE_HAPTIC_SPRING, &event)
            break;
        }
        return;
    }
    if (haptic & GE_HAPTIC_DAMPER) {
        event.jcondition.type = GE_JOYDAMPERFORCE;
        event.jcondition.saturation.left = USHRT_MAX;
        event.jcondition.saturation.right = USHRT_MAX;
        switch (step) {
        case 0:
            event.jcondition.coefficient.right = SHRT_MAX / 2;
            START("Playing right damper force.\n", &event)
            break;
        case 1:
            event.jcondition.coefficient.left = SHRT_MAX / 2;
            START("Playing left damper force.\n", &event)
            break;
        case 2:
            STOP("Stop damper force.\n", GE_HAPTIC_DAMPER, &event)
            break;
        }
        return;
    }

    if (haptic == GE_HAPTIC_NONE) {
        set_done();
    }
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {

    setup_handlers();

    GPOLL_INTERFACE poll_interface = {
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION
    };
    if (ginput_init(&poll_interface, GE_MKB_SOURCE_NONE, ignore_event) < 0) {
        exit(-1);
    }

    display_haptic();

    if (js_select()) {
        exit(-1);
    }

    GTIMER_CALLBACKS timer_callbacks = {
            .fp_read = timer_read,
            .fp_close = timer_close,
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    int timer = gtimer_start(42, PERIOD, &timer_callbacks);
    if (timer < 0) {
        set_done();
    }

    while (!is_done()) {

        gpoll();

        ginput_periodic_task();

        haptic_task();
    }

    if (timer >= 0) {
        gtimer_close(timer);
    }

    ginput_quit();

    printf("Exiting\n");
    fflush(stdout);

    return 0;
}
