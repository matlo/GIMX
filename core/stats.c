/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stddef.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <adapter.h>

#define STATS_PERIOD 500000 //ms

#ifdef WIN32
static LARGE_INTEGER freq;
#endif

static struct
{
#ifndef WIN32
  struct timeval tlast;
#else
  LARGE_INTEGER tlast;
#endif
  int cpt;
} stats[MAX_CONTROLLERS] = {};

void stats_init(int id)
{
#ifndef WIN32
  gettimeofday(&stats[id].tlast, NULL);
#else
  QueryPerformanceCounter(&stats[id].tlast);
  QueryPerformanceFrequency(&freq);
#endif
}

void stats_update(int id)
{
  stats[id].cpt++;
}

int stats_get_frequency(int id)
{
  int ret = -1;
  int tdiff;

#ifndef WIN32
  struct timeval tnow;
  gettimeofday(&tnow, NULL);

  tdiff = (tnow.tv_sec * 1000000 + tnow.tv_usec) - (stats[id].tlast.tv_sec * 1000000 + stats[id].tlast.tv_usec);
#else
  LARGE_INTEGER tnow;
  QueryPerformanceCounter(&tnow);

  tdiff = (tnow.QuadPart - stats[id].tlast.QuadPart) * 1000000 / freq.QuadPart;
#endif

  if(tdiff > STATS_PERIOD)
  {
    ret = stats[id].cpt*1000000/tdiff;
    stats[id].tlast = tnow;
    stats[id].cpt = 0;
  }

  return ret;
}
