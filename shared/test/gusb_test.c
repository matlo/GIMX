/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>
#include <string.h>

#include <gusb.h>
#include <gpoll.h>
#include <gtimer.h>
#include "common.h"

#define PERIOD 5000 //microseconds
#define RUMBLE_PERIOD 1000000 //microseconds
#define FF_PERIOD 80000 //microseconds

typedef struct {
  unsigned short length;
  unsigned char data[64];
} s_usb_packet;

static struct {
  unsigned short vid;
  unsigned short pid;
  struct {
      unsigned char in;
      unsigned char out;
  } endpoints;
  s_usb_packet start;
  s_usb_packet stop;
} rumble_cmds[] = {
    {
        .vid = 0x046d,
        .pid = 0xc218,
        .endpoints = { .in = 0x81, .out = 0x01 },
        .start = { 7, { 0x51, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x00 } },
        .stop =  { 1, { 0xf3 } },
    },
    {
        .vid = 0x054c,
        .pid = 0x05c4,
        .endpoints = { .in = 0x84, .out = 0x03 },
        .start = { 9, { 0x05, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00 } },
        .stop =  { 9, { 0x05, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00 } },
    }
};

static int rumble_index = -1;

static struct {
  unsigned short vid;
  unsigned short pid;
  struct {
      unsigned char in;
      unsigned char out;
  } endpoints;
  s_usb_packet stop;
  s_usb_packet left;
  s_usb_packet right;
} ff_cmds[] = {
    {
        .vid = 0x046d,
        .pid = 0xc29a,
        .endpoints = { .in = 0x81, .out = 0x01 },
        .stop = { 7, { 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, // stop all forces
        .left  = { 7, { 0x11, 0x08, 0x60, 0x80, 0x00, 0x00, 0x00 } },
        .right = { 7, { 0x11, 0x08, 0xa0, 0x80, 0x00, 0x00, 0x00 } },
    },
};

static int ff_index = -1;

static int usb = -1;

static char * usb_select() {

  char * path = NULL;

  struct gusb_device * usb_devs = gusb_enumerate(0x0000, 0x0000);
  if (usb_devs == NULL) {
    fprintf(stderr, "No usb device detected!\n");
    return NULL;
  }
  printf("Available usb devices:\n");
  unsigned int index = 0;
  struct gusb_device * current;
  for (current = usb_devs; current != NULL; current = current->next) {
    printf("%d VID 0x%04x PID 0x%04x PATH %s\n", index++, current->vendor_id, current->product_id, current->path);
  }

  printf("Select the usb device number: ");
  fflush(stdout);
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice < index) {
    current = usb_devs;
    while(choice > 0) {
        current = current->next;
        --choice;
    }
    unsigned int i;
    for (i = 0; i < sizeof(rumble_cmds) / sizeof(*rumble_cmds); ++i) {
      if(rumble_cmds[i].vid == current->vendor_id && rumble_cmds[i].pid == current->product_id) {
        rumble_index = i;
      }
    }
    for (i = 0; i < sizeof(ff_cmds) / sizeof(*ff_cmds); ++i) {
      if(ff_cmds[i].vid == current->vendor_id && ff_cmds[i].pid == current->product_id) {
        ff_index = i;
      }
    }
    if (rumble_index == -1 && ff_index == -1) {
      fprintf(stderr, "Selected usb device is not supported!\n");
    } else {
      path = strdup(current->path);
      if(path == NULL) {
        fprintf(stderr, "can't duplicate path.\n");
      }
    }
  } else {
    fprintf(stderr, "Invalid choice.\n");
  }

  gusb_free_enumeration(usb_devs);

  return path;
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

int usb_read(int user __attribute__((unused)), unsigned char endpoint, const void * buf, int status) {

  if (status < 0) {
    set_done();
    return 1;
  }

  int ret = gusb_poll(usb, endpoint);
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

static int usb_busy = 0;

static int counter = 0;

void rumble_task(int device) {

  if(rumble_index < 0) {
    return;
  }

  static int rumble = 0;

  if (!usb_busy) {

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
      gusb_write(device, rumble_cmds[rumble_index].endpoints.out, rumble_cmds[rumble_index].start.data, rumble_cmds[rumble_index].start.length);
    } else  {
      gusb_write(device, rumble_cmds[rumble_index].endpoints.out, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length);
    }
    usb_busy = 1;
  }
}

void ff_task(int device) {

  if(ff_index < 0) {
    return;
  }

  static int ff_play = 0;
  static int ff_dir = 0;

  if (!usb_busy) {

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
        gusb_write(device, ff_cmds[ff_index].endpoints.out, ff_cmds[ff_index].left.data, ff_cmds[ff_index].left.length);
      } else {
        gusb_write(device, ff_cmds[ff_index].endpoints.out, ff_cmds[ff_index].right.data, ff_cmds[ff_index].right.length);
      }
      ++cpt;
      if(cpt == FF_PERIOD / PERIOD) {
        ff_dir = !ff_dir;
        cpt = 0;
      }
    } else  {
      gusb_write(device, ff_cmds[ff_index].endpoints.out, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length);
    }
    usb_busy = 1;
  }
}

