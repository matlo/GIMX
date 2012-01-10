/*
 * serial_con.h
 *
 *  Created on: 9 juin 2011
 *      Author: matlo
 */

#ifndef SERIAL_CON_H_
#define SERIAL_CON_H_

#include <stdint.h>

typedef enum
{
  C_TYPE_JOYSTICK,
  C_TYPE_360_PAD,
  C_TYPE_SIXAXIS,
  C_TYPE_PS2_PAD
} e_controller_type;

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
} s_report_data;

typedef struct
{
  uint8_t head;
  uint8_t Bt1;
  uint8_t Bt2;
  int8_t Z;
  int8_t Rz;
  int8_t X;
  int8_t Y;
} s_report_data2;

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

int serial_connect(char*);
void serial_send(e_controller_type, int);
void serial_close();

#endif /* SERIAL_CON_H_ */
