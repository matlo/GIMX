/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gpoll.h>

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <gimxinput/src/windows/rawinput.h>

#include <gimxcommon/include/gerror.h>

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

static void (* rawinput_callback)() = NULL;

void gpoll_init(void) __attribute__((constructor));
void gpoll_init(void) {
  unsigned int i;
  for (i = 0; i < sizeof(sources) / sizeof(*sources); ++i) {
    sources[i].handle = INVALID_HANDLE_VALUE;
    sources[i].fd = -1;
  }
}

static int get_slot(HANDLE handle) {
  unsigned int i;
  for (i = 0; i < sizeof(sources) / sizeof(*sources); ++i) {
    if (sources[i].handle == handle) {
      return i;
    }
  }
  return -1;
}

static int get_slot_fd(int fd) {
  unsigned int i;
  for (i = 0; i < sizeof(sources) / sizeof(*sources); ++i) {
    if (sources[i].fd == fd) {
      return i;
    }
  }
  return -1;
}

/*
 * Register a socket as an event source.
 * Note that the socket becomes non-blocking.
 */
int gpoll_register_fd(int fd, int user, const GPOLL_CALLBACKS * callbacks) {

  if (fd < 0) {
    PRINT_ERROR_OTHER("fd is invalid")
    return -1;
  }
  if (!callbacks->fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!callbacks->fp_read) {
    PRINT_ERROR_OTHER("fp_read is NULL")
    return -1;
  }

  int slot = get_slot_fd(fd);
  if (slot < 0) {
    slot = get_slot(INVALID_HANDLE_VALUE);
    if (slot < 0) {
      PRINT_ERROR_OTHER("no slot available")
    }
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
  sources[slot].fp_read = callbacks->fp_read;
  sources[slot].fp_cleanup = callbacks->fp_close;

  if (slot > max_source) {
    max_source = slot;
  }

  return 0;
}

int gpoll_register_handle(HANDLE handle, int user, const GPOLL_CALLBACKS * callbacks) {

  if (handle == INVALID_HANDLE_VALUE) {
    PRINT_ERROR_OTHER("handle is invalid")
    return -1;
  }
  if (!callbacks->fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!callbacks->fp_read && !callbacks->fp_write) {
    PRINT_ERROR_OTHER("fp_read and fp_write are NULL")
    return -1;
  }

  int slot = get_slot(handle);
  if (slot < 0) {
    slot = get_slot(INVALID_HANDLE_VALUE);
    if (slot < 0) {
      PRINT_ERROR_OTHER("no slot available")
      return -1;
    }
  }

  sources[slot].fd = -1;
  sources[slot].user = user;
  sources[slot].handle = handle;
  sources[slot].fp_read = callbacks->fp_read;
  sources[slot].fp_write = callbacks->fp_write;
  sources[slot].fp_cleanup = callbacks->fp_close;

  if (slot > max_source) {
    max_source = slot;
  }

  return 0;
}

int gpoll_remove_handle(HANDLE handle) {

  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].handle == handle) {
      memset(sources + i, 0x00, sizeof(*sources));
      sources[i].handle = INVALID_HANDLE_VALUE;
      sources[i].fd = -1;
      return 0;
    }
  }
  return -1;
}

int gpoll_remove_fd(int fd) {

  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].fd == fd) {
      WSACloseEvent(sources[i].handle);
      memset(sources + i, 0x00, sizeof(*sources));
      sources[i].handle = INVALID_HANDLE_VALUE;
      sources[i].fd = -1;
      return 0;
    }
  }
  return -1;
}

static unsigned int fill_handles(HANDLE handles[]) {

  unsigned int count = 0;
  unsigned int i;
  for (i = 0; i < MAX_SOURCES; ++i) {
    if (sources[i].fp_read || sources[i].fp_write) {
      handles[count] = sources[i].handle;
      ++count;
    }
  }
  return count;
}

void gpoll() {

  int i;
  DWORD result;
  int done = 0;

  do {
    HANDLE handles[max_source + 1];
    DWORD count = fill_handles(handles);

    unsigned int dwWakeMask = 0;

    if(rawinput_callback != NULL) {
      dwWakeMask = QS_RAWINPUT;
    }

    result = MsgWaitForMultipleObjects(count, handles, FALSE, INFINITE, dwWakeMask);

    if (result == WAIT_FAILED) {
      PRINT_ERROR_GETLASTERROR("MsgWaitForMultipleObjects")
      continue;
    }

    /*
     * Check the state of every object so as to prevent starvation.
     */

    if(rawinput_callback != NULL) {
      if (GetQueueStatus(QS_RAWINPUT)) {
        rawinput_callback();
      }
    }

    for (i = 0; i < MAX_SOURCES; ++i) {
      if (sources[i].fp_read == NULL && sources[i].fp_write == NULL) {
        continue;
      }
      if (result >= count || sources[i].handle != handles[result]) {
        /*
         * Check every object except the one that has been signaled.
         */
        DWORD lresult = WaitForSingleObject(sources[i].handle, 0);
        if (lresult == WAIT_FAILED) {
          PRINT_ERROR_GETLASTERROR("WaitForSingleObject")
          continue;
        } else if (lresult != WAIT_OBJECT_0) {
          continue;
        }
      }
      if (sources[i].fd >= 0) {
        WSANETWORKEVENTS NetworkEvents;
        /*
         * Network source
         */
        if (WSAEnumNetworkEvents(sources[i].fd, sources[i].handle, &NetworkEvents)) {
          PRINT_ERROR_GETLASTERROR("WSAEnumNetworkEvents")
          sources[i].fp_cleanup(sources[i].user);
        } else {
        if (NetworkEvents.lNetworkEvents & FD_READ) {
          if (NetworkEvents.iErrorCode[FD_READ_BIT]) {
            PRINT_ERROR_OTHER("iErrorCode[FD_READ_BIT] is set");
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
    }
  } while (!done);
}

void gpoll_set_rawinput_callback(void (*callback)())
{
  rawinput_callback = callback;
}
