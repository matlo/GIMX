/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#include <gimxhid/include/ghid.h>
#include <gimxuhid/include/guhid.h>
#include <gimxinput/include/ginput.h>
#include <gimxpoll/include/gpoll.h>
#include <gimxtimer/include/gtimer.h>

#include <gimxcommon/test/common.h>
#include <gimxcommon/test/handlers.c>
#include <gimxcommon/test/hid.c>
#include <gimxcommon/test/input.c>
#include <gimxcommon/test/timer.c>

#define PERIOD 10000//microseconds

static int uhid = -1;
static int hid = -1;

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

int hid_read(int user __attribute__((unused)), const void * buf, int status) {

  if (status < 0) {
    set_done();
    return 1;
  }

  int ret = ghid_poll(hid);
  if (ret < 0) {
    set_done();
    return 1;
  }

  if (status > 0) {
    struct timeval t;
    gettimeofday(&t, NULL);
    printf("%ld.%06ld ", t.tv_sec, t.tv_usec);
    printf("%s\n", __func__);
    dump((unsigned char *) buf, status);
    fflush(stdout);
    guhid_write(uhid, buf, status);
  }

  return 0;
}

int hid_close(int user __attribute__((unused))) {
  set_done();
  return 0;
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {

  setup_handlers();

  if (ghid_init() < 0) {
    return -1;
  }

  char * path = hid_select();

  if (path == NULL) {
    fprintf(stderr, "No HID device selected!\n");
    return -1;
  }

  hid = ghid_open_path(path);

  if (hid >= 0) {

    const s_hid_info * hidInfo = ghid_get_hid_info(hid);

    printf("Opened device: VID 0x%04x PID 0x%04x PATH %s\n", hidInfo->vendor_id, hidInfo->product_id, path);

    dump(hidInfo->reportDescriptor, hidInfo->reportDescriptorLength);

    //Create a virtual hid device
    uhid = guhid_create(hidInfo, hid);

    if (uhid >= 0) {

      GPOLL_INTERFACE poll_interface = {
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
      };
      if (ginput_init(&poll_interface, GE_MKB_SOURCE_PHYSICAL, process_event) == 0) {

        display_devices();

        GHID_CALLBACKS ghid_callbacks = {
                .fp_read = hid_read,
                .fp_write = NULL,
                .fp_close = hid_close,
                .fp_register = REGISTER_FUNCTION,
                .fp_remove = REMOVE_FUNCTION,
        };
        if (ghid_register(hid, 42, &ghid_callbacks) != -1) {
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

          int ret = ghid_poll(hid);
          if (ret < 0) {
            set_done();
          }

          while (!is_done()) {

            gpoll();

            //do something periodically

          }

          if (timer >= 0) {
            gtimer_close(timer);
          }

        }

        ginput_quit();
      }

      ghid_close(hid);
    }

    guhid_close(uhid);
  }

  ghid_exit();

  free(path);

  printf("Exiting\n");

  return 0;
}
