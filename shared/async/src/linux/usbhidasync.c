/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <usbhidasync.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define USBHIDASYNC_MAX_DEVICES 256

typedef struct {
	int configuration;
	struct {
		int number;
		int alternateSetting;
		s_hid_info hidInfo;
	} interface;
	struct {
		struct {
			unsigned char address;
			unsigned short size;
		} in;
		struct {
			unsigned char address;
			unsigned short size;
		} out;
	} endpoints;
} s_config;

static struct {
	char * path;
	libusb_device_handle * devh;
	unsigned short vendor;
	unsigned short product;
	s_config config;
	struct {
		int user;
		ASYNC_READ_CALLBACK fp_read;
		ASYNC_WRITE_CALLBACK fp_write;
		ASYNC_CLOSE_CALLBACK fp_close;
	} callback;
	int pending_transfers;
} usbdevices[USBHIDASYNC_MAX_DEVICES] = { };

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
	return libusb_error_name(errcode);
}
#endif

void print_error_libusb(const char * file, int line, const char * func, const char * libusbfunc, int ret) {

	fprintf(stderr, "%s:%d %s: %s failed with error: %s\n", file, line, func, libusbfunc, libusb_strerror(ret));
}
#define PRINT_ERROR_LIBUSB(libusbfunc,ret) print_error_libusb(__FILE__, __LINE__, __func__, libusbfunc, ret);

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

static libusb_context* ctx = NULL;
static libusb_device** devs = NULL;
static ssize_t cnt = 0;
static int nb_opened = 0;

#define REPORTS_MAX 2

static struct libusb_transfer ** transfers = NULL;
static unsigned int transfers_nb = 0;

static int add_transfer(struct libusb_transfer * transfer) {
	int i;
	for (i = 0; i < transfers_nb; ++i) {
		if (transfers[i] == transfer) {
			return 0;
		}
	}
	void * ptr = realloc(transfers, (transfers_nb + 1) * sizeof(*transfers));
	if (ptr) {
		transfers = ptr;
		transfers[transfers_nb] = transfer;
		transfers_nb++;
		usbdevices[(unsigned long) transfer->user_data].pending_transfers++;
		return 0;
	} else {
		PRINT_ERROR_ALLOC_FAILED("realloc")
		return -1;
	}
}

static void remove_transfer(struct libusb_transfer * transfer) {
	unsigned int i;
	for (i = 0; i < transfers_nb; ++i) {
		if (transfers[i] == transfer) {
			memmove(transfers + i, transfers + i + 1, (transfers_nb - i - 1) * sizeof(*transfers));
			transfers_nb--;
			void * ptr = realloc(transfers, transfers_nb * sizeof(*transfers));
			if (ptr || !transfers_nb) {
				transfers = ptr;
			} else {
				PRINT_ERROR_ALLOC_FAILED("realloc")
			}
			usbdevices[(unsigned long) transfer->user_data].pending_transfers--;
			free(transfer->buffer);
			libusb_free_transfer(transfer);
			break;
		}
	}
}

void usbhidasync_init(void) __attribute__((constructor (101)));
void usbhidasync_init(void) {
	nb_opened = 0;
}

void usbhidasync_clean(void) __attribute__((destructor (101)));
void usbhidasync_clean(void) {
	int i;
	for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
		if (usbdevices[i].devh != NULL) {
			usbhidasync_close(i);
		}
	}
}

