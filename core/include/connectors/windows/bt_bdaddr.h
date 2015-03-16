/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_BDADDR_H_
#define BT_BDADDR_H_

/* BD Address */
typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

int bachk(const char *str);
void baswap(bdaddr_t *dst, bdaddr_t *src);
int ba2str(const bdaddr_t *ba, char *str);
int str2ba(const char *str, bdaddr_t *ba);

#endif /* BT_BDADDR_H_ */
