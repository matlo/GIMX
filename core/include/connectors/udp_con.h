/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef UDP_CON_H_
#define UDP_CON_H_

int udp_listen(unsigned int ip, unsigned short port);
int udp_connect(unsigned int ip, unsigned short port);
unsigned int udp_send(int fd, unsigned char* buf, unsigned int len);
int udp_close(int fd);

#endif /* UDP_CON_H_ */