inline int usbhidasync_check_device(int device, const char * file, unsigned int line, const char * func) {
	if (device < 0 || device >= USBHIDASYNC_MAX_DEVICES) {
		PRINT_ERROR_OTHER("invalid device")
		return -1;
	}
	if (usbdevices[device].devh == NULL) {
		PRINT_ERROR_OTHER("no such device")
		return -1;
	}
	return 0;
}
#define USBHIDASYNC_CHECK_DEVICE(device,retValue) \
  if(usbhidasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static char * make_path(libusb_device * dev) {
	static char str[] = "00:00";
	snprintf(str, sizeof(str), "%02x:%02x", libusb_get_bus_number(dev), libusb_get_device_address(dev));
	return str;
}

static int add_device(const char * path, s_config * config, int print) {
	int i;
	for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
		if (usbdevices[i].path && !strcmp(usbdevices[i].path, path)) {
			if (print) {
				PRINT_ERROR_OTHER("device already opened")
			}
			return -1;
		}
	}
	for (i = 0; i < USBHIDASYNC_MAX_DEVICES; ++i) {
		if (usbdevices[i].devh == NULL) {
			usbdevices[i].path = strdup(path);
			if (usbdevices[i].path != NULL) {
				usbdevices[i].config = *config;
				++nb_opened;
				return i;
			} else {
				PRINT_ERROR_OTHER("can't duplicate path")
				return -1;
			}
		}
	}
	return -1;
}

static int submit_transfer(struct libusb_transfer * transfer) {
	/*
	 * Don't submit the transfer if it can't be added in the 'transfers' table.
	 * Otherwise it would not be possible to cleanly cancel it.
	 */
	int ret = add_transfer(transfer);

	if (ret != -1) {
		ret = libusb_submit_transfer(transfer);
		if (ret != LIBUSB_SUCCESS) {
			PRINT_ERROR_LIBUSB("libusb_submit_transfer", ret)
			remove_transfer(transfer);
			return -1;
		}
	}
	return ret;
}

static void usb_callback(struct libusb_transfer* transfer);

static int poll_interrupt(int device) {

	unsigned int address = usbdevices[device].config.endpoints.in.address;
	unsigned int size = usbdevices[device].config.endpoints.in.size;

	unsigned char * buf = calloc(size, sizeof(char));
	if (buf == NULL) {

		PRINT_ERROR_ALLOC_FAILED("calloc")
		return -1;
	}

	struct libusb_transfer * transfer = libusb_alloc_transfer(0);
	if (transfer == NULL) {

		PRINT_ERROR_ALLOC_FAILED("libusb_alloc_transfer")
		free(buf);
		return -1;
	}

	libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, address, buf, size,
			(libusb_transfer_cb_fn) usb_callback, (void *) (unsigned long) device, 1000);

	return submit_transfer(transfer);
}

static void usb_callback(struct libusb_transfer* transfer) {
	int device = (unsigned long) transfer->user_data;

	if (transfer->type == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
		if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
			if (transfer->endpoint == usbdevices[device].config.endpoints.in.address) {
				usbdevices[device].callback.fp_read(usbdevices[device].callback.user, transfer->buffer,
						transfer->actual_length);
			} else if (transfer->endpoint == usbdevices[device].config.endpoints.out.address) {
				usbdevices[device].callback.fp_write(usbdevices[device].callback.user);
			}
		} else {
			if (transfer->status != LIBUSB_TRANSFER_TIMED_OUT && transfer->status != LIBUSB_TRANSFER_CANCELLED) {
				fprintf(stderr, "libusb_transfer failed with status %s (endpoint=0x%02x)\n",
						libusb_error_name(transfer->status), transfer->endpoint);
			}
		}

		if (transfer->endpoint == usbdevices[device].config.endpoints.in.address
				&& transfer->status != LIBUSB_TRANSFER_CANCELLED) {

			submit_transfer(transfer);
		} else {

			remove_transfer(transfer);
		}
	}
}

int handle_events(int unused) {
#ifndef WIN32
	return libusb_handle_events(ctx);
#else
	if(ctx != NULL)
	{
		struct timeval tv = {};
		return libusb_handle_events_timeout(ctx, &tv);
	}
	else
	{
		return 0;
	}
#endif
}

