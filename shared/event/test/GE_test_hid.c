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
#include <limits.h>
#include <sys/time.h>

#include <hidasync.h>
#include "common.h"

#define PERIOD 5000 //microseconds
#define RUMBLE_PERIOD 1000000 //microseconds
#define FF_PERIOD 80000 //microseconds

#ifdef WIN32
#define REGISTER_FUNCTION GE_AddSourceHandle
#else
#define REGISTER_FUNCTION GE_AddSource
#endif

typedef struct {
  unsigned short length;
  unsigned char data[65];
} s_packet;

static struct {
  unsigned short vid;
  unsigned short pid;
  s_packet start;
  s_packet stop;
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
  s_packet stop;
  s_packet left;
  s_packet right;
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

static int device = -1;

static void terminate(int sig) {
  done = 1;
}

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwType) {
  switch(dwType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:

    done = 1; //signal the main thread to terminate

    //Returning would make the process exit!
    //We just make the handler sleep until the main thread exits,
    //or until the maximum execution time for this handler is reached.
    Sleep(10000);

    return TRUE;
    default:
    break;
  }
  return FALSE;
}
#endif

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

  struct timeval t;
  gettimeofday(&t, NULL);
  printf("%ld.%06ld ", t.tv_sec, t.tv_usec);
  printf("%s\n", __func__);
  dump((unsigned char *) buf, count);
  fflush(stdout);
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
      hidasync_write(device, rumble_cmds[rumble_index].start.data, rumble_cmds[rumble_index].start.length);
    } else  {
      hidasync_write(device, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length);
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
        hidasync_write(device, ff_cmds[ff_index].left.data, ff_cmds[ff_index].left.length);
      } else {
        hidasync_write(device, ff_cmds[ff_index].right.data, ff_cmds[ff_index].right.length);
      }
      ++cpt;
      if(cpt == FF_PERIOD / PERIOD) {
        ff_dir = !ff_dir;
        cpt = 0;
      }
    } else  {
      hidasync_write(device, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length);
    }
    hid_busy = 1;
  }
}

void hid_task(int device) {

  if(done) {
    return;
  }
  rumble_task(device);
  ff_task(device);
}

int hid_write(int user, int transfered) {

  /*struct timeval t;
  gettimeofday(&t, NULL);
  printf("%ld.%06ld %s\n", t.tv_sec, t.tv_usec, __func__);*/
  hid_busy = 0;
  hid_task(device);
  return 0;
}

int hid_close(int user) {

  done = 1;
  return 0;
}

int ignore_event(GE_Event* event) {

  return 0;
}

char * hid_select() {

  char * path = NULL;

  s_hid_dev * hid_devs = hidasync_enumerate(0x0000, 0x0000);
  if (hid_devs == NULL) {
    fprintf(stderr, "No HID device detected!\n");
    return NULL;
  }
  printf("Available HID devices:\n");
  unsigned int index = 0;
  s_hid_dev * current;
  for (current = hid_devs; current != NULL; ++current) {
    printf("%d VID 0x%04x PID 0x%04x PATH %s\n", index++, current->vendor_id, current->product_id, current->path);
    if (current->next == 0) {
      break;
    }
  }

  printf("Select the HID device number: ");
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice < index) {
    path = strdup(hid_devs[choice].path);
    if(path == NULL) {
      fprintf(stderr, "can't duplicate path.\n");
    }
  } else {
    fprintf(stderr, "Invalid choice.\n");
  }

  hidasync_free_enumeration(hid_devs);

  return path;
}

int main(int argc, char* argv[]) {

  if (!GE_initialize(GE_MKB_SOURCE_NONE)) {
    fprintf(stderr, "GE_initialize failed\n");
    exit(-1);
  }

  (void) signal(SIGINT, terminate);
  (void) signal(SIGTERM, terminate);

  char * path = hid_select();

  if(path == NULL) {
    fprintf(stderr, "No HID device selected!\n");
    GE_quit();
    exit(-1);
  }

  device = hidasync_open_path(path);

  if (device >= 0) {

    const s_hid_info * hid_info = hidasync_get_hid_info(device);

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

    printf("Opened device: VID 0x%04x PID 0x%04x PATH %s\n", hid_info->vendor_id, hid_info->product_id, path);

    if (hidasync_register(device, 42, hid_read, hid_write, hid_close, REGISTER_FUNCTION) != -1) {

      GE_SetCallback(ignore_event);

      GE_TimerStart(PERIOD);

      hid_task(device);

      while (!done || hid_busy) {

        GE_PumpEvents();

        ++counter;
      }

      GE_TimerClose();

      if(rumble_index >= 0) {
        hidasync_write_timeout(device, rumble_cmds[rumble_index].stop.data, rumble_cmds[rumble_index].stop.length, 1);
      }

      if(ff_index >= 0) {
        hidasync_write_timeout(device, ff_cmds[ff_index].stop.data, ff_cmds[ff_index].stop.length, 1);
      }
    }

    hidasync_close(device);
  }

  free(path);

  GE_quit();

  printf("Exiting\n");

  return 0;
}
