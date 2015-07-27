/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "connectors/hidasync.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <Hidsdi.h>
#include <dirent.h>
#include <Setupapi.h>

static struct {
    HANDLE handle;
    OVERLAPPED rOverlapped;
    OVERLAPPED wOverlapped;
    char * buf[HIDASYNC_MAX_TRANSFER_SIZE];
    unsigned int bread;
    unsigned short vendor;
    unsigned short product;
    char * path;
    struct {
        int user;
        int (*fp_read)(int, const char * buf, unsigned int count);
    } callback;
} devices[HIDASYNC_MAX_DEVICES] = { };

#define CHECK_DEVICE(device) \
    if(device < 0 || device >= HIDASYNC_MAX_DEVICES || devices[device].handle == INVALID_HANDLE_VALUE) { \
        fprintf(stderr, "%s:%d %s: no such device (%d)\n", __FILE__, __LINE__, __func__, device); \
        return -1; \
    }

static int add_device(const char * device_path, HANDLE handle, unsigned short vendor, unsigned short product) {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        if(devices[i].path && !strcmp(devices[i].path, device_path)) {
            fprintf(stderr, "%s:%d add_device %s: device already opened\n", __FILE__, __LINE__, device_path);
            return -1;
        }
    }
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        if(devices[i].handle == INVALID_HANDLE_VALUE) {
            devices[i].path = strdup(device_path);
            if(devices[i].path != NULL) {
                devices[i].handle = handle;
                devices[i].vendor = vendor;
                devices[i].product = product;
                return i;
            }
            else {
                fprintf(stderr, "%s:%d add_device %s: can't duplicate path\n", __FILE__, __LINE__, device_path);
                return -1;
            }
        }
    }
    return -1;
}

/*
 * \brief Initialize internal structures. \
 * This function has to be called once before using any other hidasync function.
 */
void hidsaync_init() {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        devices[i].handle = INVALID_HANDLE_VALUE;
    }
}

/*
 * \brief Close all opened devices.
 */
void hidasync_quit() {
    int i;
    for (i = 0; i < HIDASYNC_MAX_DEVICES; ++i) {
        hidasync_close(i);
    }
}

static int hidasync_set_overlapped(int device) {
    /*
     * disable timeouts
     */
    COMMTIMEOUTS timeouts = { 0 };
    if (SetCommTimeouts(devices[device].handle, &timeouts) == FALSE) {
        //TODO MLA: log error
        return -1;
    }
    /*
     * create event objects for overlapped I/O
     */
    devices[device].rOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(devices[device].rOverlapped.hEvent == INVALID_HANDLE_VALUE) {
        //TODO MLA: log error
        return -1;
    }
    devices[device].wOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(devices[device].wOverlapped.hEvent == INVALID_HANDLE_VALUE) {
        //TODO MLA: log error
        return -1;
    }
    return 0;
}

/*
 * \brief Open a hid device.
 *
 * \param device_path  the path of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. bad path, device already opened).
 */
int hidasync_open_path(const char * device_path) {
    DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
    int ret = -1;
    if(device_path != NULL) {
        HANDLE handle = CreateFile(device_path, accessdirection, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
        if(handle != INVALID_HANDLE_VALUE) {
            HIDD_ATTRIBUTES attributes = { .Size = sizeof(HIDD_ATTRIBUTES) };
            if(HidD_GetAttributes(handle, &attributes) == TRUE) {
                ret = add_device(device_path, handle, attributes.VendorID, attributes.ProductID);
                if(ret == -1) {
                    CloseHandle(handle);
                }
                else if(hidasync_set_overlapped(ret) == -1) {
                    hidasync_close(ret);
                    ret = -1;
                }
            }
            else {
                //TODO MLA: log error
                CloseHandle(handle);
            }
        }
        else {
            //TODO MLA: log error
        }
    }
    return ret;
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
  SP_DEVICE_INTERFACE_DATA iface = { .cbSize = sizeof(SP_DEVICE_INTERFACE_DATA) };
  SP_DEVICE_INTERFACE_DETAIL_DATA *details;
  int index;
  
  HidD_GetHidGuid(&guid);
	info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if(info != INVALID_HANDLE_VALUE) {
    for(index = 0; ; ++index) {
      if(SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface) == FALSE) {
        //TODO MLA: log error
        break;
      }
      if(SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL) == FALSE) {
        //TODO MLA: log error
        continue;
      }
      details = calloc(reqd_size, sizeof(char));
      if(details == NULL) {
        fprintf(stderr, "%s:%d calloc failed\n", __FILE__, __LINE__);
        continue;
      }
      details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      if(SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL) == TRUE) {

        int device = hidasync_open_path(details->DevicePath);

        if(device >= 0) {
          if(devices[device].vendor == vendor && devices[device].product == product)
          {
            ret = device;
            break;
          }
          hidasync_close(device);
        }
      }
      else {
        //TODO MLA: log error
      }
      free(details);
      details = NULL;
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

    CHECK_DEVICE(device)

    *vendor = devices[device].vendor;
    *product = devices[device].product;

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

    CHECK_DEVICE(device)

    if (CloseHandle(devices[device].rOverlapped.hEvent) != 0) {
        //TODO MLA: log error
    }
    devices[device].rOverlapped.hEvent = INVALID_HANDLE_VALUE;
    if (CloseHandle(devices[device].wOverlapped.hEvent) != 0) {
        //TODO MLA: log error
    }
    devices[device].wOverlapped.hEvent = INVALID_HANDLE_VALUE;
    if (CloseHandle(devices[device].handle) != 0) {
        //TODO MLA: log error
    }
    devices[device].handle = INVALID_HANDLE_VALUE;
    free(devices[device].path);
    devices[device].path = NULL;

    return 0;
}

