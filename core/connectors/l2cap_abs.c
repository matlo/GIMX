/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/bluetooth/l2cap_abs.h>
#include <connectors/bluetooth/bt_abs.h>
#include <stdlib.h>
#include <stdio.h>

static s_l2cap_abs * l2cap_abs[E_L2CAP_ABS_MAX] = {};

s_l2cap_abs * l2cap_abs_get()
{
  return l2cap_abs[bt_abs_value];
}

void l2cap_abs_register(e_l2cap_abs index, s_l2cap_abs * value)
{
  l2cap_abs[index] = value;
}

void l2cap_abs_init(void) __attribute__((constructor (102)));
void l2cap_abs_init(void)
{
  int index;
  for(index = 0; index < sizeof(l2cap_abs) / sizeof(*l2cap_abs); ++index)
  {
    if(!l2cap_abs[index])
    {
      fprintf(stderr, "l2cap_abs %d is missing!\n", index);
      exit(-1);
    }
  }
}
