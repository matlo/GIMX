/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#include <gimxhid/include/ghid.h>
#include <gimxpoll/include/gpoll.h>
#include <gimxtimer/include/gtimer.h>

#include <gimxcommon/test/common.h>
#include <gimxcommon/test/handlers.c>
#include <gimxcommon/test/hid.c>
#include <gimxcommon/test/timer.c>

#define PERIOD 5000 //microseconds
#define RUMBLE_PERIOD 1000000 //microseconds
#define FF_PERIOD 80000 //microseconds

typedef struct {
  unsigned short length;
  unsigned char data[65];
} s_hid_packet;

static struct {
  unsigned short vid;
  unsigned short pid;
  s_hid_packet start;
  s_hid_packet stop;
} rumble_cmds[] = {
    {
        .vid = 0x046d,
        .pid = 0xc218,
        .start = { 8, { 0x00, 0x51, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x00 } },
        .stop =  { 2, { 0x00, 0xf3 } },
    },
    {
        .vid = 0x054c,
        .pid = 0x05c4,
        .start = { 9, { 0x05, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00 } },
        .stop =  { 9, { 0x05, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00 } },
    }
};

static int rumble_index = -1;

static struct {
  unsigned short vid;
  unsigned short pid;
  s_hid_packet stop;
  s_hid_packet left;
  s_hid_packet right;
} ff_cmds[] = {
    {
        .vid = 0x046d,
        .pid = 0xc29a,
        .stop = { 8, { 0x00, 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // stop all forces
        .left  = { 8, { 0x00, 0x11, 0x08, 0x60, 0x80, 0x00, 0x00, 0x00 } },
        .right = { 8, { 0x00, 0x11, 0x08, 0xa0, 0x80, 0x00, 0x00, 0x00 } },
    },
};

static int ff_index = -1;

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

  if (is_done()) {
    return -1;
  }

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
  }

  return 0;
}

static int hid_busy = 0;

static int counter = 0;

void rumble_task(int device) {

  if(rumble_index < 0) {
    return;
  }

  static int rumble = 0;

  if (!hid_busy) {

    if (counter >= RUMBLE_PERIOD / PERIOD) {
      if(rumble) {
        printf("Stop rumble\n");
      } else {
        printf("Start rumble\n");
      }
      fflush(stdout);
      rumble = !rumble;
      counter = 0;
    }

    if(rumble) {
      ghid_write(device, rumble_cmds[rumble_index].start.data, rumble_cmds[rumble_index].start.length);
    } else  {
      ghid_write(device, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length);
    }
    hid_busy = 1;
  }
}

void ff_task(int device) {

  if(ff_index < 0) {
    return;
  }

  static int ff_play = 0;
  static int ff_dir = 0;

  if (!hid_busy) {

    if (counter >= RUMBLE_PERIOD / PERIOD) {
      if(ff_play) {
        printf("Stop rumble\n");
      } else {
        printf("Start rumble\n");
      }
      fflush(stdout);
      ff_play = !ff_play;
      counter = 0;
    }

    if(ff_play) {
      static int cpt = 0;
      if(ff_dir) {
        ghid_write(device, ff_cmds[ff_index].left.data, ff_cmds[ff_index].left.length);
      } else {
        ghid_write(device, ff_cmds[ff_index].right.data, ff_cmds[ff_index].right.length);
      }
      ++cpt;
      if(cpt == FF_PERIOD / PERIOD) {
        ff_dir = !ff_dir;
        cpt = 0;
      }
    } else  {
      ghid_write(device, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length);
    }
    hid_busy = 1;
  }
}

void hid_task(int device) {

  if(is_done()) {
    return;
  }
  rumble_task(device);
  ff_task(device);
}

int hid_write(int user __attribute__((unused)), int transfered __attribute__((unused))) {

  /*struct timeval t;
  gettimeofday(&t, NULL);
  printf("%ld.%06ld %s\n", t.tv_sec, t.tv_usec, __func__);*/
  hid_busy = 0;
  hid_task(hid);
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

  if(path == NULL) {
    fprintf(stderr, "No HID device selected!\n");
    exit(-1);
  }

  hid = ghid_open_path(path);

  if (hid >= 0) {

    const s_hid_info * hid_info = ghid_get_hid_info(hid);

    printf("Opened device: VID 0x%04x PID 0x%04x PATH %s\n", hid_info->vendor_id, hid_info->product_id, path);

    unsigned int i;
    for (i = 0; i < sizeof(rumble_cmds) / sizeof(*rumble_cmds); ++i) {
      if(rumble_cmds[i].vid == hid_info->vendor_id && rumble_cmds[i].pid == hid_info->product_id) {
        rumble_index = i;
      }
    }
    for (i = 0; i < sizeof(ff_cmds) / sizeof(*ff_cmds); ++i) {
      if(ff_cmds[i].vid == hid_info->vendor_id && ff_cmds[i].pid == hid_info->product_id) {
        ff_index = i;
      }
    }

    GHID_CALLBACKS ghid_callbacks = {
            .fp_read = hid_read,
            .fp_write = hid_write,
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

      hid_task(hid);

      int ret = ghid_poll(hid);
      if (ret < 0) {
        set_done();
      }

      while (!is_done() || hid_busy) {

        gpoll();

        ++counter;
      }

      if (timer >= 0) {
        gtimer_close(timer);
      }

      if(rumble_index >= 0) {
        ghid_write_timeout(hid, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length, 1000);
      }

      if(ff_index >= 0) {
        ghid_write_timeout(hid, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length, 1000);
      }

      ghid_close(hid);
    }
  }

  ghid_exit();

  free(path);

  printf("Exiting\n");

  return 0;
}
