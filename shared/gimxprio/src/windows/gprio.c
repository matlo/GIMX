/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <gimxcommon/include/gerror.h>

int gprio() {

  if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
    PRINT_ERROR_GETLASTERROR("SetPriorityClass")
    return -1;
  }
  DWORD priority = GetPriorityClass(GetCurrentProcess());
  if (priority == 0) {
    PRINT_ERROR_GETLASTERROR("GetPriorityClass")
    return -1;
  }
  if (priority != REALTIME_PRIORITY_CLASS) {
    PRINT_ERROR_OTHER("failed to set priority class")
  }
  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL)) {
    PRINT_ERROR_GETLASTERROR("SetThreadPriority")
    return -1;
  }
  priority = GetThreadPriority(GetCurrentThread());
  if (priority == 0) {
    PRINT_ERROR_GETLASTERROR("GetPriorityClass")
    return -1;
  }
  if (priority != THREAD_PRIORITY_TIME_CRITICAL) {
    PRINT_ERROR_OTHER("failed to set thread priority")
  }
  return 0;
}
