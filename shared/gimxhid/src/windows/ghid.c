/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <windows.h>
#include <Hidsdi.h>
#include <Setupapi.h>
#include <gimxhid/include/ghid.h>
#include <gimxcommon/include/gerror.h>
#include <gimxcommon/include/async.h>

int ghid_init() {

    return async_init();
}

int ghid_exit() {

    return async_exit();
}

int open_path(const char * path, int print) {

  int device = async_open_path(path, print);
  if(device >= 0) {
    HANDLE * handle = async_get_handle(device);
    HIDD_ATTRIBUTES attributes = { .Size = sizeof(HIDD_ATTRIBUTES) };
    if(HidD_GetAttributes(handle, &attributes) == TRUE) {
        PHIDP_PREPARSED_DATA preparsedData;
        HIDP_CAPS hidCapabilities;
        if(HidD_GetPreparsedData(handle, &preparsedData) == TRUE) {
            if(HidP_GetCaps(preparsedData, &hidCapabilities) == HIDP_STATUS_SUCCESS ) {
                s_hid_info * hid_info = (s_hid_info *) calloc(1, sizeof(*hid_info));
                if (hid_info != NULL) {
                    hid_info->vendor_id = attributes.VendorID;
                    hid_info->product_id = attributes.ProductID;
                    hid_info->bcdDevice = attributes.VersionNumber;
                    async_set_private(device, hid_info);
                    async_set_write_size(device, hidCapabilities.OutputReportByteLength);
                    async_set_read_size(device, hidCapabilities.InputReportByteLength);
                    async_set_device_type(device, E_ASYNC_DEVICE_TYPE_HID);
                } else {
                    PRINT_ERROR_ALLOC_FAILED("malloc")
                    async_close(device);
                    device = -1;
                }
            }
            else {
                if (print) {
                    PRINT_ERROR_OTHER("HidP_GetCaps")
                }
                async_close(device);
                device = -1;
            }
            HidD_FreePreparsedData(preparsedData);
        }
        else {
            if (print) {
                PRINT_ERROR_OTHER("HidD_GetPreparsedData")
            }
            async_close(device);
            device = -1;
        }
    }
    else {
        if (print) {
            PRINT_ERROR_OTHER("HidD_GetAttributes")
        }
        async_close(device);
        device = -1;
    }
  }
  return device;
}

struct ghid_device * ghid_enumerate(unsigned short vendor, unsigned short product) {

  struct ghid_device * devs = NULL;

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
				PRINT_ERROR_ALLOC_FAILED("calloc")
				continue;
			}
			details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if(SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL) == FALSE) {
				PRINT_ERROR_GETLASTERROR("SetupDiGetDeviceInterfaceDetail")
				free(details);
				continue;
			}
			int device = open_path(details->DevicePath, 0);
			free(details);

			if(device >= 0) {
			    s_hid_info * hid_info = (s_hid_info *) async_get_private(device);
			    if (hid_info == NULL) {
			        async_close(device);
			        continue;
			    }
				if(vendor) {
					if (hid_info->vendor_id != vendor) {
						async_close(device);
						continue;
					}
					if(product) {
						if(hid_info->product_id != product) {
							async_close(device);
							continue;
						}
					}
				}

				char * path = strdup(async_get_path(device));

				if(path == NULL) {
					PRINT_ERROR_OTHER("strdup failed")
	                async_close(device);
					continue;
				}

		        void * ptr = malloc(sizeof(*devs));
		        if (ptr == NULL) {
		            PRINT_ERROR_ALLOC_FAILED("malloc")
		            free(path);
                    async_close(device);
		            continue;
		        }

		        struct ghid_device * dev = ptr;

		        dev->path = path;
                dev->vendor_id = hid_info->vendor_id;
                dev->product_id = hid_info->product_id;
                dev->bcdDevice = hid_info->bcdDevice;
                dev->interface_number = -1;
                char * interface = strstr(path, "&mi_");
                if (interface != NULL) {
                    sscanf(interface + 4, "%02x", &dev->interface_number);
                }
                dev->next = NULL;

		        struct ghid_device * current;
		        struct ghid_device * previous = NULL;
		        for (current = devs; current != NULL; current = current->next) {
		            if (strcmp(dev->path, current->path) < 0) {
		                if (previous != NULL) {
		                    previous->next = dev;
		                } else {
		                    devs = dev;
		                }
		                dev->next = current;
		                break;
		            }
		            previous = current;
		        }

		        if (current == NULL) {
		            if (devs == NULL) {
		                devs = dev;
		            } else {
		                previous->next = dev;
		            }
		        }

				async_close(device);
			}
		}
	}

  return devs;
}

void ghid_free_enumeration(struct ghid_device * devs) {

    struct ghid_device * current = devs;
    while (current != NULL) {
        struct ghid_device * next = current->next;
        free(current->path);
        free(current);
        current = next;
    }
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
        PRINT_ERROR_ALLOC_FAILED("calloc")
        continue;
      }
      details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      if(SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL) == FALSE) {
        PRINT_ERROR_GETLASTERROR("SetupDiGetDeviceInterfaceDetail")
        free(details);
        details = NULL;
        continue;
      }
      int device = open_path(details->DevicePath, 0);
      free(details);
      details = NULL;
      if(device >= 0) {
        s_hid_info * hid_info = (s_hid_info *) async_get_private(device);
        if(hid_info != NULL && hid_info->vendor_id == vendor && hid_info->product_id == product)
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

    return (s_hid_info *) async_get_private(device);
}

/*
 * \brief Close a hid device.
 *
 * \param device  the identifier of the hid device to close.
 *
 * \return 0 in case of success, or -1 in case of failure (i.e. bad device identifier).
 */
int ghid_close(int device) {

    free(async_get_private(device));

    return async_close(device);
}

/*
 * \brief Read from a hid device, with a timeout. Use this function in a synchronous context.
 *
 * \param device  the identifier of the hid device
 * \param buf     the buffer where to store the data
 * \param count   the maximum number of bytes to read
 * \param timeout the maximum time to wait, in milliseconds
 *
 * \return the number of bytes actually read
 */
int ghid_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  return async_read_timeout(device, buf, count, timeout);
}

/*
 * \brief Register the device as an event source, and set the external callbacks. \
 * This function triggers an asynchronous context.
 *
 * \param device      the hid device
 * \param user        the user to pass to the external callback
 * \param callbacks   the device callbacks
 *
 * \return 0 in case of success, or -1 in case of error
 */
int ghid_register(int device, int user, const GHID_CALLBACKS * callbacks) {

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

  return async_write(device, buf, count);
}

int ghid_poll(int device __attribute__((unused))) {

    return 0;
}