int usbhidasync_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {
	int transferred;

	if (count >= usbdevices[device].config.endpoints.out.size) {

		PRINT_ERROR_OTHER("incorrect write size")
		return -1;
	}

	unsigned char * buffer = calloc(usbdevices[device].config.endpoints.out.size, sizeof(unsigned char));
	if (buffer == NULL) {

		PRINT_ERROR_ALLOC_FAILED("calloc")
		return -1;
	}

	memcpy(buffer, buf + 1, count - 1);

	int ret = libusb_interrupt_transfer(usbdevices[device].devh, usbdevices[device].config.endpoints.out.address,
			(void *) buffer, usbdevices[device].config.endpoints.out.size - 1, &transferred, timeout * 1000);
	if (ret != LIBUSB_SUCCESS) {

		PRINT_ERROR_LIBUSB("libusb_interrupt_transfer", ret)
		free(buffer);
		return -1;
	}

	free(buffer);
	return 0;
}

int usbhidasync_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {
	int transferred;

	int ret = libusb_interrupt_transfer(usbdevices[device].devh, usbdevices[device].config.endpoints.in.address,
			(void *) buf, count, &transferred, timeout * 1000);
	if (ret != LIBUSB_SUCCESS) {
		PRINT_ERROR_LIBUSB("libusb_interrupt_transfer", ret)
		return -1;
	}

	return 0;
}

#ifdef WIN32
#define PACKED __attribute__((gcc_struct, packed))
#else
#define PACKED __attribute__((packed))
#endif

typedef struct PACKED
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short bcdHID;
	unsigned char bCountryCode;
	unsigned char bNumDescriptors;
	unsigned char bReportDescriptorType;
	unsigned short wReportDescriptorLength;
} s_hid_descriptor;

static s_hid_info probe_hid(const unsigned char * extra, int extra_length) {

	s_hid_info hidInfo = { };	
	
	unsigned int pos;
	for (pos = 0; pos < extra_length && extra[pos]; pos += extra[pos]) {

		if(extra[pos] < sizeof(s_hid_descriptor)) {
			continue;
		}

		s_hid_descriptor * hidDescriptor = (s_hid_descriptor *) (extra + pos);

		if (hidDescriptor->bDescriptorType == LIBUSB_DT_HID && hidDescriptor->bReportDescriptorType == LIBUSB_DT_REPORT) {
			hidInfo.version = hidDescriptor->bcdHID;
			hidInfo.countryCode = hidDescriptor->bCountryCode;
			hidInfo.reportDescriptorLength = hidDescriptor->wReportDescriptorLength;
			return hidInfo;
		}
	}

	return hidInfo;
}

/*
 * Look for an interface that is from the HID class and that has at least one interrupt endpoint.
 * Return the following properties:
 * - bConfigurationValue
 * - bInterfaceNumber
 * - bAlternateSetting
 * - interrupt in endpoint: bEndpointAddress, wMaxPacketSize
 * - interrupt out endpoint: bEndpointAddress, wMaxPacketSize
 * - hid info (version, country, report descriptor)
 */
static s_config probe_device(libusb_device * dev, struct libusb_device_descriptor * desc) {

	s_config config = { .configuration = -1 };

	int cfg;
	for (cfg = 0; cfg < desc->bNumConfigurations && config.configuration == -1; ++cfg) {
		struct libusb_config_descriptor * configuration;
		int ret = libusb_get_config_descriptor(dev, cfg, &configuration);
		if (ret != LIBUSB_SUCCESS) {
			PRINT_ERROR_LIBUSB("libusb_get_config_descriptor", ret)
			return config;
		}
		int itf;
		for (itf = 0; itf < configuration->bNumInterfaces && config.configuration == -1; ++itf) {
			int alt;
			const struct libusb_interface * interface = configuration->interface + itf;
			for (alt = 0; alt < interface->num_altsetting && config.configuration == -1; ++alt) {
				const struct libusb_interface_descriptor * interfaceDesc = interface->altsetting + alt;
				if (interfaceDesc->bInterfaceClass == LIBUSB_CLASS_HID) {
					config.configuration = configuration->bConfigurationValue;
					config.interface.number = itf;
					config.interface.alternateSetting = alt;
                    config.interface.hidInfo = probe_hid(interfaceDesc->extra, interfaceDesc->extra_length);
					int ep;
					for (ep = 0; ep < interfaceDesc->bNumEndpoints; ++ep) {
						const struct libusb_endpoint_descriptor * endpoint = interfaceDesc->endpoint + ep;
						if ((endpoint->bmAttributes & 0b00000011) == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
							if ((endpoint->bEndpointAddress & 0b10000000) == LIBUSB_ENDPOINT_IN) {
								if (config.endpoints.in.address == 0) {
									config.endpoints.in.address = endpoint->bEndpointAddress;
									config.endpoints.in.size = endpoint->wMaxPacketSize;
								}
							} else {
								if (config.endpoints.out.address == 0) {
									config.endpoints.out.address = endpoint->bEndpointAddress;
									config.endpoints.out.size = endpoint->wMaxPacketSize;
								}
							}
						}
					}
				}
			}
		}
		libusb_free_config_descriptor(configuration);
	}
	return config;
}

