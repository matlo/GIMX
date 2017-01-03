/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gserial.h>
#include <gimxcommon/include/gerror.h>
#include <gimxcommon/include/async.h>

#include <stdio.h>
#include <unistd.h>

int gserial_init() {

    return async_init();
}

int gserial_exit() {

    return async_exit();
}

typedef struct {
    unsigned char restoreParams;
    DCB prevParams;
    unsigned char restoreTimeouts;
    COMMTIMEOUTS prevTimeouts;
} s_serial_params;

static int set_serial_params(int device, unsigned int baudrate) {

    s_serial_params * params = (s_serial_params *) calloc(1, sizeof(*params));

    if (params == NULL) {
        PRINT_ERROR_ALLOC_FAILED("malloc")
        return -1;
    }

    HANDLE * handle = async_get_handle(device);

    /*
     * disable timeouts
     */
    if (GetCommTimeouts(handle, &params->prevTimeouts) == 0) {
        PRINT_ERROR_GETLASTERROR("GetCommTimeouts")
        free(params);
        return -1;
    }
    params->restoreTimeouts = 1;
    COMMTIMEOUTS newTimeouts = { 0 };
    if (SetCommTimeouts(handle, &newTimeouts) == 0) {
        PRINT_ERROR_GETLASTERROR("SetCommTimeouts")
        free(params);
        return -1;
    }
    /*
     * set baudrate
     */
    params->prevParams.DCBlength = sizeof(params->prevParams);
    if (GetCommState(handle, &params->prevParams) == 0) {
        PRINT_ERROR_GETLASTERROR("GetCommState")
        free(params);
        return -1;
    }
    params->restoreParams = 1;
    DCB newSerialParams = params->prevParams;
    newSerialParams.BaudRate = baudrate;
    newSerialParams.ByteSize = 8;
    newSerialParams.StopBits = ONESTOPBIT;
    newSerialParams.Parity = NOPARITY;
    if (SetCommState(handle, &newSerialParams) == 0) {
        PRINT_ERROR_GETLASTERROR("SetCommState")
        free(params);
        return -1;
    }
    async_set_private(device, params);
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
int gserial_open(const char * port, unsigned int baudrate) {

    char scom[sizeof("\\\\.\\") + strlen(port)];
    snprintf(scom, sizeof(scom), "\\\\.\\%s", port);

    int device = async_open_path(scom, 1);
    if (device < 0) {
        return -1;
    }

    async_set_device_type(device, E_ASYNC_DEVICE_TYPE_SERIAL);

    if (set_serial_params(device, baudrate) < 0) {
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
 * \param timeout the maximum time to wait, in milliseconds
 *
 * \return the number of bytes actually read
 */
int gserial_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

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
int gserial_set_read_size(int device, unsigned int size) {

    return async_set_read_size(device, size);
}

/*
 * \brief Register the device as an event source, and set the external callbacks. \
 * This function triggers an asynchronous context. \
 * The fp_read callback is responsible for setting the next read size.
 *
 * \param device      the serial device
 * \param user        the user to pass to the external callback
 * \param callbacks   the device callbacks
 *
 * \return 0 in case of success, or -1 in case of error
 */
int gserial_register(int device, int user, const GSERIAL_CALLBACKS * callbacks) {

    ASYNC_CALLBACKS async_callbacks = {
            .fp_read = callbacks->fp_read,
            .fp_write = callbacks->fp_write,
            .fp_close = callbacks->fp_close,
            .fp_register = callbacks->fp_register,
            .fp_remove = callbacks->fp_remove,
    };
    return async_register(device, user, &async_callbacks);
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
int gserial_write_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

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
int gserial_write(int device, const void * buf, unsigned int count) {

    return async_write(device, buf, count);
}

/*
 * \brief This function closes a serial device.
 *
 * \param device  the serial device
 *
 * \return 0 in case of a success, -1 in case of an error
 */
int gserial_close(int device) {

    usleep(10000); //sleep 10ms to leave enough time for the last packet to be sent

    HANDLE * handle = async_get_handle(device);
    s_serial_params * params = (s_serial_params *) async_get_private(device);

    if (handle != NULL && params != NULL) {
        if (params->restoreParams && SetCommState(handle, &params->prevParams) == 0) {
            PRINT_ERROR_GETLASTERROR("SetCommState")
        }
        if (params->restoreTimeouts && SetCommTimeouts(handle, &params->prevTimeouts) == 0) {
            PRINT_ERROR_GETLASTERROR("SetCommTimeouts")
        }
    }
    
    free(params);

    return async_close(device);
}
