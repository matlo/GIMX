/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <gerror.h>

int gprio()
{
  if(!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
  {
    PRINT_ERROR_GETLASTERROR("SetPriorityClass")
    return -1;
  }
  if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
  {
    PRINT_ERROR_GETLASTERROR("SetThreadPriority")
    return -1;
  }
  return 0;
}
