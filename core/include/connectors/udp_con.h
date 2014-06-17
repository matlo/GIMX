/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef UDP_CON_H_
#define UDP_CON_H_

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN
#define MSG_DONTWAIT 0
typedef int socklen_t;
#endif

#define BYTE_LEN_0_BYTE 0x00
#define BYTE_LEN_1_BYTE 0x01

#define BYTE_TYPE 0x11
#define BYTE_SEND_REPORT 0xff

int udp_listen(unsigned int ip, unsigned short port);
int udp_connect(unsigned int ip, unsigned short port, int* type);
unsigned int udp_send(int fd, unsigned char* buf, socklen_t buflen);
unsigned int udp_sendto(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t salen);
unsigned int udp_recvfrom(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t* salen);
int udp_close(int fd);

#endif /* UDP_CON_H_ */
