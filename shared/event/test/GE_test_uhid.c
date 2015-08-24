/*
 * linux_test.c
 *
 *  Created on: 13 janv. 2013
 *      Author: matlo
 */

#include <GE.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <hidasync.h>
#include <uhidasync.h>
#include "common.h"

#define DEVICE_VID  0x046d
#define DEVICE_PID  0xca03

/*#define DEVICE_VID  0x046d
#define DEVICE_PID  0xc293*/

#define PERIOD 10000//microseconds

#ifdef WIN32
#define REGISTER_FUNCTION GE_AddSourceHandle
#else
#define REGISTER_FUNCTION GE_AddSource
#endif

static int uhid = -1;

static void terminate(int sig) {
    done = 1;
}

static void dump(const unsigned char * packet, unsigned char length) {
    int i;
    for (i = 0; i < length; ++i) {
        if (i && !(i % 8)) {
            printf("\n");
        }
        printf("0x%02x ", packet[i]);
    }
    printf("\n");
}

int hid_read(int user, const void * buf, unsigned int count) {
    printf("read user: %d\n", user);
    dump((unsigned char *) buf, count);
    uhidasync_write(uhid, buf, count);
    return 0;
}

int hid_close(int user) {
    printf("close user: %d\n", user);
    return 0;
}

int main(int argc, char* argv[]) {

    (void) signal(SIGINT, terminate);

#ifndef WIN32
    setlinebuf(stdout);
#endif

    //Open Logitech Momo racing
    int hid = hidasync_open_ids(DEVICE_VID, DEVICE_PID);

    if (hid >= 0) {

        const s_hid_info * hidInfo = hidasync_get_hid_info(hid);
        dump(hidInfo->reportDescriptor, hidInfo->reportDescriptorLength);

        //Create a virtual hid device
        uhid = uhidasync_create(hidInfo);

        if (uhid >= 0) {

            if (GE_initialize(GE_MKB_SOURCE_NONE)) {

                display_devices();

                if (hidasync_register(hid, 42, hid_read, NULL, hid_close, REGISTER_FUNCTION) != -1) {

                    GE_SetCallback(process_event);

                    GE_TimerStart(PERIOD);

                    while (!done) {

                        GE_PumpEvents();

                        //do something periodically

                    }

                    GE_TimerClose();

                }
            } else {
                fprintf(stderr, "GE_initialize failed\n");
                exit(-1);
            }

            hidasync_close(hid);
        }

        uhidasync_close(uhid);

    } else {

        fprintf(stderr, "hid device not found\n");
    }

    GE_quit();

    printf("Exiting\n");

    return 0;
}
