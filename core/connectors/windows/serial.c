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
  OVERLAPPED rOverlapped;
  OVERLAPPED wOverlapped;
  unsigned char data[HEADER_SIZE+BUFFER_SIZE];
  unsigned char bread;
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
  HANDLE handle = CreateFile(scom, accessdirection, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
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
    serials[id].rOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    serials[id].wOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    return 0;
  }
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(int id, void* pdata, unsigned int size)
{
  DWORD dwBytesWritten = 0;

  if(!WriteFile(serials[id].handle, (uint8_t*)pdata, size, NULL, &serials[id].wOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      printf("WriteFile failed with error %lu\n", GetLastError());
      return -1;
    }
    int ret = WaitForSingleObject(serials[id].wOverlapped.hEvent, INFINITE);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(serials[id].handle, &serials[id].wOverlapped, &dwBytesWritten, FALSE))
        {
          printf("GetOverlappedResult failed with error %lu\n", GetLastError());
          return -1;
        }
        break;
      default:
        printf("WaitForSingleObject failed with error %lu\n", GetLastError());
        return -1;
    }
  }
  else
  {
    dwBytesWritten = size;
  }

  return dwBytesWritten;
}

int serial_sendv(int id, void* pdata1, unsigned int size1, void* pdata2, unsigned int size2)
{
  if(serial_send(id, pdata1, size1) != size1)
  {
    return -1;
  }
  if(serial_send(id, pdata2, size2) != size2)
  {
    return -1;
  }
  return 0;
}

/*
 * This function tries to read 'size' bytes of data.
 * It blocks until 'size' bytes of data have been read or after 1s has elapsed.
 *
 * It should only be used in the initialization stages, i.e. before the mainloop.
 */
int serial_recv(int id, void* pdata, unsigned int size)
{
  DWORD dwBytesRead = 0;

  if(!ReadFile(serials[id].handle, (uint8_t*)pdata, size, NULL, &serials[id].rOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      printf("ReadFile failed with error %lu\n", GetLastError());
      return -1;
    }
    int ret = WaitForSingleObject(serials[id].rOverlapped.hEvent, INFINITE);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(serials[id].handle, &serials[id].rOverlapped, &dwBytesRead, FALSE))
        {
          printf("GetOverlappedResult failed with error %lu\n", GetLastError());
          return -1;
        }
        break;
      default:
        printf("WaitForSingleObject failed with error %lu\n", GetLastError());
        return -1;
    }
  }
  else
  {
    dwBytesRead = size;
  }

  return dwBytesRead;
}

/*
 * Close the serial port.
 */
int serial_close(int id)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  CloseHandle(serials[id].wOverlapped.hEvent);
  CloseHandle(serials[id].rOverlapped.hEvent);
  CloseHandle(serials[id].handle);
  return 0;
}

static int read_header(int id)
{
  int ret = 0;

  if(!ReadFile(serials[id].handle, (uint8_t*)serials[id].data, HEADER_SIZE, NULL, &serials[id].rOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      fprintf(stderr, "ReadFile failed with error %lu\n", GetLastError());
      ret = -1;
    }
  }

  return ret;
}

static int serial_read_callback(int id)
{
  int ret = 0;

  DWORD dwBytesRead = 0;

  if (!GetOverlappedResult(serials[id].handle, &serials[id].rOverlapped, &dwBytesRead, FALSE))
  {
    printf("GetOverlappedResult failed with error %lu\n", GetLastError());
    return -1;
  }

  serials[id].bread += dwBytesRead;

  DWORD remaining;

  if(serials[id].bread < HEADER_SIZE)
  {
    remaining = HEADER_SIZE - serials[id].bread;
  }
  else
  {
    remaining = serials[id].data[1] - (serials[id].bread - HEADER_SIZE);
  }

  if(remaining > 0)
  {
    if(!ReadFile(serials[id].handle, (uint8_t*)serials[id].data+serials[id].bread, remaining, NULL, &serials[id].rOverlapped))
    {
      if(GetLastError() != ERROR_IO_PENDING)
      {
        fprintf(stderr, "ReadFile failed with error %lu\n", GetLastError());
        ret = -1;
      }
    }
  }
  else
  {
    if(serials[id].data[0] == BYTE_SPOOF_DATA)
    {
      ret = adapter_forward_data_out(id, serials[id].data+HEADER_SIZE, serials[id].data[1]);
    }
    else
    {
      fprintf(stderr, "unhandled packet (type=0x%02x)\n", serials[id].data[0]);
    }

    serials[id].bread = 0;

    read_header(id);
  }

  return ret;
}

void serial_add_source(int id)
{
  /*
   * Disable the timeouts.
   */
  COMMTIMEOUTS timeouts = { 0 };
  if (!SetCommTimeouts(serials[id].handle, &timeouts))
  {
    fprintf(stderr, "can't set timeout\n");
  }

  read_header(id);

  GE_AddSourceHandle(serials[id].rOverlapped.hEvent, id, serial_read_callback, NULL, serial_close);
}
