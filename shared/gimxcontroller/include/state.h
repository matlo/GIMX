/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef STATE_H_
#define STATE_H_

#include "ds2.h"
#include "ds3.h"
#include "ds4.h"
#include "joystick.h"
#include "xbox.h"
#include "x360.h"
#include "xone.h"
#include "t300rs_ps4.h"
#include "g27_ps3.h"
#include "g29_ps4.h"
#include "df_ps2.h"
#include "dfp_ps2.h"
#include "gtf_ps2.h"

typedef union
{
  s_gc_state_joystick js;
  s_gc_state_ds2 ds2;
  s_gc_state_ds3 ds3;
  s_gc_state_ds4 ds4;
  s_gc_state_xbox xbox;
  s_gc_state_x360 x360;
  s_gc_state_xone xone;
  s_gc_state_t300rsPs4 t300rsPs4;
  s_gc_state_g27Ps3 g27Ps3;
  s_gc_state_g29Ps4 g29Ps4;
  s_gc_state_dfPs2 dfPs2;
  s_gc_state_dfpPs2 dfpPs2;
  s_gc_state_gtfPs2 gtfPs2;
} s_gc_state;

#endif /* STATE_H_ */
