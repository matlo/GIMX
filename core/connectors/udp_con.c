/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/udp_con.h>
#include <connectors/protocol.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <ginput.h>
#include <errno.h>

#ifndef WIN32
#define psockerror(msg) perror(msg)
#endif

/*
 * Open a UDP socket and listen for UDP packets.
 */
int udp_listen(unsigned int ip, unsigned short port)
{
  int fd;
  int error = 0;

#ifdef WIN32
  if (wsa_init() < 0)
  {
    return -1;
  }
#endif

  if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    psockerror("socket");
    error = 1;
  }
  else
  {
    struct sockaddr_in sa =
    { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = ip };
  
    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
    {
      psockerror("bind");
      error = 1;
    }
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
 * Open a UDP socket and set the destination,
 * and then try to get the controller type from the remote GIMX.
 */
int udp_connect(unsigned int ip, unsigned short port, int* type)
{
  int fd = -1;
  int error = 0;

#ifdef WIN32
  if (wsa_init() < 0)
  {
    return -1;
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

      if(udp_send(fd, request, sizeof(request)) > 0)
      {
#ifndef WIN32
        struct timeval tv = { .tv_sec = 2 };
        void* optval = &tv;
#else
        unsigned long tv = 2000;
        const char* optval = (char*)&tv;
#endif
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, optval, sizeof(tv)) < 0) {
            psockerror("setsockopt SO_RCVTIMEO");
            error = 1;
        }
        else
        {
          unsigned char answer[3];
          socklen_t salen = sizeof(sa);
          int ret = udp_recvfrom(fd, answer, sizeof(answer), (struct sockaddr *) &sa, &salen);
          if(ret == sizeof(answer))
          {
            if(answer[0] == BYTE_TYPE && answer[1] == BYTE_LEN_1_BYTE)
            {
              *type = answer[2];
            }
            else
            {
              fprintf(stderr, "invalid reply from remote gimx (type=%d, length=%d)\n", answer[0], answer[1]);
              error = 1;
            }
          }
          else if(ret > 0)
          {
            fprintf(stderr, "invalid reply from remote gimx (size=%d)\n", ret);
            error = 1;
          }
          else
          {
            fprintf(stderr, "can't get controller type from remote gimx\n");
            error = 1;
          }
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
 * Send a packet to the remote GIMX.
 */
int udp_send(int fd, unsigned char* buf, socklen_t buflen)
{
  int ret = send(fd, (const void*)buf, buflen, MSG_DONTWAIT);
  if(ret < 0)
  {
    psockerror("send");
  }
  return ret;
}

/*
 * Send a packet to the remote GIMX.
 */
int udp_sendto(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t salen)
{
  int ret = sendto(fd, (const void*)buf, buflen, MSG_DONTWAIT, sa, salen);
  if(ret < 0)
  {
    psockerror("sendto");
  }
  return ret;
}

/*
 * Get the UDP packet and the address of the client.
 */
int udp_recvfrom(int fd, unsigned char* buf, socklen_t buflen, struct sockaddr* sa, socklen_t* salen)
{
  int ret = recvfrom(fd, (char*)buf, buflen, 0, sa, salen);
  if(ret < 0)
  {
    psockerror("recvfrom");
  }
  return ret;
}

/*
 * Close a UDP socket.
 */
int udp_close(int fd)
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
