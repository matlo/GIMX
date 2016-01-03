/*
 * linux_test.c
 *
 *  Created on: 13 janv. 2013
 *      Author: matlo
 */

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <gserial.h>
#include "common.h"

#define PERIOD 10000//microseconds

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

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

static int serial = -1;

static unsigned char packet[64] = { };
static unsigned char result[sizeof(packet)] = { };
static unsigned char read = 0;

int serial_read(int user, const void * buf, int status) {

  if (status < 0) {
    done = 1;
    return 1;
  }

  memcpy(result + read, buf, status);
  read += status;

  gserial_set_read_size(serial, sizeof(packet) - read);

  if (read < sizeof(packet)) {
    return 0;
  }

  printf("user: %d\n", user);
  dump((unsigned char *) result, sizeof(packet));

  if (memcmp(result, packet, sizeof(packet))) {
    fprintf(stderr, "bad packet content\n");
    done = 1;
  } else {
    unsigned int i;
    for (i = 0; i < sizeof(packet); ++i) {
      packet[i]++;
    }
    gserial_write(serial, packet, sizeof(packet));

    read = 0;
    gserial_set_read_size(serial, sizeof(packet));
  }

  return 0;
}

int serial_close(int user) {
  printf("close user: %d\n", user);
  return 0;
}

int main(int argc, char* argv[]) {
  if (!ginput_init(GE_MKB_SOURCE_NONE, process_event)) {
    fprintf(stderr, "GE_initialize failed\n");
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

#ifndef WIN32
  setlinebuf(stdout);
#endif

  serial = gserial_open("/dev/ttyUSB0", 500000);

  if (serial >= 0) {
    gserial_register(serial, 42, serial_read, NULL, serial_close, REGISTER_FUNCTION);

    gserial_set_read_size(serial, sizeof(packet));

    int timer = gtimer_start(42, PERIOD, timer_read, timer_close, REGISTER_FUNCTION);
    if (timer < 0) {
      done = 1;
    }

    gserial_write(serial, packet, sizeof(packet));

    while (!done) {
      gpoll();

      //do something periodically
    }

    if (timer >= 0) {
      gtimer_close(timer);
    }

    gserial_close(serial);
  } else {
    fprintf(stderr, "error opening serial device\n");
  }

  ginput_quit();

  printf("Exiting\n");

  return 0;
}
