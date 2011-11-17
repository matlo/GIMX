/*
 * lin_serial_con.h
 *
 *  Created on: 9 juin 2011
 *      Author: matlo
 */

#ifndef LIN_SERIAL_CON_H_
#define LIN_SERIAL_CON_H_

#include "serial_con.h"

int lin_serial_connect(char*);
int lin_serial_send(s_report_data* pdata);
void lin_serial_close();

#endif /* LIN_SERIAL_CON_H_ */
