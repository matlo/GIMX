/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/serial.h>
#include <winsock2.h>
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
        /*
         * disable timeouts
         */
        COMMTIMEOUTS timeouts = { 0 };
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
 * Send 'size' bytes of data.
 * This function blocks until the write completes.
 * 
 * Writing on a serial port that only has rx/tx lines should be fast since there's no feedback.
 */
int serial_send(int id, void* pdata, unsigned int size)
{
  DWORD dwBytesWritten = 0;

  if(!WriteFile(serials[id].handle, (uint8_t*)pdata, size, NULL, &serials[id].wOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      fprintf(stderr, "WriteFile failed with error %lu\n", GetLastError());
      return -1;
    }
    int ret = WaitForSingleObject(serials[id].wOverlapped.hEvent, INFINITE);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(serials[id].handle, &serials[id].wOverlapped, &dwBytesWritten, FALSE))
        {
          fprintf(stderr, "GetOverlappedResult failed with error %lu\n", GetLastError());
          return -1;
        }
        break;
      default:
        fprintf(stderr, "WaitForSingleObject failed with error %lu\n", GetLastError());
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
      fprintf(stderr, "ReadFile failed with error %lu\n", GetLastError());
      return -1;
    }
    int ret = WaitForSingleObject(serials[id].rOverlapped.hEvent, 1000);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(serials[id].handle, &serials[id].rOverlapped, &dwBytesRead, FALSE))
        {
          fprintf(stderr, "GetOverlappedResult failed with error %lu\n", GetLastError());
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        CancelIo(serials[id].handle);
        fprintf(stderr, "WaitForSingleObject failed: timeout expired.\n");
        break;
      default:
        fprintf(stderr, "WaitForSingleObject failed with error %lu\n", GetLastError());
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

/*
 * This function starts an overlapped read.
 * If the read completes immediately, it returns the number of transferred bytes, which is the number of requested bytes.
 * If the read is pending, it returns -1.
 */
static int start_overlapped_read(int id, int remaining)
{
  int ret = -1;
  
  if(!ReadFile(serials[id].handle, (uint8_t*)serials[id].data+serials[id].bread, remaining, NULL, &serials[id].rOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      fprintf(stderr, "ReadFile failed with error %lu\n", GetLastError());
      ret = -1;
    }
  }
  else
  {
    // the read immediately completed
    serials[id].bread += remaining;
    
    ret = remaining;
  }
  
  return ret;
}

/*
 * This function checks if a packet value is available.
 * If a packet value is available, it returns the size of the value (which can be 0).
 * If no packet value is available yet, it returns -1.
 */
static int read_value(int id)
{
  int remaining = serials[id].data[1] - (serials[id].bread - HEADER_SIZE);
  
  if(remaining < 1)
  {
    return serials[id].data[1];
  }

  return start_overlapped_read(id, remaining);
}

/*
 * This function checks if a packet header is available.
 * If a packet header is available, it returns HEADER_SIZE.
 * If no packet header is available yet, it returns -1.
 */
static int read_header(int id)
{
  int remaining = HEADER_SIZE - serials[id].bread;
  
  if(remaining < 1)
  {
    return HEADER_SIZE;
  }

  return start_overlapped_read(id, remaining);
}

int read_packet(int id)
{
  int ret = read_header(id);
  
  /*
   * Since we are reading with no timeout,
   * a positive value indicates that the header is complete.
   */
  if(ret > 0)
  {
    ret = read_value(id);

    /*
     * Since we are reading with no timeout,
     * a non-negative value indicates that the value is complete.
     */
    if(ret >= 0)
    {
      ret = adapter_process_packet(id, serials[id].data);

      serials[id].bread = 0;
    }
  }
  
  return ret;
}

static int serial_read_callback(int id)
{
  DWORD dwBytesRead = 0;

  if (!GetOverlappedResult(serials[id].handle, &serials[id].rOverlapped, &dwBytesRead, FALSE))
  {
    fprintf(stderr, "GetOverlappedResult failed with error %lu\n", GetLastError());
    return -1;
  }

  serials[id].bread += dwBytesRead;
  
  while(read_packet(id) >= 0) ;
  
  return 0;
}

void serial_add_source(int id)
{
  while(read_packet(id) >= 0) ;

  GE_AddSourceHandle(serials[id].rOverlapped.hEvent, id, serial_read_callback, NULL, serial_close);
}
