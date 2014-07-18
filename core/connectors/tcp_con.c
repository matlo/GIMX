/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef WIN32
#include <sys/ioctl.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pwd.h>
#else
#include <winsock2.h>
#define MSG_DONTWAIT 0
#endif
#include "gimx.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * Connect to all responding controllers.
 */
int tcp_connect(unsigned int ip, unsigned short port)
{
  int fd;
  struct sockaddr_in addr;

#ifdef WIN32
  WSADATA wsadata;

  if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR)
  {
    fprintf(stderr, "WSAStartup");
    return -1;
  }

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "socket");
    return -1;
  }
#else
  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    fprintf(stderr, "socket");
    return -1;
  }
#endif
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(ip);
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    fd = -1;
  }
  else
  {
    gprintf(_("connected to emu %s:%d\n"), inet_ntoa(addr.sin_addr), port);
  }

#ifdef WIN32
  // Set the socket I/O mode; iMode = 0 for blocking; iMode != 0 for non-blocking
  int iMode = 1;
  ioctlsocket(fd, FIONBIO, (u_long FAR*) &iMode);
#endif

  return fd;
}

/*
 * Close connection.
 */
void tcp_close(int fd)
{
  close(fd);
}

/*
 * Send a packet.
 */
int tcp_send(int fd, const unsigned char* buf, int length)
{
  return send(fd, (const void*)buf, length, MSG_DONTWAIT);
}