/*
 * \brief Read from a hid device, with a timeout.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in seconds
 *
 * \return the number of bytes actually read
 */
int hidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  DWORD dwBytesRead = 0;

  if(!ReadFile(devices[device].handle, buf, count, NULL, &devices[device].rOverlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      fprintf(stderr, "ReadFile failed with error %lu\n", GetLastError());
      return -1;
    }
    int ret = WaitForSingleObject(devices[device].rOverlapped.hEvent, 1000);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(devices[device].handle, &devices[device].rOverlapped, &dwBytesRead, FALSE))
        {
          fprintf(stderr, "GetOverlappedResult failed with error %lu\n", GetLastError());
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        CancelIo(devices[device].handle);
        fprintf(stderr, "WaitForSingleObject failed: timeout expired.\n");
        break;
      default:
        fprintf(stderr, "WaitForSingleObject failed with error %lu\n", GetLastError());
        return -1;
    }
  }
  else
  {
    dwBytesRead = count;
  }

  return dwBytesRead;
}


/*
 * This function starts an overlapped read.
 * If the read completes immediately, it returns the number of transferred bytes, which is the number of requested bytes.
 * If the read is pending, it returns -1.
 */
static int start_overlapped_read(int device) {
  int ret = -1;
  
  if(!ReadFile(devices[device].handle, devices[device].buf, sizeof(devices[device].buf), NULL, &devices[device].rOverlapped))
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
    ret = devices[device].bread = sizeof(devices[device].buf);
  }
  
  return ret;
}

static int read_packet(int device) {
  
  if(devices[device].bread) {
    devices[device].callback.fp_read(devices[device].callback.user, (const char *)devices[device].buf, devices[device].bread);
    devices[device].bread = 0;
  }

  return start_overlapped_read(device);
}

/*
 * \brief the callback for hid devices that are added as event sources.
 *
 * \param device  the hid device
 *
 * \return 0 in case of a success, -1 in case of an error
 */
static int hidasync_read(int device) {

    CHECK_DEVICE(device)
    
    DWORD dwBytesRead = 0;

    if (!GetOverlappedResult(devices[device].handle, &devices[device].rOverlapped, &dwBytesRead, FALSE))
    {
      fprintf(stderr, "GetOverlappedResult failed with error %lu\n", GetLastError());
      return -1;
    }

    devices[device].bread = dwBytesRead;
    
    while(read_packet(device) >= 0) ;

    return 0;
}

/*
 * \brief Register the device as an event source, and set the external callback to call on data reception.
 *
 * \param device      the hid device
 * \param user        the user to pass to the external callback
 * \param fp_read     the external callback
 * \param fp_register the function to register the device as an event source
 *
 * \return 0 in case of success, or -1 in case of error
 */
int hidasync_register(int device, int user, HIDASYNC_READ_CALLBACK fp_read, HIDASYNC_REGISTER_SOURCE fp_register) {

    CHECK_DEVICE(device)
    
    while(read_packet(device) >= 0) ;

    devices[device].callback.user = user;
    devices[device].callback.fp_read = fp_read;

    fp_register(devices[device].rOverlapped.hEvent, device, hidasync_read, NULL, hidasync_close);

    return 0;
}

/*
 * \brief Send data to a hid device.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer containing the data to send
 * \param count   the maximum number of bytes to send
 *
 * \return 0 in case of success, or -1 in case of error
 */
int hidasync_write(int device, const void * buf, unsigned int count) {

    CHECK_DEVICE(device)

    DWORD dwBytesWritten = 0;

    if(!WriteFile(devices[device].handle, buf, count, NULL, &devices[device].wOverlapped))
    {
      if(GetLastError() != ERROR_IO_PENDING)
      {
        fprintf(stderr, "WriteFile failed with error %lu\n", GetLastError());
        return -1;
      }
      //TODO MLA: process the result asynchronously
    }
    else
    {
      dwBytesWritten = count;
    }

    return dwBytesWritten;
}
