/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

static HANDLE hTimer = NULL;

inline HANDLE timer_get()
{
  return hTimer;
}

HANDLE timer_start(int usec)
{
  hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  LARGE_INTEGER li = { .QuadPart = 0 };
  SetWaitableTimer(hTimer, &li, usec / 1000, NULL, NULL, FALSE);
  
  return hTimer;
}

int timer_close(int unused)
{
  CloseHandle(hTimer);

  return 1;
}

int timer_read(int unused)
{
  return 1;
}
