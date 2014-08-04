/*
 * GE_Linux.h
 *
 *  Created on: 3 avr. 2013
 *      Author: matlo
 */

#ifndef GE_LINUX_H_
#define GE_LINUX_H_

#include <linux/input.h>

#define GE_BTN_LEFT       (BTN_LEFT-BTN_MOUSE)
#define GE_BTN_RIGHT      (BTN_RIGHT-BTN_MOUSE)
#define GE_BTN_MIDDLE     (BTN_MIDDLE-BTN_MOUSE)
#define GE_BTN_SIDE       (BTN_SIDE-BTN_MOUSE)
#define GE_BTN_EXTRA      (BTN_EXTRA-BTN_MOUSE)
#define GE_BTN_FORWARD    (BTN_FORWARD-BTN_MOUSE)
#define GE_BTN_BACK       (BTN_BACK-BTN_MOUSE)
#define GE_BTN_TASK       (BTN_TASK-BTN_MOUSE)
#define GE_BTN_WHEELUP    (GE_BTN_TASK+1)
#define GE_BTN_WHEELDOWN  (GE_BTN_TASK+2)
#define GE_BTN_WHEELRIGHT (GE_BTN_TASK+3)
#define GE_BTN_WHEELLEFT  (GE_BTN_TASK+4)

#endif /* GE_LINUX_H_ */
