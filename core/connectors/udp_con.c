/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#else
#include <winsock2.h>
#define MSG_DONTWAIT 0
#endif
#include <stdio.h>
#include <unistd.h>
#include <GE.h>

#ifdef WIN32
//this is used to make sure WSAStartup/WSACleanup are only called once,
//and to make sure all the sockets are closed before calling WSACleanup
static unsigned int cnt = 0;
#endif

int udp_listen(unsigned int ip, unsigned short port)
{
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    perror("socket");
    return -1;
  }

  struct sockaddr_in sa =
  { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

  if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
  {
    perror("bind");
    close(fd);
    return -1;
  }

  return fd;
}

int udp_connect(unsigned int ip, unsigned short port)
{
  int fd;
  
#ifdef WIN32
  WSADATA wsadata;

  if(!cnt)
  {
    if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR)
    {
      fprintf(stderr, "WSAStartup");
      return -1;
    }
  }
#endif

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    perror("socket");
#ifdef WIN32
    if(!cnt)
    {
      WSACleanup();
    }
#endif
    return -1;
  }

  struct sockaddr_in sa =
  { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

  if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
  {
    perror("connect");
#ifdef WIN32
    if(!cnt)
    {
      WSACleanup();
    }
#endif
    close(fd);
    return -1;
  }
  
#ifdef WIN32
  ++cnt;
#endif

  return fd;
}

unsigned int udp_send(int fd, unsigned char* buf, unsigned int len)
{
  int ret = send(fd, (const void*)buf, len, MSG_DONTWAIT);

  if(ret != len && errno != ECONNREFUSED)
  {
    perror("send");
    return -1;
  }

  return 0;
}

int udp_close(int fd)
{
  close(fd);
  
#ifdef WIN32
  --cnt;
  
  if(!cnt)
  {
    WSACleanup();
  }
#endif

  return 1;
}
