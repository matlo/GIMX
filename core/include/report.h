/*
 * report.h
 *
 *  Created on: 7 déc. 2012
 *      Author: matlo
 */

#ifndef REPORT_H_
#define REPORT_H_

#include <controllers/controller.h>
#include <controllers/ds2.h>
#include <controllers/ds3.h>
#include <controllers/ds4.h>
#include <controllers/joystick.h>
#include <controllers/xbox.h>
#include <controllers/x360.h>

typedef union
{
  s_report_joystick js;
  s_report_ds2 ds2;
  s_report_ds3 ds3;
  s_report_ds4 ds4;
  s_report_xbox xbox;
  s_report_x360 x360;
} s_report_union;

typedef struct
{
  unsigned char packet_type;
  unsigned char value_len;
  s_report_union value;
} s_report;

unsigned int report_build(s_controller* controller, s_report* report);

#endif /* REPORT_H_ */
