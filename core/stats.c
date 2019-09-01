/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxtime/include/gtime.h>
#include <gimx.h>

#define STATS_PERIOD 500000 //ms

static struct
{
  gtime tlast;
  int cpt;
} stats[MAX_CONTROLLERS] = {};

void stats_init(int id)
{
  stats[id].tlast = gtime_gettime();
}

void stats_update(int id)
{
  stats[id].cpt++;
}

int stats_get_frequency(int id)
{
  int ret = -1;

  gtime tnow = gtime_gettime();

  gtimediff tdiff = tnow - stats[id].tlast;

  if(tdiff > STATS_PERIOD)
  {
    ret = stats[id].cpt * 1000000 / tdiff;
    stats[id].tlast = tnow;
    stats[id].cpt = 0;
  }

  return ret;
}
