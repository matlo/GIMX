/*
 * report.h
 *
 *  Created on: 7 déc. 2012
 *      Author: matlo
 */

#ifndef REPORT_H_
#define REPORT_H_

#include <emuclient.h>

/*
 * The usb report structure to send over the serial connection.
 */
typedef struct
{
  int16_t X;
  int16_t Y;
  int16_t Z;
  int16_t Rz;
  uint16_t Hat;
  uint16_t Bt;
} s_report_joystick;

typedef struct
{
  uint8_t head;
  uint8_t Bt1;
  uint8_t Bt2;
  int8_t Z;
  int8_t Rz;
  int8_t X;
  int8_t Y;
} s_report_ps2;

typedef struct
{
  uint8_t type;
  uint8_t size;
  uint16_t buttons;
  uint8_t ltrigger;
  uint8_t rtrigger;
  uint16_t xaxis;
  uint16_t yaxis;
  uint16_t zaxis;
  uint16_t taxis;
  uint8_t unused[6];
} s_report_360;

typedef union
{
  s_report_joystick js;
  s_report_ps2 ps2;
  s_report_360 x360;
  unsigned char sixaxis[49];
} s_report;

unsigned int report_build(s_report* report, e_controller_type type);

#endif /* REPORT_H_ */