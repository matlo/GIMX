/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/serial.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

/*
 * The baud rate in bps.
 */
static int baudrate = 500000;

/*
 * Connect to a serial port.
 */
SERIALOBJECT serial_connect(char* portname)
{
  DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
  char scom[16];
  snprintf(scom, sizeof(scom), "\\\\.\\%s", portname);
  HANDLE serial = CreateFile(scom, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
  if (serial == INVALID_HANDLE_VALUE)
  {
    return NULL;
  }
  DCB dcbSerialParams =
  { 0 };
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  if (!GetCommState(serial, &dcbSerialParams))
  {
    CloseHandle(serial);
    return NULL;
  }
  dcbSerialParams.BaudRate = baudrate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  if (!SetCommState(serial, &dcbSerialParams))
  {
    CloseHandle(serial);
    return NULL;
  }
  COMMTIMEOUTS timeouts =
  { 0 };
  timeouts.ReadIntervalTimeout = 0;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 1000;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 0;
  if (!SetCommTimeouts(serial, &timeouts))
  {
    CloseHandle(serial);
    return NULL;
  }
  return serial;
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(SERIALOBJECT serial, void* pdata, unsigned int size)
{
  DWORD dwBytesWrite = 0;

  WriteFile(serial, (uint8_t*)pdata, size, &dwBytesWrite, NULL);

  return dwBytesWrite;
}

int serial_read(SERIALOBJECT serial, void* pdata, unsigned int size)
{
  DWORD dwBytesRead = 0;

  if(ReadFile(serial, (uint8_t*)pdata, size, &dwBytesRead, NULL))
  {
    return dwBytesRead;
  }
  return 0;
}

int serial_recv(SERIALOBJECT serial, void* pdata, unsigned int size)
{
  return serial_read(serial, pdata, size);
}

/*
 * Close the serial port.
 */
void serial_close(SERIALOBJECT serial)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  CloseHandle(serial);
}
