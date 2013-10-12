/*
 * common.h
 *
 *  Created on: 14 janv. 2013
 *      Author: matlo
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <GE.h>

extern volatile int done;

void display_devices();
int process_event(GE_Event*);

#endif /* COMMON_H_ */
