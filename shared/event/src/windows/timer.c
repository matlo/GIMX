/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

static HANDLE hTimer = NULL;

HANDLE timer_get()
{
  return hTimer;
}

HANDLE timer_start(int usec)
{
  timeBeginPeriod(1);
  
  hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  if(hTimer)
  {
    LARGE_INTEGER li = { .QuadPart = -(usec*10) };
    if(!SetWaitableTimer(hTimer, &li, usec / 1000, NULL, NULL, FALSE))
    {
      fprintf(stderr, "SetWaitableTimer failed.\n");
      hTimer = NULL;
    }
  }
  else
  {
    fprintf(stderr, "CreateWaitableTimer failed.\n");
  }
  
  if(!hTimer)
  {
    timeEndPeriod(0);
  }

  return hTimer;
}

int timer_close(int unused)
{
  if(hTimer)
  {
    CloseHandle(hTimer);

    timeEndPeriod(0);
  }

  return 1;
}

int timer_read(int unused)
{
  return 1;
}
