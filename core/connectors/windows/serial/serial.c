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

static struct
{
  HANDLE handle;
  OVERLAPPED rOverlapped;
  OVERLAPPED wOverlapped;
  s_packet packet;
  unsigned char bread;
} serials[MAX_CONTROLLERS] = {};

/*
 * \brief Initialize all handles to NULL.
 */
void serial_init()
{
  int i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    serials[i].handle = NULL;
  }
}

/*
 * The baud rate in bps.
 */
static int baudrate = 500000;

/*
 * \brief Open a serial port. The serial port is registered for further operations.
 *
 * \param id        the instance id
 * \param portname  the serial port to open, e.g. COM9
 *
 * \return 0 in case of a success, -1 in case of an error
 */
int serial_open(int id, char* portname)
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
 * \brief Send data to the serial port.
 *
 * This function blocks until the write completes.
 * Writing on a serial port that only has rx/tx lines should be fast since there's no feedback.
 *
 * \param id     the serial port instance
 * \param pdata  a pointer to the data to send
 * \param size   the size in bytes of the data to send
 *
 * \return the number of bytes actually written, or -1 in case of an error
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

/*
 * \brief This function tries to read 'size' bytes of data.
 *
 * It blocks until 'size' bytes of data have been read or after 1s has elapsed.
 * It should only be used in the initialization stages, i.e. before the mainloop.
 *
 * \param id     the instance id
 * \param pdata  the pointer where to store the data
 * \param size   the number of bytes to retrieve
 *
 * \return the number of bytes actually read
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
 * \brief This function closes a serial port.
 *
 * \param id  the instance id
 *
 * \return 0
 */
int serial_close(int id)
{
  if(serials[id].handle != NULL)
  {
    usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
    CloseHandle(serials[id].wOverlapped.hEvent);
    serials[id].wOverlapped.hEvent = NULL;
    CloseHandle(serials[id].rOverlapped.hEvent);
    serials[id].rOverlapped.hEvent = NULL;
    CloseHandle(serials[id].handle);
    serials[id].handle = NULL;
  }
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
  
  if(!ReadFile(serials[id].handle, (uint8_t*)&serials[id].packet+serials[id].bread, remaining, NULL, &serials[id].rOverlapped))
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
  int remaining = serials[id].packet.header.length - (serials[id].bread - sizeof(serials->packet.header));
  
  if(remaining < 1)
  {
    return serials[id].packet.header.length;
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
  int remaining = sizeof(serials->packet.header) - serials[id].bread;
  
  if(remaining < 1)
  {
    return sizeof(serials->packet.header);
  }

  return start_overlapped_read(id, remaining);
}

static int read_packet(int id)
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
      ret = adapter_process_packet(id, &serials[id].packet);

      serials[id].bread = 0;
    }
  }
  
  return ret;
}

/*
 * \brief the serial callback for serial ports that are added as event sources.
 *
 * \param id  the instance id
 *
 * \return 0 in case of a success, -1 in case of an error
 */
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

/*
 * \brief Add a serial port as an event source.
 *
 * \param id  the instance id
 */
void serial_add_source(int id)
{
  while(read_packet(id) >= 0) ;

  GE_AddSourceHandle(serials[id].rOverlapped.hEvent, id, serial_read_callback, NULL, serial_close);
}
