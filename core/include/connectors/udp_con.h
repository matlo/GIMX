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
#include <connectors/windows/sockets.h>
#endif

int udp_listen(unsigned int ip, unsigned short port);
int udp_connect(unsigned int ip, unsigned short port, int* type);
int udp_send(int fd, unsigned char* buf, socklen_t buflen);
int udp_sendto(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t salen);
int udp_recvfrom(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t* salen);
int udp_close(int fd);

#endif /* UDP_CON_H_ */
