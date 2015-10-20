/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FFB_LOGITECH_H_
#define FFB_LOGITECH_H_

#define FFB_LOGITECH_OUTPUT_REPORT_SIZE 7

void ffb_logitech_process_report(int device, unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]);
int ffb_logitech_get_report(int device, unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]);
int ffb_logitech_is_logitech_wheel(unsigned short vendor, unsigned short product);
void ffb_logitech_set_native_mode();

#endif /* FFB_LOGITECH_H_ */
