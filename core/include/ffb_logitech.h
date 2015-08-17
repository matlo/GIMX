/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FFB_LOGITECH_H_
#define FFB_LOGITECH_H_

#define FFB_LOGITECH_OUTPUT_REPORT_SIZE 7

void ffb_logitech_process_report(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]);
int ffb_logitech_get_report(unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]);

#endif /* FFB_LOGITECH_H_ */
