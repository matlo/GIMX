/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <events.h>
#include <stdio.h>

static int debug = 0;

#define eprintf(...) if(debug) printf(__VA_ARGS__)

static GE_Event queue[MAX_EVENTS];
static unsigned char queue_index_first = 0;
static unsigned char queue_index_last = 0;

void queue_init()
{
  queue_index_first = 0;
  queue_index_last = 0;
}

int queue_push_event(GE_Event* ev)
{
  int ret = -1;
  eprintf("first: %d last: %d\n", queue_index_first, queue_index_last);
  if(queue_index_last >= queue_index_first)
  {
    queue[queue_index_last] = *ev;
    queue_index_last++;
    queue_index_last%=MAX_EVENTS;
    if(queue_index_last == queue_index_first)
    {
      queue_index_last = MAX_EVENTS-1;
    }
    else
    {
      ret = 0;
    }
  }
  else if(queue_index_last+1 < queue_index_first)
  {
    queue[queue_index_last] = *ev;
    queue_index_last++;
    ret = 0;
  }
  eprintf("ret: %d first: %d last: %d\n", ret, queue_index_first, queue_index_last);
  return ret;
}

int queue_peep_events(GE_Event *events, int numevents)
{
  int i;
  int j = 0;
  if(queue_index_first > queue_index_last)
  {
    for(i=queue_index_first; i<MAX_EVENTS; ++i)
    {
      eprintf("peep: %d\n", i);
      events[j] = queue[i];
      ++queue_index_first;
      queue_index_first%=MAX_EVENTS;
      ++j;
      if(j == numevents)
      {
        return numevents;
      }
    }
  }
  for(i=queue_index_first; i<queue_index_last; ++i)
  {
    eprintf("peep: %d\n", i);
    events[j] = queue[i];
    ++queue_index_first;
    queue_index_first%=MAX_EVENTS;
    ++j;
    if(j == numevents)
    {
      return numevents;
    }
  }
  return j;
}
