/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <hidasync.h>

#include <stdio.h>
#include <Hidsdi.h>
#include <Setupapi.h>

int open_path(const char * path, int print) {
  
  int device = async_open_path(path, print);
  if(device >= 0) {
    HIDD_ATTRIBUTES attributes = { .Size = sizeof(HIDD_ATTRIBUTES) };
    if(HidD_GetAttributes(devices[device].handle, &attributes) == TRUE) {
        PHIDP_PREPARSED_DATA preparsedData;
        HIDP_CAPS hidCapabilities;
        if(HidD_GetPreparsedData(devices[device].handle, &preparsedData) == TRUE) {
            if(HidP_GetCaps(preparsedData, &hidCapabilities) == HIDP_STATUS_SUCCESS ) {
                devices[device].write.size = hidCapabilities.OutputReportByteLength;
                devices[device].hid.vendor = attributes.VendorID;
                devices[device].hid.product = attributes.ProductID;
            }
            else {
                ASYNC_PRINT_ERROR("HidP_GetCaps")
                async_close(device);
                device = -1;
            }
            HidD_FreePreparsedData(preparsedData);
        }
        else {
            ASYNC_PRINT_ERROR("HidD_GetPreparsedData")
            async_close(device);
            device = -1;
        }
    }
    else {
        ASYNC_PRINT_ERROR("HidD_GetAttributes")
        async_close(device);
        device = -1;
    }
  }
  return device;
}

/*
 * \brief Open a hid device.
 *
 * \param path  the path of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. bad path, device already opened).
 */
int hidasync_open_path(const char * path) {
    
  return open_path(path, 1);
}

/*
 * \brief Open a hid device.
 *
 * \param vendor   the vendor id of the hid device to open.
 * \param product  the product id of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. no device found).
 */
int hidasync_open_ids(unsigned short vendor, unsigned short product)
{
  int ret = -1;

  GUID guid;
  HDEVINFO info;
  DWORD reqd_size;
  SP_DEVICE_INTERFACE_DATA iface;
  SP_DEVICE_INTERFACE_DETAIL_DATA *details;
  int index;
  
  HidD_GetHidGuid(&guid);
  info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if(info != INVALID_HANDLE_VALUE) {
    for(index = 0; ; ++index) {
	    iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
      if(SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface) == FALSE) {
        break; //no more device
      }
      if(SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL) == FALSE) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
          continue;
	      }
      }
      details = calloc(reqd_size, sizeof(char));
      if(details == NULL) {
        fprintf(stderr, "%s:%d calloc failed\n", __FILE__, __LINE__);
        continue;
      }
      details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      if(SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL) == FALSE) {
        ASYNC_PRINT_ERROR("SetupDiGetDeviceInterfaceDetail")
        free(details);
        details = NULL;
        continue;
      }
      int device = open_path(details->DevicePath, 0);
      free(details);
      details = NULL;
      if(device >= 0) {
        if(devices[device].hid.vendor == vendor && devices[device].hid.product == product)
        {
          ret = device;
          break;
        }
        async_close(device);
      }
    }
  }

  return ret;
}

/*
 * \brief Get the USB ids of a hid device.
 *
 * \param device  the identifier of the hid device
 * \param vendor  where to store the vendor id
 * \param product where to store the product id
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int hidasync_get_ids(int device, unsigned short * vendor, unsigned short * product) {

    ASYNC_CHECK_DEVICE(device)

    *vendor = devices[device].hid.vendor;
    *product = devices[device].hid.product;

    return 0;
}

/*
 * \brief Close a hid device.
 *
 * \param device  the identifier of the hid device to close.
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int hidasync_close(int device) {

    return async_close(device);
}

/*
 * \brief Read from a hid device, with a timeout. Use this function in a synchronous context.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in seconds
 *
 * \return the number of bytes actually read
 */
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  return async_read_timeout(device, buf, count, timeout);
}

/*
 * \brief Register the device as an event source, and set the external callbacks. \
 * This function triggers an asynchronous context.
 *
 * \param device      the hid device
 * \param user        the user to pass to the external callback
 * \param fp_read     the external callback to call on data reception
 * \param fp_write    the external callback to call on write completion
 * \param fp_close    the external callback to call on failure
 * \param fp_register the function to register the device as an event source
 *
 * \return 0 in case of success, or -1 in case of error
 */
int hidasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {

    if(async_set_read_size(device, HIDASYNC_MAX_TRANSFER_SIZE) < 0) {
      
      return -1;
    }
    
    return async_register(device, user, fp_read, fp_write, fp_close, fp_register);
}

/*
 * \brief Write to a hid device, with a timeout. Use this function in a synchronous context. \
 * In case of timeout, the function request the cancellation of the write operation, \
 * and _blocks_ until either the cancellation or the write operation succeeds. \
 * Therefore don't expect the timeout to be very precise.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer containing the data to write
 * \param count   the number of bytes in buf
 * \param timeout the maximum time to wait for the completion, in seconds
 *
 * \return the number of bytes actually written (0 in case of timeout)
 */
int hidasync_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

  return async_write_timeout(device, buf, count, timeout);
}

/*
 * \brief Send data to a hid device. Use this function in an asynchronous context.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer containing the data to send
 * \param count   the maximum number of bytes to send
 *
 * \return -1 in case of error, 0 in case of pending write, or the number of bytes written
 */
int hidasync_write(int device, const void * buf, unsigned int count) {

    return async_write(device, buf, count);
}
