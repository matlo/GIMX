/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <limits.h>
#include <gimxhid/include/ghid.h>
#include <string.h>

static char * hid_select() {

  char * path = NULL;

  struct ghid_device * hid_devs = ghid_enumerate(0x0000, 0x0000);
  if (hid_devs == NULL) {
    fprintf(stderr, "No HID device detected!\n");
    return NULL;
  }
  printf("Available HID devices:\n");
  unsigned int index = 0;
  struct ghid_device * current;
  for (current = hid_devs; current != NULL; current = current->next) {
    printf("%d VID 0x%04x PID 0x%04x PATH %s\n", index++, current->vendor_id, current->product_id, current->path);
  }

  printf("Select the HID device number: ");
  fflush(stdout);
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice < index) {
    current = hid_devs;
    while(choice > 0) {
        current = current->next;
        --choice;
    }
    path = strdup(current->path);
    if(path == NULL) {
      fprintf(stderr, "can't duplicate path.\n");
    }
  } else {
    fprintf(stderr, "Invalid choice.\n");
  }

  ghid_free_enumeration(hid_devs);

  return path;
}
