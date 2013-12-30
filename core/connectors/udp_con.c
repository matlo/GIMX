/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#define MSG_DONTWAIT 0
#endif
#include <stdio.h>
#include <unistd.h>
#include <GE.h>

int udp_listen(unsigned int ip, unsigned short port)
{
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    fprintf(stderr, "socket\n");
    return -1;
  }

  struct sockaddr_in sa =
  { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

  if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
  {
    fprintf(stderr, "socket\n");
    close(fd);
    return -1;
  }

  return fd;
}

int udp_connect(unsigned int ip, unsigned short port)
{
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    fprintf(stderr, "socket\n");
    return -1;
  }

  struct sockaddr_in sa =
  { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

  if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
  {
    fprintf(stderr, "connect\n");
    close(fd);
    return -1;
  }

  return fd;
}

unsigned int udp_send(int fd, unsigned char* buf, unsigned int len)
{
  //TODO: fix flags
  return send(fd, buf, len, MSG_DONTWAIT);
}

int udp_close(int fd)
{
  close(fd);
  /*
   * TODO MLA: Windows implementation.
   */
  GE_RemoveSource(fd);

  return 1;
}