int claim_device(int device, libusb_device * dev) {

	int ret = libusb_open(dev, &usbdevices[device].devh);
	if (ret != LIBUSB_SUCCESS) {
		PRINT_ERROR_LIBUSB("libusb_open", ret)
		usbhidasync_close(device);
		return -1;
	}

#if defined(LIBUSB_API_VERSION) || defined(LIBUSBX_API_VERSION)
	libusb_set_auto_detach_kernel_driver(usbdevices[device].devh, 1);
#else
#ifndef WIN32
	ret = libusb_kernel_driver_active(usbdevices[device].devh, 0);
	if(ret == 1)
	{
		ret = libusb_detach_kernel_driver(usbdevices[device].devh, 0);
		if(ret != LIBUSB_SUCCESS)
		{
			PRINT_ERROR_LIBUSB("libusb_detach_kernel_driver", ret)
			usbhidasync_close(device);
			return -1;
		}
	}
	else if(ret != LIBUSB_SUCCESS)
	{
		PRINT_ERROR_LIBUSB("libusb_kernel_driver_active", ret)
		usbhidasync_close(device);
		return -1;
	}
#endif
#endif
	int configuration;

	ret = libusb_get_configuration(usbdevices[device].devh, &configuration);
	if (ret != LIBUSB_SUCCESS) {
		PRINT_ERROR_LIBUSB("libusb_get_configuration", ret)
		usbhidasync_close(device);
		return -1;
	}

	if (configuration != usbdevices[device].config.configuration) {
		ret = libusb_set_configuration(usbdevices[device].devh, usbdevices[device].config.configuration);
		if (ret != LIBUSB_SUCCESS) {
			PRINT_ERROR_LIBUSB("libusb_set_configuration", ret)
			usbhidasync_close(device);
			return -1;
		}
	}

	ret = libusb_claim_interface(usbdevices[device].devh, usbdevices[device].config.interface.number);
	if (ret != LIBUSB_SUCCESS) {
		PRINT_ERROR_LIBUSB("libusb_claim_interface", ret)
		usbhidasync_close(device);
		return -1;
	}

	ret = libusb_set_interface_alt_setting(usbdevices[device].devh, usbdevices[device].config.interface.number,
			usbdevices[device].config.interface.alternateSetting);
	if (ret != LIBUSB_SUCCESS) {
		PRINT_ERROR_LIBUSB("libusb_set_interface_alt_setting", ret)
		usbhidasync_close(device);
		return -1;
	}

	s_hid_info * hidInfo = &usbdevices[device].config.interface.hidInfo;
	if(hidInfo->reportDescriptorLength > 0) {
		hidInfo->reportDescriptor = calloc(hidInfo->reportDescriptorLength, sizeof(unsigned char));
		if (hidInfo->reportDescriptor == NULL) {
			PRINT_ERROR_ALLOC_FAILED("calloc");
			usbhidasync_close(device);
			return -1;
		}
		int ret = libusb_control_transfer( usbdevices[device].devh, LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE, LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_REPORT << 8) | 0, 0, hidInfo->reportDescriptor, hidInfo->reportDescriptorLength, 1000); 
		if (ret < 0) {
			PRINT_ERROR_LIBUSB("libusb_get_descriptor", ret)
			usbhidasync_close(device);
			return -1;
		}
                else {
			hidInfo->reportDescriptorLength = ret;
		}
	}

	return 0;
}

