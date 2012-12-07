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
#include "config.h"
#include "dump.h"
#include "emuclient.h"
#include <unistd.h>

/*
 * Controller are listening from TCP_PORT to TCP_PORT+MAX_CONTROLLERS-1
 */
#define TCP_PORT 21313

/*
 * Sockets to controllers.
 */
static int sockfd[MAX_CONTROLLERS];

/*
 * Connect to all responding controllers.
 */
int tcp_connect(void)
{
    int fd;
    int i;
    int ret = -1;
    struct sockaddr_in addr;

    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
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
      addr.sin_port = htons(TCP_PORT+i);
#ifdef WIN32
      addr.sin_addr.s_addr = inet_addr(emuclient_params.ip);
#else
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
      if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
        fd = -1;
      }
      else
      {
        ret = 1;
        gprintf(_("connected to emu %d\n"), i);
      }

#ifdef WIN32
      // Set the socket I/O mode; iMode = 0 for blocking; iMode != 0 for non-blocking
      int iMode = 1;
      ioctlsocket(fd, FIONBIO, (u_long FAR*) &iMode);
#endif

      sockfd[i] = fd;

    }

    return ret;
}

/*
 * Close all connections.
 */
void tcp_close()
{
  int i;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    if(sockfd[i] > -1)
    {
      close(sockfd[i]);
    }
  }
}


int send_single(int c_id, const char* buf, int length)
{
  if(!sockfd[c_id]) return 0;

  return send(sockfd[c_id], buf, length, MSG_DONTWAIT);
}

/*
 * Send a command to each controller that has its status changed.
 */
void tcp_send(int force_update)
{
  int i;
  unsigned char buf[48];

  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    if (force_update || controller[i].send_command)
    {
      if (assemble_input_01(buf, sizeof(buf), state + i) < 0)
      {
        gprintf(_("can't assemble\n"));
      }
      send_single(i, (const char*)buf, 48);

      if (controller[i].send_command)
      {
        if(emuclient_params.status)
        {
          sixaxis_dump_state(state + i, i);
        }

        controller[i].send_command = 0;
      }
    }
  }
}

