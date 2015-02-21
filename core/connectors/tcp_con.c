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
#include <connectors/windows/sockets.h>
#endif
#include "gimx.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#define psockerror(msg) perror(msg)
#endif

/*
 * Open a TCP socket and connect to ip:port.
 */
int tcp_connect(unsigned int ip, unsigned short port)
{
  int fd;
  int error = 0;

#ifdef WIN32
  if (wsa_init() < 0)
  {
    return -1;
  }
#endif

  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != -1)
  {
    struct sockaddr_in sa =
    { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != -1)
    {
      gprintf(_("connected to %s:%d\n"), inet_ntoa(sa.sin_addr), port);

#ifdef WIN32
      // Set the socket I/O mode; iMode = 0 for blocking; iMode != 0 for non-blocking
      int iMode = 1;
      if(ioctlsocket(fd, FIONBIO, (u_long FAR*) &iMode) == SOCKET_ERROR)
      {
        psockerror("ioctlsocket");
        error = 1;
      }
#endif
    }
    else
    {
      psockerror("connect");
      error = 1;
    }
  }
  else
  {
    psockerror("socket");
    error = 1;
  }

  if(error && fd >= 0)
  {
    close(fd);
    fd = -1;
  }

#ifdef WIN32
  wsa_count(error);
#endif

  return fd;
}

/*
 * Close connection.
 */
int tcp_close(int fd)
{
  if(fd >= 0)
  {
    close(fd);

#ifdef WIN32
    wsa_clean();
#endif
  }

  return 1;
}

/*
 * Send a packet.
 */
int tcp_send(int fd, const unsigned char* buf, int length)
{
  int ret = send(fd, (const void*)buf, length, MSG_DONTWAIT | MSG_NOSIGNAL);
  if(ret < 0)
  {
    psockerror("send");
  }
  return ret;
}

/*
 * Receive data from a tcp socket.
 */
int tcp_recv(int fd, unsigned char* buf, int len)
{
  int ret = recv(fd, (void*)buf, len, 0);
  if(ret < 0)
  {
    psockerror("recv");
  }
  return ret;
}