int usbhidasync_open_ids(unsigned short vendor, unsigned short product) {
	int ret = -1;
	int dev_i;

	if (!ctx) {
		ret = libusb_init(&ctx);
		if (ret != LIBUSB_SUCCESS) {
			PRINT_ERROR_LIBUSB("libusb_init", ret)
			return -1;
		}
	}

	if (!devs) {
		cnt = libusb_get_device_list(ctx, &devs);
		if (cnt < 0) {
			PRINT_ERROR_LIBUSB("libusb_get_device_list", cnt)
			return -1;
		}
	}

	for (dev_i = 0; dev_i < cnt; ++dev_i) {
		struct libusb_device_descriptor desc;
		ret = libusb_get_device_descriptor(devs[dev_i], &desc);
		if (!ret) {
			if (desc.idVendor == vendor && desc.idProduct == product) {
				s_config config = probe_device(devs[dev_i], &desc);
				if (config.configuration == 0) {
					continue;
				}

				int device = add_device(make_path(devs[dev_i]), &config, 0);
				if (device < 0) {
					continue;
				}

				if (claim_device(device, devs[dev_i]) != -1) {
					usbdevices[device].vendor = desc.idVendor;
					usbdevices[device].product = desc.idProduct;
					return device;
				}
			}
		}
	}

	return -1;
}

int usbhidasync_open_path(const char * path) {
	int ret = -1;
	int dev_i;

	unsigned char bus, address;

	if (sscanf(path, "%hhx:%hhx", &bus, &address) != 2) {
		PRINT_ERROR_OTHER("invalid path");
		return -1;
	}

	if (!ctx) {
		ret = libusb_init(&ctx);
		if (ret != LIBUSB_SUCCESS) {
			PRINT_ERROR_LIBUSB("libusb_init", ret)
			return -1;
		}
	}

	if (!devs) {
		cnt = libusb_get_device_list(ctx, &devs);
		if (cnt < 0) {
			PRINT_ERROR_LIBUSB("libusb_get_device_list", cnt)
			return -1;
		}
	}

	for (dev_i = 0; dev_i < cnt; ++dev_i) {
		if (bus != libusb_get_bus_number(devs[dev_i]) || address != libusb_get_device_address(devs[dev_i])) {
			continue;
		}
		struct libusb_device_descriptor desc;
		ret = libusb_get_device_descriptor(devs[dev_i], &desc);
		if (!ret) {
			s_config config = probe_device(devs[dev_i], &desc);
			if (config.configuration == 0) {
				continue;
			}

			int device = add_device(make_path(devs[dev_i]), &config, 0);
			if (device < 0) {
				continue;
			}

			if (claim_device(device, devs[dev_i]) != -1) {
				usbdevices[device].vendor = desc.idVendor;
				usbdevices[device].product = desc.idProduct;
				return device;
			}
		}
	}

	return -1;
}

int usbhidasync_get_ids(int device, unsigned short * vendor, unsigned short * product) {

	USBHIDASYNC_CHECK_DEVICE(device, -1)

	*vendor = usbdevices[device].vendor;
	*product = usbdevices[device].product;

	return 0;
}

const s_hid_info * usbhidasync_get_hid_info(int device) {

	USBHIDASYNC_CHECK_DEVICE(device, NULL)
	
	return &usbdevices[device].config.interface.hidInfo;
}

static int close_callback(int device) {

	USBHIDASYNC_CHECK_DEVICE(device, -1)

	return usbdevices[device].callback.fp_close(usbdevices[device].callback.user);
}

