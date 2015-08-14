/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <uhidasync.h>

#include <linux/uhid.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define UHID_MAX_DEVICES 256

#define UHID_PATH "/dev/uhid"

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);

static struct {
	int fd;

} uhidasync_devices[UHID_MAX_DEVICES] = { };

void uhidasync_init(void) __attribute__((constructor (101)));
void uhidasync_init(void) {
	int i;
	for (i = 0; i < UHID_MAX_DEVICES; ++i) {
		uhidasync_devices[i].fd = -1;
	}
}

void uhidasync_clean(void) __attribute__((destructor (101)));
void uhidasync_clean(void) {
	int i;
	for (i = 0; i < UHID_MAX_DEVICES; ++i) {
		if (uhidasync_devices[i].fd >= 0) {
			uhidasync_close(i);
		}
	}
}

inline int uhidasync_check_device(int device, const char * file, unsigned int line, const char * func) {
	if (device < 0 || device >= UHID_MAX_DEVICES) {
		PRINT_ERROR_OTHER("invalid device")
		return -1;
	}
	if (uhidasync_devices[device].fd == -1) {
		PRINT_ERROR_OTHER("no such device")
		return -1;
	}
	return 0;
}
#define uhidasync_CHECK_DEVICE(device) \
  if(uhidasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return -1; \
  }

static int add_device(int fd) {
	int i;
	for (i = 0; i < UHID_MAX_DEVICES; ++i) {
		if (uhidasync_devices[i].fd == -1) {
			uhidasync_devices[i].fd = fd;
			return i;
		}
	}
	return -1;
}

static int uhid_write(int fd, const struct uhid_event *ev) {

	ssize_t ret = write(fd, ev, sizeof(*ev));
	if (ret < 0) {
		PRINT_ERROR_ERRNO("write")
		return -1;
	} else if (ret != sizeof(*ev)) {
		fprintf(stderr, "Wrong size written to uhid: %ld != %lu\n", ret, sizeof(ev));
		return -1;
	} else {
		return 0;
	}
}

static __u8 momo2_rdesc_fixed[] = {
	0x05, 0x01,         /*  Usage Page (Desktop),               */
	0x09, 0x04,         /*  Usage (Joystik),                    */
	0xA1, 0x01,         /*  Collection (Application),           */
	0xA1, 0x02,         /*      Collection (Logical),           */
	0x95, 0x01,         /*          Report Count (1),           */
	0x75, 0x0A,         /*          Report Size (10),           */
	0x15, 0x00,         /*          Logical Minimum (0),        */
	0x26, 0xFF, 0x03,   /*          Logical Maximum (1023),     */
	0x35, 0x00,         /*          Physical Minimum (0),       */
	0x46, 0xFF, 0x03,   /*          Physical Maximum (1023),    */
	0x09, 0x30,         /*          Usage (X),                  */
	0x81, 0x02,         /*          Input (Variable),           */
	0x95, 0x0A,         /*          Report Count (10),          */
	0x75, 0x01,         /*          Report Size (1),            */
	0x25, 0x01,         /*          Logical Maximum (1),        */
	0x45, 0x01,         /*          Physical Maximum (1),       */
	0x05, 0x09,         /*          Usage Page (Button),        */
	0x19, 0x01,         /*          Usage Minimum (01h),        */
	0x29, 0x0A,         /*          Usage Maximum (0Ah),        */
	0x81, 0x02,         /*          Input (Variable),           */
	0x06, 0x00, 0xFF,   /*          Usage Page (FF00h),         */
	0x09, 0x00,         /*          Usage (00h),                */
	0x95, 0x04,         /*          Report Count (4),           */
	0x81, 0x02,         /*          Input (Variable),           */
	0x95, 0x01,         /*          Report Count (1),           */
	0x75, 0x08,         /*          Report Size (8),            */
	0x26, 0xFF, 0x00,   /*          Logical Maximum (255),      */
	0x46, 0xFF, 0x00,   /*          Physical Maximum (255),     */
	0x09, 0x01,         /*          Usage (01h),                */
	0x81, 0x02,         /*          Input (Variable),           */
	0x05, 0x01,         /*          Usage Page (Desktop),       */
	0x09, 0x31,         /*          Usage (Y),                  */
	0x81, 0x02,         /*          Input (Variable),           */
	0x09, 0x32,         /*          Usage (Z),                  */
	0x81, 0x02,         /*          Input (Variable),           */
	0x06, 0x00, 0xFF,   /*          Usage Page (FF00h),         */
	0x09, 0x00,         /*          Usage (00h),                */
	0x81, 0x02,         /*          Input (Variable),           */
	0xC0,               /*      End Collection,                 */
	0xA1, 0x02,         /*      Collection (Logical),           */
	0x09, 0x02,         /*          Usage (02h),                */
	0x95, 0x07,         /*          Report Count (7),           */
	0x91, 0x02,         /*          Output (Variable),          */
	0xC0,               /*      End Collection,                 */
	0xC0                /*  End Collection                      */
};

int uhidasync_create(const char * name, unsigned short vendor, unsigned short product) {

	int fd = open(UHID_PATH, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		PRINT_ERROR_ERRNO("open")
		return -1;
	}

	struct uhid_event ev = {
		.type = UHID_CREATE,
		.u.create.rd_data = momo2_rdesc_fixed,
		.u.create.rd_size = sizeof(momo2_rdesc_fixed),
		.u.create.bus = BUS_VIRTUAL,
		.u.create.vendor = vendor,
		.u.create.product = product,
		.u.create.version = 0,
		.u.create.country = 0,
	};

	if(strlen(name) >= sizeof(ev.u.create.name)) {

		PRINT_ERROR_OTHER("name is too long")
		close(fd);
		return -1;
	}

	strcpy((char *)ev.u.create.name, name);

	if(uhid_write(fd, &ev) < 0) {
		close(fd);
		return -1;
	}

	int device = add_device(fd);

	if(device < 0) {
		close(fd);
	}

	return device;
}

int uhidasync_close(int device) {

	struct uhid_event ev = { .type = UHID_DESTROY };
	uhid_write(uhidasync_devices[device].fd, &ev);
	close(uhidasync_devices[device].fd);
	uhidasync_devices[device].fd = -1;

	return 0;
}

int uhidasync_write(int device, const void * buf, unsigned int count) {

	if(count > UHID_DATA_MAX) {

		PRINT_ERROR_OTHER("count is higher than UHID_DATA_MAX")
		return -1;
	}

	struct uhid_event ev = { .type = UHID_INPUT, .u.input.size = count };
	memcpy(ev.u.input.data, buf, count);

	return uhid_write(uhidasync_devices[device].fd, &ev);
}
