/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serialasync.h>

#include <stdio.h>
#include <unistd.h>

static int set_serial_params(int device, unsigned int baudrate) {
  
  /*
   * disable timeouts
   */
  COMMTIMEOUTS * prevTimeouts = &devices[device].serial.prevTimeouts;
  if (GetCommTimeouts(devices[device].handle, prevTimeouts) == 0) {
      ASYNC_PRINT_ERROR("GetCommTimeouts")
      return -1;
  }
  devices[device].serial.restoreTimeouts = 1;
  COMMTIMEOUTS newTimeouts = { 0 };
  if (SetCommTimeouts(devices[device].handle, &newTimeouts) == 0) {
      ASYNC_PRINT_ERROR("SetCommTimeouts")
      return -1;
  }
  /*
   * set baudrate
   */
  DCB * prevSerialParams = &devices[device].serial.prevParams;
  prevSerialParams->DCBlength = sizeof(*prevSerialParams);
  if (GetCommState(devices[device].handle, prevSerialParams) == 0) {
    ASYNC_PRINT_ERROR("GetCommState")
    return -1;
  }
  devices[device].serial.restoreParams = 1;
  DCB newSerialParams = *prevSerialParams;
  newSerialParams.BaudRate = baudrate;
  newSerialParams.ByteSize = 8;
  newSerialParams.StopBits = ONESTOPBIT;
  newSerialParams.Parity = NOPARITY;
  if (SetCommState(devices[device].handle, &newSerialParams) == 0) {
    ASYNC_PRINT_ERROR("SetCommState")
    return -1;
  }
  return 0;
}

/*
 * \brief Open a serial device. The serial device is registered for further operations.
 *
 * \param port     the serial device to open, e.g. COM9
 * \param baudrate the baudrate in bytes per second
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. no device found).
 */
int serialasync_open(char * port, unsigned int baudrate) {

  char scom[sizeof("\\\\.\\") + strlen(port)];
  snprintf(scom, sizeof(scom), "\\\\.\\%s", port);
  
  int device = async_open_path(scom, 1);
  if(device < 0) {
    return -1;
  }
  
  if(set_serial_params(device, baudrate) < 0) {
    async_close(device);
    return -1;
  }
  
  return 0;
}

/*
 * \brief Read from a serial device, with a timeout. Use this function in a synchronous context.
 *
 * \param device  the identifier of the serial device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in seconds
 *
 * \return the number of bytes actually read
 */
int serialasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  return async_read_timeout(device, buf, count, timeout);
}

/*
 * \brief Set the amount of bytes for the next read operation.
 *
 * \param device  the identifier of the serial device
 * \param size   the maximum number of bytes to read
 *
 * \return 0 in case of success, or -1 in case of error
 */
int serialasync_set_read_size(int device, unsigned int size) {

  return async_set_read_size(device, size);
}

/*
 * \brief Register the device as an event source, and set the external callbacks. \
 * This function triggers an asynchronous context. \
 * The fp_read callback is responsible for setting the next read size.
 *
 * \param device      the serial device
 * \param user        the user to pass to the external callback
 * \param fp_read     the external callback to call on data reception
 * \param fp_write    the external callback to call on write completion
 * \param fp_close    the external callback to call on failure
 * \param fp_register the function to register the device as an event source
 *
 * \return 0 in case of success, or -1 in case of error
 */
int serialasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {

    return async_register(device, user, fp_read, fp_write, fp_close, fp_register);
}

/*
 * \brief Write to a serial device, with a timeout. Use this function in a synchronous context. \
 * In case of timeout, the function request the cancellation of the write operation, \
 * and _blocks_ until either the cancellation or the write operation succeeds. \
 * Therefore don't expect the timeout to be very precise.
 *
 * \param device  the identifier of the serial device
 * \param buf     the buffer containing the data to write
 * \param count   the number of bytes in buf
 * \param timeout the maximum time to wait for the completion, in seconds
 *
 * \return the number of bytes actually written (0 in case of timeout)
 */
int serialasync_write_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

    return async_write_timeout(device, buf, count, timeout);
}

/*
 * \brief Send data to a serial device. Use this function in an asynchronous context.
 *
 * \param device  the identifier of the serial device
 * \param buf     the buffer containing the data to send
 * \param count   the maximum number of bytes to send
 *
 * \return -1 in case of error, 0 in case of pending write, or the number of bytes written
 */
int serialasync_write(int device, const void * buf, unsigned int count) {

    return async_write(device, buf, count);
}

/*
 * \brief This function closes a serial device.
 *
 * \param device  the serial device
 *
 * \return 0 in case of a success, -1 in case of an error
 */
int serialasync_close(int device) {

    usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent

    if (devices[device].serial.restoreParams && SetCommState(devices[device].handle, &devices[device].serial.prevParams) == 0) {
        ASYNC_PRINT_ERROR("SetCommState")
    }
    if(devices[device].serial.restoreTimeouts && SetCommTimeouts(devices[device].handle, &devices[device].serial.prevTimeouts) == 0) {
        ASYNC_PRINT_ERROR("SetCommTimeouts")
    }

    return async_close(device);
}