int usbhidasync_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write,
		ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {

	USBHIDASYNC_CHECK_DEVICE(device, -1)

	if (fp_read != NULL && usbdevices[device].config.endpoints.in.address == 0x00) {

		PRINT_ERROR_OTHER("device has no interrupt in endpoint")
		return -1;
	}

	if (fp_write != NULL && usbdevices[device].config.endpoints.out.address == 0x00) {

		PRINT_ERROR_OTHER("device has no interrupt out endpoint")
		return -1;
	}

	const struct libusb_pollfd** pfd_usb = libusb_get_pollfds(ctx);
	int poll_i;
	for (poll_i = 0; pfd_usb[poll_i] != NULL; ++poll_i) {

		fp_register(pfd_usb[poll_i]->fd, device, handle_events, handle_events, close_callback);
	}
	free(pfd_usb);

	usbdevices[device].callback.user = user;
	usbdevices[device].callback.fp_read = fp_read;
	usbdevices[device].callback.fp_write = fp_write;
	usbdevices[device].callback.fp_close = fp_close;

	if (usbdevices[device].callback.fp_read != NULL) {

		poll_interrupt(device);
	}

	return 0;
}

/*
 * Cancel all pending tranfers for a given device.
 */
static void cancel_transfers(int device) {
	unsigned int i;
	for (i = 0; i < transfers_nb; ++i) {

		if ((unsigned long) (transfers[i]->user_data) == device) {

			libusb_cancel_transfer(transfers[i]);
		}
	}

	while (usbdevices[device].pending_transfers) {

		if (libusb_handle_events(ctx) != LIBUSB_SUCCESS) {

			break;
		}
	}
}

int usbhidasync_close(int device) {

	USBHIDASYNC_CHECK_DEVICE(device, -1)

	cancel_transfers(device);

	libusb_release_interface(usbdevices[device].devh, usbdevices[device].config.interface.number);
#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
#ifndef WIN32
	libusb_attach_kernel_driver(usbdevices[device].devh, 0);
#endif
#endif
	libusb_close(usbdevices[device].devh);
	usbdevices[device].devh = NULL;
	free(usbdevices[device].path);
	usbdevices[device].path = NULL;
	free(usbdevices[device].config.interface.hidInfo.reportDescriptor);
	usbdevices[device].config.interface.hidInfo.reportDescriptor = NULL;
	--nb_opened;
	if (!nb_opened) {
		while (transfers_nb) {

			if (libusb_handle_events(ctx) != LIBUSB_SUCCESS) {

				break;
			}
		}
		free(transfers);
		transfers = NULL;
		transfers_nb = 0;
		libusb_free_device_list(devs, 1);
		devs = NULL;
		libusb_exit(ctx);
		ctx = NULL;
	}

	return 1;
}

int usbhidasync_write(int device, const void * buf, unsigned int count) {

	USBHIDASYNC_CHECK_DEVICE(device, -1)

	if (usbdevices[device].callback.fp_write == NULL) {

		PRINT_ERROR_OTHER("missing write callback")
		return -1;
	}

	if (count >= usbdevices[device].config.endpoints.out.size) {

		PRINT_ERROR_OTHER("incorrect write size")
		return -1;
	}

	unsigned char * buffer = calloc(usbdevices[device].config.endpoints.out.size, sizeof(unsigned char));
	if (buffer == NULL) {
		PRINT_ERROR_ALLOC_FAILED("calloc")
		return -1;
	}

	memcpy(buffer, buf + 1, count - 1);

	struct libusb_transfer * transfer = libusb_alloc_transfer(0);
	if (transfer == NULL) {

		PRINT_ERROR_ALLOC_FAILED("libusb_alloc_transfer")
		free(buffer);
		return -1;
	}

	libusb_fill_interrupt_transfer(transfer, usbdevices[device].devh, usbdevices[device].config.endpoints.out.address,
			buffer, usbdevices[device].config.endpoints.out.size - 1, (libusb_transfer_cb_fn) usb_callback,
			(void *) (unsigned long) device, 1000);

	return submit_transfer(transfer);
}

