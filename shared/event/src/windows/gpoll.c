/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <gpoll.h>
#include "rawinput.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define MAX_SOURCES (MAXIMUM_WAIT_OBJECTS-1)

static struct
{
  int fd;
  int user;
  HANDLE handle;
  int (*fp_read)(int);
  int (*fp_write)(int);
  int (*fp_cleanup)(int);
} sources[MAX_SOURCES] = {};

static int max_source = 0;

unsigned char mkb_source = GE_MKB_SOURCE_NONE;

void gpoll_init(void) __attribute__((constructor (101)));
void gpoll_init(void) {
  unsigned int i;
  for (i = 0; i < sizeof(sources) / sizeof(*sources); ++i) {
    sources[i].handle = INVALID_HANDLE_VALUE;
    sources[i].fd = -1;
  }
}

static int get_slot() {
  unsigned int i;
  for (i = 0; i < sizeof(sources) / sizeof(*sources); ++i) {
    if (sources[i].handle == INVALID_HANDLE_VALUE) {
      return i;
    }
  }
  return -1;
}

/*
 * Register a socket as an event source.
 * Note that the socket becomes non-blocking.
 */
int gpoll_register_fd(int fd, int user, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write,
    GPOLL_CLOSE_CALLBACK fp_close) {

  if (fd < 0) {
    PRINT_ERROR_OTHER("fd is invalid")
    return -1;
  }
  if (!fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!fp_read && !fp_write) {
    PRINT_ERROR_OTHER("fp_read and fp_write are NULL")
    return -1;
  }

  int slot = get_slot();
  if (slot < 0) {
    PRINT_ERROR_OTHER("no slot available")
  }

  HANDLE evt = CreateEvent(NULL, TRUE, FALSE, NULL);
  if(WSAEventSelect(fd, evt, FD_READ | FD_CLOSE) == SOCKET_ERROR)
  {
    PRINT_ERROR_OTHER("WSAEventSelect failed.")
    return -1;
  }

  sources[slot].fd = fd;
  sources[slot].user = user;
  sources[slot].handle = evt;
  sources[slot].fp_read = fp_read;
  sources[slot].fp_cleanup = fp_close;

  if (slot > max_source) {
    max_source = slot;
  }

  return 0;
}

int gpoll_register_handle(HANDLE handle, int user, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write,
    GPOLL_CLOSE_CALLBACK fp_close) {

  if (handle == INVALID_HANDLE_VALUE) {
    PRINT_ERROR_OTHER("handle is invalid")
    return -1;
  }
  if (!fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!fp_read && !fp_write) {
    PRINT_ERROR_OTHER("fp_read and fp_write are NULL")
    return -1;
  }

  int slot = get_slot();
  if (slot < 0) {
    PRINT_ERROR_OTHER("no slot available")
    return -1;
  }

  sources[slot].fd = -1;
  sources[slot].user = user;
  sources[slot].handle = handle;
  sources[slot].fp_read = fp_read;
  sources[slot].fp_write = fp_write;
  sources[slot].fp_cleanup = fp_close;

  if (slot > max_source) {
    max_source = slot;
  }

  return 0;
}

void gpoll_remove_handle(HANDLE handle) {

  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].handle == handle) {
      memset(sources + i, 0x00, sizeof(*sources));
      sources[i].handle = INVALID_HANDLE_VALUE;
      sources[i].fd = -1;
      break;
    }
  }
}

void gpoll_remove_fd(int fd) {

  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].fd == fd) {
      WSACloseEvent(sources[i].handle);
      memset(sources + i, 0x00, sizeof(*sources));
      sources[i].handle = INVALID_HANDLE_VALUE;
      sources[i].fd = -1;
      break;
    }
  }
}

static int fill_handles(HANDLE handles[]) {

  int count = 0;
  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].fp_read || sources[i].fp_write) {
      handles[count] = sources[i].handle;
      ++count;
    }
  }
  return count;
}

void plasterror(const char* msg) {

  DWORD error = GetLastError();
  LPTSTR pBuffer = NULL;

  if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPTSTR) & pBuffer, 0,
      NULL)) {
    fprintf(stderr, "%s failed with error: %lu\n", msg, error);
  } else {
    fprintf(stderr, "%s failed with error: %s\n", msg, pBuffer);
    LocalFree(pBuffer);
  }
}

void gpoll() {

  int i;
  DWORD result;
  int done = 0;

  do {
    HANDLE handles[max_source + 1];
    int count = fill_handles(handles);

    unsigned int dwWakeMask = 0;

    if(mkb_source == GE_MKB_SOURCE_PHYSICAL) {
      dwWakeMask = QS_RAWINPUT;
    }

    result = MsgWaitForMultipleObjects(count, handles, FALSE, INFINITE, dwWakeMask);

    if (result == WAIT_FAILED) {
      plasterror("MsgWaitForMultipleObjects");
    } else if (result == WAIT_OBJECT_0 + count) {

      if(mkb_source == GE_MKB_SOURCE_PHYSICAL) {
        rawinput_poll();
      }
    } else {
      for (i = 0; i <= max_source; ++i) {
        if (sources[i].handle == handles[result]) {
          if (sources[i].fd >= 0) {
            WSANETWORKEVENTS NetworkEvents;
            /*
             * Network source
             */
            if (WSAEnumNetworkEvents(sources[i].fd, handles[result], &NetworkEvents)) {
              plasterror("WSAEnumNetworkEvents");
              sources[i].fp_cleanup(sources[i].user);
            } else {
              if (NetworkEvents.lNetworkEvents & FD_READ) {
                if (NetworkEvents.iErrorCode[FD_READ_BIT]) {
                  fprintf(stderr, "iErrorCode[FD_READ_BIT] is set\n");
                  sources[i].fp_cleanup(sources[i].user);
                } else {
                  if (sources[i].fp_read(sources[i].user)) {
                    done = 1;
                  }
                }
              }
            }
          } else {
            /*
             * Other sources (timers, COM port, HID...)
             */
            if (sources[i].fp_read != NULL) {
              if (sources[i].fp_read(sources[i].user)) {
                done = 1;
              }
            }
            if (sources[i].fp_write != NULL) {
              if (sources[i].fp_write(sources[i].user)) {
                done = 1;
              }
            }
          }
          break;
        }
      }
    }
  } while (!done);
}
