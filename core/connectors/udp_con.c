/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#include <connectors/udp_con.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <GE.h>
#include <errno.h>

#ifdef WIN32
//this is used to make sure WSAStartup/WSACleanup are only called once,
//and to make sure all the sockets are closed before calling WSACleanup
static unsigned int cnt = 0;
#endif

/*
 * Open a UDP socket and listen for UDP packets.
 */
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

/*
 * 1. open a UDP socket and set the destination,
 * 2. try to get the controller type from the remote GIMX.
 * If 1. is not successful the function returns -1.
 * If 1. is sucessful but 2. fails, the function returns a valid socket,
 * but the controller type is not set.
 */
int udp_connect(unsigned int ip, unsigned short port, int* type)
{
  int fd = -1;
  int error = 0;
  
#ifdef WIN32
  WSADATA wsadata;

  if(!cnt)
  {
    if (WSAStartup(MAKEWORD(2,2), &wsadata) == SOCKET_ERROR)
    {
      fprintf(stderr, "WSAStartup");
      return -1;
    }
  }
#endif

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0)
  {
    struct sockaddr_in sa =
    { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };

    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) != -1)
    {
      //request the controller type from the remote gimx
      
      unsigned char request[] = {BYTE_TYPE, BYTE_LEN_0_BYTE};
      
      if(udp_send(fd, request, sizeof(request)) != -1)
      {
        unsigned char answer[3];
        socklen_t salen = sizeof(sa);
        int ret = udp_recvfrom(fd, answer, sizeof(answer), (struct sockaddr *) &sa, &salen);
        if(ret == sizeof(answer) && answer[0] == BYTE_TYPE && answer[1] == BYTE_LEN_1_BYTE)
        {
          *type = answer[2];
        }
        else
        {
          fprintf(stderr, "can't get controller type from remote gimx\n");
        }
      }
      else
      {
        fprintf(stderr, "can't send request to remote gimx\n");
        error = 1;
      }
    }
    else
    {
      perror("connect");
      error = 1;
    }
  }
  else
  {
    perror("socket");
    error = 1;
  }

  if(error && fd >= 0)
  {
    close(fd);
    fd = -1;
  }
  
#ifdef WIN32
  if(!error)
  {  
    ++cnt;
  }
  
  if(!cnt)
  {
    WSACleanup();
  }
#endif

  return fd;
}

/*
 * Send a packet to the remote GIMX.
 * "Connection refused" errors are ignored.
 */
unsigned int udp_send(int fd, unsigned char* buf, socklen_t buflen)
{
  int ret = send(fd, (const void*)buf, buflen, MSG_DONTWAIT);

  if(ret != buflen)
  {
#ifndef WIN32
    if(errno != ECONNREFUSED)
    {
      perror("sendto");
      return -1;
    }
#else
    int error = WSAGetLastError();
    if(error != WSAECONNREFUSED)
    {
      fprintf(stderr, "sendto failed with error %d\n", error);
      return -1;
    }
#endif
  }

  return 0;
}

/*
 * Send a packet to the remote GIMX.
 *
 * "Connection refused" errors are ignored.
 */
unsigned int udp_sendto(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t salen)
{
  int ret = sendto(fd, (const void*)buf, buflen, MSG_DONTWAIT, sa, salen);

  if(ret != buflen)
  {
#ifndef WIN32
    if(errno != ECONNREFUSED)
    {
      perror("sendto");
      return -1;
    }
#else
    int error = WSAGetLastError();
    if(error != WSAECONNREFUSED)
    {
      fprintf(stderr, "sendto failed with error %d\n", error);
      return -1;
    }
#endif
  }

  return 0;
}

/*
 * Get the UDP packet and the address of the client.
 * If there is not data available, return 0.
 */
unsigned int udp_recvfrom(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t* salen)
{
  int nread = 0;
  int ret;
  while(!nread)
  {
    if((ret = recvfrom(fd, (char*)buf, buflen, 0, sa, salen)) < 0)
    {
#ifndef WIN32
      if(errno != EWOULDBLOCK)
      {
        perror("recvfrom");
        return 0;
      }
#else
      int error = WSAGetLastError();
      if(error != WSAEWOULDBLOCK && errno != WSAECONNRESET)
      {
        fprintf(stderr, "recvfrom failed with error %d\n", error);
        return 0;
      }
#endif
    }
    else
    {
      nread = ret;
    }
  }
  return nread;
}

/*
 * Close a UDP socket.
 */
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
