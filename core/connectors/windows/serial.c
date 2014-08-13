/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/serial.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <adapter.h>

static struct serial
{
  HANDLE handle;
} serials[MAX_CONTROLLERS] = {};

/*
 * The baud rate in bps.
 */
static int baudrate = 500000;

/*
 * Connect to a serial port.
 */
int serial_connect(int id, char* portname)
{
  DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
  char scom[16];
  snprintf(scom, sizeof(scom), "\\\\.\\%s", portname);
  HANDLE handle = CreateFile(scom, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
  if (handle != INVALID_HANDLE_VALUE)
  {
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(handle, &dcbSerialParams))
    {
      CloseHandle(handle);
      handle = INVALID_HANDLE_VALUE;
    }
    else
    {
      dcbSerialParams.BaudRate = baudrate;
      dcbSerialParams.ByteSize = 8;
      dcbSerialParams.StopBits = ONESTOPBIT;
      dcbSerialParams.Parity = NOPARITY;
      if (!SetCommState(handle, &dcbSerialParams))
      {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
      }
      else
      {
        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 1000;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
        if (!SetCommTimeouts(handle, &timeouts))
        {
          CloseHandle(handle);
          handle = INVALID_HANDLE_VALUE;
        }
      }
    }
  }
  if(handle == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  else
  {
    serials[id].handle = handle;
    return 0;
  }
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(int id, void* pdata, unsigned int size)
{
  DWORD dwBytesWrite = 0;

  WriteFile(serials[id].handle, (uint8_t*)pdata, size, &dwBytesWrite, NULL);

  return dwBytesWrite;
}

int serial_read(int id, void* pdata, unsigned int size)
{
  DWORD dwBytesRead = 0;

  if(ReadFile(serials[id].handle, (uint8_t*)pdata, size, &dwBytesRead, NULL))
  {
    return dwBytesRead;
  }
  return 0;
}

int serial_recv(int id, void* pdata, unsigned int size)
{
  return serial_read(serials[id].handle, pdata, size);
}

/*
 * Close the serial port.
 */
void serial_close(int id)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  CloseHandle(serials[id].handle);
}
