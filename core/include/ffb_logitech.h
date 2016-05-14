/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FFB_LOGITECH_H_
#define FFB_LOGITECH_H_

#define FFB_LOGITECH_OUTPUT_REPORT_SIZE 7

typedef struct {
  unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE + 1];
} s_ffb_report;

void ffb_logitech_process_report(int device, unsigned char data[FFB_LOGITECH_OUTPUT_REPORT_SIZE]);
s_ffb_report * ffb_logitech_get_report(int device);
void ffb_logitech_ack(int device);

#ifdef WIN32
int ffb_logitech_is_logitech_wheel(unsigned short vendor, unsigned short product);
#endif

#endif /* FFB_LOGITECH_H_ */
