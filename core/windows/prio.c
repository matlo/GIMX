/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <windows.h>

void set_prio()
{
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}
