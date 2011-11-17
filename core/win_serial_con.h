/*
 * win_serial.h
 *
 *  Created on: 28 mai 2011
 *      Author: Matlo
 *
 *  License: GPLv3
 */

#ifndef WIN_SERIAL_CON_H_
#define WIN_SERIAL_CON_H_

#include "serial_con.h"

int win_serial_connect(char*);
void win_serial_send(s_report_data* pdata);
void win_serial_close();

#endif /* WIN_SERIAL_H_ */
