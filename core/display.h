/*
 * display.h
 *
 *  Created on: 6 mai 2012
 *      Author: matlo
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define BUTTON_NB 17

void display_init();
void display_end();
void display_run(int axes[4], int max_axis, int buttons[BUTTON_NB], int max_button);
void display_calibration();

#endif /* DISPLAY_H_ */