void usb_task(int device) {

  if(is_done()) {
    return;
  }
  rumble_task(device);
  ff_task(device);
}

int usb_write(int user __attribute__((unused)), unsigned char endpoint __attribute__((unused)), int transfered __attribute__((unused))) {

  /*struct timeval t;
  gettimeofday(&t, NULL);
  printf("%ld.%06ld %s\n", t.tv_sec, t.tv_usec, __func__);*/

  if (endpoint == 0x00) {

      unsigned char endpoint;
      if(rumble_index >= 0) {
        endpoint = rumble_cmds[rumble_index].endpoints.in;
      }
      if(ff_index >= 0) {
        endpoint = ff_cmds[ff_index].endpoints.in;
      }
      int ret = gusb_poll(usb, endpoint);
      if (ret < 0) {
        set_done();
      }
  }

  usb_busy = 0;
  usb_task(usb);
  return 0;
}

int usb_close(int user __attribute__((unused))) {
  set_done();
  return 0;
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {

  setup_handlers();

  if (gusb_init() < 0) {
    return -1;
  }

  char * path = usb_select();

  if(path == NULL) {
    fprintf(stderr, "No usb device selected!\n");
    exit(-1);
  }

  usb = gusb_open_path(path);

  if (usb >= 0) {

    const s_usb_descriptors * usb_desc = gusb_get_usb_descriptors(usb);

    printf("Opened device: VID 0x%04x PID 0x%04x PATH %s\n", usb_desc->device.idVendor, usb_desc->device.idProduct, path);

    GUSB_CALLBACKS gusb_callbacks = {
            .fp_read = usb_read,
            .fp_write = usb_write,
            .fp_close = usb_close,
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if (gusb_register(usb, 42, &gusb_callbacks) != -1) {

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

      if (!is_done()) {
          struct {
              struct usb_ctrlrequest req;
              unsigned char data[64];
          } transfer = {
                  .req = {
                          .bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          .bRequest = 0x01, // GET REPORT
                          .wValue = 0x0100, // report type = input (1), report id = 0
                          .wIndex = 0x0000, // interface 0
                          .wLength = sizeof(transfer.data),
                  },
                  .data = { }
          };

          int ret = gusb_write(usb, 0x00, &transfer, sizeof(transfer.req));
          if (ret < 0) {
              set_done();
          } else {
              usb_busy = 1;
          }
      }

      while (!is_done() || usb_busy) {

        gpoll();

        ++counter;
      }

      if (timer >= 0) {
        gtimer_close(timer);
      }

      if(rumble_index >= 0) {
        gusb_write_timeout(usb, rumble_cmds[rumble_index].endpoints.out, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length, 1000);
      }

      if(ff_index >= 0) {
        gusb_write_timeout(usb, ff_cmds[ff_index].endpoints.out, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length, 1000);
      }

      gusb_close(usb);
    }
  }

  gusb_exit();

  free(path);

  printf("Exiting\n");

  return 0;
}
