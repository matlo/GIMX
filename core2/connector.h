/*
 * connector.h
 *
 *  Created on: 5 déc. 2012
 *      Author: matlo
 */

#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "emuclient.h"

int connector_init(e_controller_type t, char* port);
void connector_clean();


#endif /* CONNECTOR_H_ */
