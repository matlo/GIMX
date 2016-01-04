/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <windows.h>
#include <Hidsdi.h>
#include <Setupapi.h>
#include <ghid.h>

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

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
                async_set_read_size(device, hidCapabilities.InputReportByteLength);
                devices[device].hidInfo.vendor_id = attributes.VendorID;
                devices[device].hidInfo.product_id = attributes.ProductID;
                devices[device].hidInfo.bcdDevice = attributes.VersionNumber;
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

s_hid_dev * ghid_enumerate(unsigned short vendor, unsigned short product) {

  s_hid_dev * hid_devs = NULL;
  unsigned int nb_hid_devs = 0;

  GUID guid;
	HidD_GetHidGuid(&guid);

	HDEVINFO info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if(info != INVALID_HANDLE_VALUE) {
		int index;
		for(index = 0; ; ++index) {
			SP_DEVICE_INTERFACE_DATA iface;
			iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if(SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface) == FALSE) {
				break; //no more device
			}
			DWORD reqd_size;
			if(SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL) == FALSE) {
				if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
					continue;
				}
			}
			SP_DEVICE_INTERFACE_DETAIL_DATA * details = calloc(reqd_size, sizeof(char));
			if(details == NULL) {
				fprintf(stderr, "%s:%d calloc failed\n", __FILE__, __LINE__);
				continue;
			}
			details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if(SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL) == FALSE) {
				ASYNC_PRINT_ERROR("SetupDiGetDeviceInterfaceDetail")
				free(details);
				continue;
			}
			int device = open_path(details->DevicePath, 0);
			free(details);

			if(device >= 0) {
				if(vendor) {
					if (devices[device].hidInfo.vendor_id != vendor) {
						async_close(device);
						continue;
					}
					if(product) {
						if(devices[device].hidInfo.product_id != product) {
							async_close(device);
							continue;
						}
					}
				}

				char * path = strdup(devices[device].path);

				if(path == NULL) {
					PRINT_ERROR_OTHER("strdup failed")
	        async_close(device);
					continue;
				}

				void * ptr = realloc(hid_devs, (nb_hid_devs + 1) * sizeof(*hid_devs));

				if(ptr == NULL) {
					PRINT_ERROR_ALLOC_FAILED("realloc")
					free(path);
					async_close(device);
					continue;
				}

				hid_devs = ptr;

				if(nb_hid_devs > 0) {
					hid_devs[nb_hid_devs - 1].next = 1;
				}

				hid_devs[nb_hid_devs].path = path;
				hid_devs[nb_hid_devs].vendor_id = devices[device].hidInfo.vendor_id;
				hid_devs[nb_hid_devs].product_id = devices[device].hidInfo.product_id;
				hid_devs[nb_hid_devs].next = 0;

				++nb_hid_devs;

				async_close(device);
			}
		}
	}

  return hid_devs;
}

void ghid_free_enumeration(s_hid_dev * hid_devs) {

  s_hid_dev * current;
  for(current = hid_devs; current != NULL; ++current) {

      free(current->path);

      if(current->next == 0) {
          break;
      }
  }
  free(hid_devs);
}

/*
 * \brief Open a hid device.
 *
 * \param path  the path of the hid device to open.
 *
 * \return the identifier of the opened device (to be used in further operations), \
 * or -1 in case of failure (e.g. bad path, device already opened).
 */
int ghid_open_path(const char * path) {
    
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
int ghid_open_ids(unsigned short vendor, unsigned short product)
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
        if(devices[device].hidInfo.vendor_id == vendor && devices[device].hidInfo.product_id == product)
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
 * \brief Get info for a hid device.
 *
 * \param device  the identifier of the hid device
 *
 * \return the hid info
 */
const s_hid_info * ghid_get_hid_info(int device) {

    ASYNC_CHECK_DEVICE(device, NULL)

    return &devices[device].hidInfo;
}

/*
 * \brief Close a hid device.
 *
 * \param device  the identifier of the hid device to close.
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int ghid_close(int device) {

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
int ghid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  if(devices[device].read.size == 0) {

    PRINT_ERROR_OTHER("the device has no HID IN endpoint")
    return -1;
  }

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
int ghid_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {
    
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
 * \return the number of bytes actually written (0 in case of timeout, -1 in case of error)
 */
int ghid_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

  if(devices[device].write.size == 0) {

    PRINT_ERROR_OTHER("the device has no HID OUT endpoint")
    return -1;
  }

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
int ghid_write(int device, const void * buf, unsigned int count) {

  if(devices[device].write.size == 0) {

    PRINT_ERROR_OTHER("the device has no HID OUT endpoint")
    return -1;
  }

  return async_write(device, buf, count);
}
