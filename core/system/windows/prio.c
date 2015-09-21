/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int set_prio()
{
  if(!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
  {
    fprintf(stderr, "SetPriorityClass failed.\n");
    return -1;
  }
  if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
  {
    fprintf(stderr, "SetThreadPriority failed.\n");
    return -1;
  }
  return 0;
}
