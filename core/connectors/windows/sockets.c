/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <winsock2.h>

//this is used to make sure WSAStartup/WSACleanup are only called once,
//and to make sure all the sockets are closed before calling WSACleanup
static unsigned int socket_count = 0;

int wsa_init()
{
  WSADATA wsadata;
  if(!socket_count)
  {
    if (WSAStartup(MAKEWORD(2,2), &wsadata) == SOCKET_ERROR)
    {
      fprintf(stderr, "WSAStartup failed.");
      return -1;
    }
  }
  return 0;
}

void wsa_count(int error)
{
  if(!error)
  {
    ++socket_count;
  }

  if(!socket_count)
  {
    WSACleanup();
  }
}

void wsa_clean()
{
  --socket_count;

  if(!socket_count)
  {
    WSACleanup();
  }
}

void psockerror(const char* msg)
{
  DWORD error = GetLastError();
  LPTSTR pBuffer = NULL;

  if(!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          error,
          0,
          (LPTSTR)&pBuffer,
          0,
          NULL))
  {
    fprintf(stderr, "%s failed with error: %lu\n", msg, error);
  }
  else
  {
    fprintf(stderr, "%s failed with error: %s\n", msg, pBuffer);
    LocalFree(pBuffer);
  }
}
