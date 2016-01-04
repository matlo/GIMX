/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int gprio()
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
