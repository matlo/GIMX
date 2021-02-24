/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef REPORT_H_
#define REPORT_H_

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
#include "switch.h"

#define MAX_REPORTS 2

typedef union
{
  s_report_joystick js;
  s_report_ds2 ds2;
  s_report_ds3 ds3;
  s_report_ds4 ds4;
  s_report_xbox xbox;
  s_report_x360 x360;
  s_report_xone xone;
  s_report_t300rsPs4 t300rsPs4;
  s_report_g27Ps3 g27Ps3;
  s_report_g29Ps4 g29Ps4;
  s_report_dfPs2 dfPs2;
  s_report_dfpPs2 dfpPs2;
  s_report_gtfPs2 gtfPs2;
  s_report_switch ns;
} s_report;

typedef struct GIMX_PACKED
{
  unsigned char type;
  unsigned char length;
  s_report value;
} s_report_packet;

#endif /* REPORT_H_ */
