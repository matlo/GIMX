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

#include <hidasync.h>
#include "common.h"

#define PERIOD 5000 //milliseconds

#ifdef WIN32
#define REGISTER_FUNCTION GE_AddSourceHandle
#else
#define REGISTER_FUNCTION GE_AddSource
#endif

typedef struct {
  unsigned short length;
  unsigned char data[8];
} s_packet;

static struct {
  unsigned short vid;
  unsigned short pid;
  s_packet init;
  s_packet rumble_start;
  s_packet rumble_stop;
  s_packet clean;
} commands[] = {
    {
        .vid = 0x046d,
        .pid = 0xc218,
        .rumble_start = { 8, { 0x00, 0x51, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x00 } },
        .rumble_stop = { 2, { 0x00, 0xf3 } },
    }
};

static int cindex = -1;

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

  printf("%s\n", __func__);
  dump((unsigned char *) buf, count);
  fflush(stdout);
  return 0;
}

static int hid_busy = 0;

int hid_write(int user) {

  hid_busy = 0;
  return 0;
}

int hid_close(int user) {

  printf("close user: %d\n", user);
  done = 1;
  return 0;
}

int ignore_event(GE_Event* event) {

  return 0;
}

void hid_task(int device) {

  if(cindex < 0) {
    return;
  }

  if (!hid_busy) {
    static int i = 1;

    if (i == 200) {
      i = 0;
    } else if (i >= 100) {
      if (i == 100) {
        printf("Stop rumble\n");
        fflush(stdout);
      }
      hidasync_write(device, commands[cindex].rumble_stop.data, commands[cindex].rumble_stop.length);
      hid_busy = 1;
    } else if (i >= 1) {
      if (i == 1) {
        printf("Start rumble\n");
        fflush(stdout);
      }
      hidasync_write(device, commands[cindex].rumble_start.data, commands[cindex].rumble_start.length);
      hid_busy = 1;
    }
    ++i;
  }
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

  int device = hidasync_open_path(path);

  if (device >= 0) {

    const s_hid_info * hid_info = hidasync_get_hid_info(device);

    unsigned int i;
    for (i = 0; i < sizeof(commands) / sizeof(*commands); ++i) {
      if(commands[i].vid == hid_info->vendor_id && commands[i].pid == hid_info->product_id) {
        cindex = i;
      }
    }

    printf("Opened device: VID 0x%04x PID 0x%04x PATH %s\n", hid_info->vendor_id, hid_info->product_id, path);

    if (hidasync_register(device, 42, hid_read, hid_write, hid_close, REGISTER_FUNCTION) != -1) {

      GE_SetCallback(ignore_event);

      GE_TimerStart(PERIOD);

      while (!done) {

        GE_PumpEvents();

        hid_task(device);
      }

      GE_TimerClose();

      if(cindex >= 0) {
        hidasync_write_timeout(device, commands[cindex].rumble_stop.data, commands[cindex].rumble_stop.length, 1);
      }
    }

    hidasync_close(device);
  }

  free(path);

  GE_quit();

  printf("Exiting\n");

  return 0;
}
