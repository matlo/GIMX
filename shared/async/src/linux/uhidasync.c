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
#include <errno.h>

#define UHID_MAX_DEVICES 256

#define UHID_PATH "/dev/uhid"

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);

static struct {
    int fd;
    int opened;
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
        fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
        return -1;
    }
    if (uhidasync_devices[device].fd < 0) {
        fprintf(stderr, "%s:%d %s: no such device\n", file, line, func);
        return -1;
    }
    return 0;
}
#define UHIDASYNC_CHECK_DEVICE(device,retValue) \
  if(uhidasync_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
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

/* Fixed report descriptors for Logitech Driving Force (and Pro)
 * wheel controllers
 *
 * The original descriptors hide the separate throttle and brake axes in
 * a custom vendor usage page, providing only a combined value as
 * GenericDesktop.Y.
 * These descriptors remove the combined Y axis and instead report
 * separate throttle (Y) and brake (RZ).
 */
static __u8 df_rdesc_fixed[] = { 0x05, 0x01, /*  Usage Page (Desktop),                   */
        0x09, 0x04, /*  Usage (Joystik),                        */
        0xA1, 0x01, /*  Collection (Application),               */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x0A, /*          Report Size (10),               */
        0x14, /*          Logical Minimum (0),            */
        0x26, 0xFF, 0x03, /*          Logical Maximum (1023),         */
        0x34, /*          Physical Minimum (0),           */
        0x46, 0xFF, 0x03, /*          Physical Maximum (1023),        */
        0x09, 0x30, /*          Usage (X),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x95, 0x0C, /*          Report Count (12),              */
        0x75, 0x01, /*          Report Size (1),                */
        0x25, 0x01, /*          Logical Maximum (1),            */
        0x45, 0x01, /*          Physical Maximum (1),           */
        0x05, 0x09, /*          Usage (Buttons),                */
        0x19, 0x01, /*          Usage Minimum (1),              */
        0x29, 0x0c, /*          Usage Maximum (12),             */
        0x81, 0x02, /*          Input (Variable),               */
        0x95, 0x02, /*          Report Count (2),               */
        0x06, 0x00, 0xFF, /*          Usage Page (Vendor: 65280),     */
        0x09, 0x01, /*          Usage (?: 1),                   */
        0x81, 0x02, /*          Input (Variable),               */
        0x05, 0x01, /*          Usage Page (Desktop),           */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x81, 0x02, /*          Input (Variable),               */
        0x25, 0x07, /*          Logical Maximum (7),            */
        0x46, 0x3B, 0x01, /*          Physical Maximum (315),         */
        0x75, 0x04, /*          Report Size (4),                */
        0x65, 0x14, /*          Unit (Degrees),                 */
        0x09, 0x39, /*          Usage (Hat Switch),             */
        0x81, 0x42, /*          Input (Variable, Null State),   */
        0x75, 0x01, /*          Report Size (1),                */
        0x95, 0x04, /*          Report Count (4),               */
        0x65, 0x00, /*          Unit (none),                    */
        0x06, 0x00, 0xFF, /*          Usage Page (Vendor: 65280),     */
        0x09, 0x01, /*          Usage (?: 1),                   */
        0x25, 0x01, /*          Logical Maximum (1),            */
        0x45, 0x01, /*          Physical Maximum (1),           */
        0x81, 0x02, /*          Input (Variable),               */
        0x05, 0x01, /*          Usage Page (Desktop),           */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x09, 0x31, /*          Usage (Y),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x09, 0x35, /*          Usage (Rz),                     */
        0x81, 0x02, /*          Input (Variable),               */
        0xC0, /*      End Collection,                     */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x95, 0x07, /*          Report Count (7),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x09, 0x03, /*          Usage (?: 3),                   */
        0x91, 0x02, /*          Output (Variable),              */
        0xC0, /*      End Collection,                     */
        0xC0 /*  End Collection                          */
};

static __u8 dfp_rdesc_fixed[] = { 0x05, 0x01, /*  Usage Page (Desktop),                   */
        0x09, 0x04, /*  Usage (Joystik),                        */
        0xA1, 0x01, /*  Collection (Application),               */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x0E, /*          Report Size (14),               */
        0x14, /*          Logical Minimum (0),            */
        0x26, 0xFF, 0x3F, /*          Logical Maximum (16383),        */
        0x34, /*          Physical Minimum (0),           */
        0x46, 0xFF, 0x3F, /*          Physical Maximum (16383),       */
        0x09, 0x30, /*          Usage (X),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x95, 0x0E, /*          Report Count (14),              */
        0x75, 0x01, /*          Report Size (1),                */
        0x25, 0x01, /*          Logical Maximum (1),            */
        0x45, 0x01, /*          Physical Maximum (1),           */
        0x05, 0x09, /*          Usage Page (Button),            */
        0x19, 0x01, /*          Usage Minimum (01h),            */
        0x29, 0x0E, /*          Usage Maximum (0Eh),            */
        0x81, 0x02, /*          Input (Variable),               */
        0x05, 0x01, /*          Usage Page (Desktop),           */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x04, /*          Report Size (4),                */
        0x25, 0x07, /*          Logical Maximum (7),            */
        0x46, 0x3B, 0x01, /*          Physical Maximum (315),         */
        0x65, 0x14, /*          Unit (Degrees),                 */
        0x09, 0x39, /*          Usage (Hat Switch),             */
        0x81, 0x42, /*          Input (Variable, Nullstate),    */
        0x65, 0x00, /*          Unit,                           */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x75, 0x08, /*          Report Size (8),                */
        0x81, 0x01, /*          Input (Constant),               */
        0x09, 0x31, /*          Usage (Y),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x09, 0x35, /*          Usage (Rz),                     */
        0x81, 0x02, /*          Input (Variable),               */
        0x81, 0x01, /*          Input (Constant),               */
        0xC0, /*      End Collection,                     */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x09, 0x02, /*          Usage (02h),                    */
        0x95, 0x07, /*          Report Count (7),               */
        0x91, 0x02, /*          Output (Variable),              */
        0xC0, /*      End Collection,                     */
        0xC0 /*  End Collection                          */
};

static __u8 fv_rdesc_fixed[] = { 0x05, 0x01, /*  Usage Page (Desktop),                   */
        0x09, 0x04, /*  Usage (Joystik),                        */
        0xA1, 0x01, /*  Collection (Application),               */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x0A, /*          Report Size (10),               */
        0x15, 0x00, /*          Logical Minimum (0),            */
        0x26, 0xFF, 0x03, /*          Logical Maximum (1023),         */
        0x35, 0x00, /*          Physical Minimum (0),           */
        0x46, 0xFF, 0x03, /*          Physical Maximum (1023),        */
        0x09, 0x30, /*          Usage (X),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x95, 0x0C, /*          Report Count (12),              */
        0x75, 0x01, /*          Report Size (1),                */
        0x25, 0x01, /*          Logical Maximum (1),            */
        0x45, 0x01, /*          Physical Maximum (1),           */
        0x05, 0x09, /*          Usage Page (Button),            */
        0x19, 0x01, /*          Usage Minimum (01h),            */
        0x29, 0x0C, /*          Usage Maximum (0Ch),            */
        0x81, 0x02, /*          Input (Variable),               */
        0x95, 0x02, /*          Report Count (2),               */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),             */
        0x09, 0x01, /*          Usage (01h),                    */
        0x81, 0x02, /*          Input (Variable),               */
        0x09, 0x02, /*          Usage (02h),                    */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x81, 0x02, /*          Input (Variable),               */
        0x05, 0x01, /*          Usage Page (Desktop),           */
        0x25, 0x07, /*          Logical Maximum (7),            */
        0x46, 0x3B, 0x01, /*          Physical Maximum (315),         */
        0x75, 0x04, /*          Report Size (4),                */
        0x65, 0x14, /*          Unit (Degrees),                 */
        0x09, 0x39, /*          Usage (Hat Switch),             */
        0x81, 0x42, /*          Input (Variable, Null State),   */
        0x75, 0x01, /*          Report Size (1),                */
        0x95, 0x04, /*          Report Count (4),               */
        0x65, 0x00, /*          Unit,                           */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),             */
        0x09, 0x01, /*          Usage (01h),                    */
        0x25, 0x01, /*          Logical Maximum (1),            */
        0x45, 0x01, /*          Physical Maximum (1),           */
        0x81, 0x02, /*          Input (Variable),               */
        0x05, 0x01, /*          Usage Page (Desktop),           */
        0x95, 0x01, /*          Report Count (1),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x09, 0x31, /*          Usage (Y),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0x09, 0x32, /*          Usage (Z),                      */
        0x81, 0x02, /*          Input (Variable),               */
        0xC0, /*      End Collection,                     */
        0xA1, 0x02, /*      Collection (Logical),               */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),          */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),         */
        0x95, 0x07, /*          Report Count (7),               */
        0x75, 0x08, /*          Report Size (8),                */
        0x09, 0x03, /*          Usage (03h),                    */
        0x91, 0x02, /*          Output (Variable),              */
        0xC0, /*      End Collection,                     */
        0xC0 /*  End Collection                          */
};

static __u8 momo_rdesc_fixed[] = { 0x05, 0x01, /*  Usage Page (Desktop),               */
        0x09, 0x04, /*  Usage (Joystik),                    */
        0xA1, 0x01, /*  Collection (Application),           */
        0xA1, 0x02, /*      Collection (Logical),           */
        0x95, 0x01, /*          Report Count (1),           */
        0x75, 0x0A, /*          Report Size (10),           */
        0x15, 0x00, /*          Logical Minimum (0),        */
        0x26, 0xFF, 0x03, /*          Logical Maximum (1023),     */
        0x35, 0x00, /*          Physical Minimum (0),       */
        0x46, 0xFF, 0x03, /*          Physical Maximum (1023),    */
        0x09, 0x30, /*          Usage (X),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x95, 0x08, /*          Report Count (8),           */
        0x75, 0x01, /*          Report Size (1),            */
        0x25, 0x01, /*          Logical Maximum (1),        */
        0x45, 0x01, /*          Physical Maximum (1),       */
        0x05, 0x09, /*          Usage Page (Button),        */
        0x19, 0x01, /*          Usage Minimum (01h),        */
        0x29, 0x08, /*          Usage Maximum (08h),        */
        0x81, 0x02, /*          Input (Variable),           */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),         */
        0x75, 0x0E, /*          Report Size (14),           */
        0x95, 0x01, /*          Report Count (1),           */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),      */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),     */
        0x09, 0x00, /*          Usage (00h),                */
        0x81, 0x02, /*          Input (Variable),           */
        0x05, 0x01, /*          Usage Page (Desktop),       */
        0x75, 0x08, /*          Report Size (8),            */
        0x09, 0x31, /*          Usage (Y),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x09, 0x32, /*          Usage (Z),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),         */
        0x09, 0x01, /*          Usage (01h),                */
        0x81, 0x02, /*          Input (Variable),           */
        0xC0, /*      End Collection,                 */
        0xA1, 0x02, /*      Collection (Logical),           */
        0x09, 0x02, /*          Usage (02h),                */
        0x95, 0x07, /*          Report Count (7),           */
        0x91, 0x02, /*          Output (Variable),          */
        0xC0, /*      End Collection,                 */
        0xC0 /*  End Collection                      */
};

//TODO MLA: try to separate the pedal axes
static __u8 ffgp_rdesc_fixed[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x04,        // Usage (Joystick)
        0xA1, 0x01,        // Collection (Application)
        0xA1, 0x02,        //   Collection (Logical)
        0x95, 0x01,        //     Report Count (1)
        0x75, 0x0A,        //     Report Size (10)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x03,  //     Logical Maximum (1023)
        0x35, 0x00,        //     Physical Minimum (0)
        0x46, 0xFF, 0x03,  //     Physical Maximum (1023)
        0x09, 0x30,        //     Usage (X)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x06,        //     Report Count (6)
        0x75, 0x01,        //     Report Size (1)
        0x25, 0x01,        //     Logical Maximum (1)
        0x45, 0x01,        //     Physical Maximum (1)
        0x05, 0x09,        //     Usage Page (Button)
        0x19, 0x01,        //     Usage Minimum (0x01)
        0x29, 0x06,        //     Usage Maximum (0x06)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //     Report Count (1)
        0x75, 0x08,        //     Report Size (8)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x46, 0xFF, 0x00,  //     Physical Maximum (255)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,        //     Usage (0x01)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x09, 0x31,        //     Usage (Y)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,        //     Usage (0x01)
        0x95, 0x03,        //     Report Count (3)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0xA1, 0x02,        //   Collection (Logical)
        0x09, 0x02,        //     Usage (0x02)
        0x95, 0x07,        //     Report Count (7)
        0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              //   End Collection
        0xC0,              // End Collection
};

static __u8 momo2_rdesc_fixed[] = { 0x05, 0x01, /*  Usage Page (Desktop),               */
        0x09, 0x04, /*  Usage (Joystik),                    */
        0xA1, 0x01, /*  Collection (Application),           */
        0xA1, 0x02, /*      Collection (Logical),           */
        0x95, 0x01, /*          Report Count (1),           */
        0x75, 0x0A, /*          Report Size (10),           */
        0x15, 0x00, /*          Logical Minimum (0),        */
        0x26, 0xFF, 0x03, /*          Logical Maximum (1023),     */
        0x35, 0x00, /*          Physical Minimum (0),       */
        0x46, 0xFF, 0x03, /*          Physical Maximum (1023),    */
        0x09, 0x30, /*          Usage (X),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x95, 0x0A, /*          Report Count (10),          */
        0x75, 0x01, /*          Report Size (1),            */
        0x25, 0x01, /*          Logical Maximum (1),        */
        0x45, 0x01, /*          Physical Maximum (1),       */
        0x05, 0x09, /*          Usage Page (Button),        */
        0x19, 0x01, /*          Usage Minimum (01h),        */
        0x29, 0x0A, /*          Usage Maximum (0Ah),        */
        0x81, 0x02, /*          Input (Variable),           */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),         */
        0x09, 0x00, /*          Usage (00h),                */
        0x95, 0x04, /*          Report Count (4),           */
        0x81, 0x02, /*          Input (Variable),           */
        0x95, 0x01, /*          Report Count (1),           */
        0x75, 0x08, /*          Report Size (8),            */
        0x26, 0xFF, 0x00, /*          Logical Maximum (255),      */
        0x46, 0xFF, 0x00, /*          Physical Maximum (255),     */
        0x09, 0x01, /*          Usage (01h),                */
        0x81, 0x02, /*          Input (Variable),           */
        0x05, 0x01, /*          Usage Page (Desktop),       */
        0x09, 0x31, /*          Usage (Y),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x09, 0x32, /*          Usage (Z),                  */
        0x81, 0x02, /*          Input (Variable),           */
        0x06, 0x00, 0xFF, /*          Usage Page (FF00h),         */
        0x09, 0x00, /*          Usage (00h),                */
        0x81, 0x02, /*          Input (Variable),           */
        0xC0, /*      End Collection,                 */
        0xA1, 0x02, /*      Collection (Logical),           */
        0x09, 0x02, /*          Usage (02h),                */
        0x95, 0x07, /*          Report Count (7),           */
        0x91, 0x02, /*          Output (Variable),          */
        0xC0, /*      End Collection,                 */
        0xC0 /*  End Collection                      */
};

/*
 * See http://wiibrew.org/wiki/Logitech_USB_steering_wheel
 */
static __u8 wii_rdesc_fixed[] = { 0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x04,        // Usage (Joystick)
        0xA1, 0x01,        // Collection (Application)
        0xA1, 0x02,        //   Collection (Logical)
        0x95, 0x01,        //     Report Count (1)
        0x75, 0x0A,        //     Report Size (10)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x03,  //     Logical Maximum (1023)
        0x35, 0x00,        //     Physical Minimum (0)
        0x46, 0xFF, 0x03,  //     Physical Maximum (1023)
        0x09, 0x30,        //     Usage (X)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x95, 0x02,        //     Report Count (2)
        0x75, 0x01,        //     Report Size (1)
        0x25, 0x01,        //     Logical Maximum (1)
        0x45, 0x01,        //     Physical Maximum (1)
        0x09, 0x01,        //     Usage (0x01)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x09,        //     Usage Page (Button)
        0x95, 0x0B,        //     Report Count (11)
        0x29, 0x0B,        //     Usage Maximum (0x0B)
        0x05, 0x09,        //     Usage Page (Button)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x95, 0x01,        //     Report Count (1)
        0x75, 0x01,        //     Report Size (1)
        0x09, 0x02,        //     Usage (0x02)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x75, 0x08,        //     Report Size (8)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x46, 0xFF, 0x00,  //     Physical Maximum (255)
        0x09, 0x31,        //     Usage (Y)
        0x09, 0x32,        //     Usage (Z)
        0x95, 0x02,        //     Report Count (2)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0xA1, 0x02,        //   Collection (Logical)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x95, 0x07,        //     Report Count (7)
        0x09, 0x03,        //     Usage (0x03)
        0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              //   End Collection
        0x0A, 0xFF, 0xFF,  //   Usage (0xFFFF)
        0x95, 0x08,        //   Report Count (8)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              // End Collection
        };

static struct {
    unsigned short vendor;
    unsigned short product;
    unsigned char * rdesc;
    unsigned short length;
} rdesc_fixed[] = {
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_WINGMAN_FFG, ffgp_rdesc_fixed, sizeof(ffgp_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_WHEEL, df_rdesc_fixed, sizeof(df_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_MOMO_WHEEL, momo_rdesc_fixed, sizeof(momo_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_MOMO_WHEEL2, momo2_rdesc_fixed, sizeof(momo2_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_VIBRATION_WHEEL, fv_rdesc_fixed, sizeof(fv_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_DFP_WHEEL, dfp_rdesc_fixed, sizeof(dfp_rdesc_fixed) },
        { USB_VENDOR_ID_LOGITECH, USB_DEVICE_ID_LOGITECH_WII_WHEEL, wii_rdesc_fixed, sizeof(wii_rdesc_fixed) },
};

static int get_fixed_rdesc(unsigned short vendor, unsigned short product) {

    int i;
    for (i = 0; i < sizeof(rdesc_fixed) / sizeof(*rdesc_fixed); ++i) {
        if (rdesc_fixed[i].vendor == vendor && rdesc_fixed[i].product == product) {
            return i;
        }
    }
    return -1;
}

int uhidasync_create(const s_hid_info * hidDesc) {

    if (hidDesc == NULL) {

        PRINT_ERROR_OTHER("invalid argument")
        return -1;
    }

    int fd = open(UHID_PATH, O_RDWR | O_NONBLOCK);

    if (fd < 0) {
        PRINT_ERROR_ERRNO("open")
        return -1;
    }

    int device = add_device(fd);

    if (device < 0) {
        close(fd);
        return -1;
    }

    unsigned char * rd_data = hidDesc->reportDescriptor;
    unsigned short rd_size = hidDesc->reportDescriptorLength;

    // Some devices have a bad report descriptor, so fix it just like the kernel does.
    int fixed = get_fixed_rdesc(hidDesc->vendorId, hidDesc->productId);
    if (fixed != -1) {
        rd_data = rdesc_fixed[fixed].rdesc;
        rd_size = rdesc_fixed[fixed].length;
    }

    struct uhid_event ev = {
            .type = UHID_CREATE,
            .u.create = {
                    .rd_data = rd_data,
                    .rd_size = rd_size,
                    .version = hidDesc->version,
                    .country = hidDesc->countryCode,
                    // Make sure no device specific driver is loaded.
                    .bus = BUS_VIRTUAL,
                    .vendor = hidDesc->vendorId,
                    .product = hidDesc->productId,
            }
    };

    snprintf((char *) ev.u.create.name, sizeof(ev.u.create.name), "%s %s", hidDesc->manufacturerString, hidDesc->productString);
    snprintf((char *) ev.u.create.uniq, sizeof(ev.u.create.uniq), "GIMX %d %d", getpid(), device);

    if (uhid_write(fd, &ev) < 0) {
        uhidasync_close(device);
        return -1;
    }

    return device;
}

static int uhid_read(int device) {

    struct uhid_event ev;
    ssize_t ret;

    memset(&ev, 0, sizeof(ev));
    ret = read(uhidasync_devices[device].fd, &ev, sizeof(ev));
    if (ret == -1) {
        if (errno == EAGAIN) {
            return 0;
        }
        PRINT_ERROR_ERRNO("read")
        return -1;
    }

    switch (ev.type) {
    case UHID_START:
        break;
    case UHID_STOP:
        break;
    case UHID_OPEN:
        uhidasync_devices[device].opened = 1;
        break;
    case UHID_CLOSE:
        uhidasync_devices[device].opened = 0;
        break;
    case UHID_OUTPUT:
        break;
    case UHID_OUTPUT_EV:
        break;
    default:
        break;
    }

    return ret;
}

int uhidasync_is_opened(int device) {

    UHIDASYNC_CHECK_DEVICE(device, 0)

    while(uhid_read(device) > 0) {}

    return uhidasync_devices[device].opened;
}

int uhidasync_close(int device) {

    UHIDASYNC_CHECK_DEVICE(device, -1)

    struct uhid_event ev = { .type = UHID_DESTROY };
    uhid_write(uhidasync_devices[device].fd, &ev);
    close(uhidasync_devices[device].fd);
    uhidasync_devices[device].fd = -1;
    uhidasync_devices[device].opened = 0;

    return 1;
}

int uhidasync_write(int device, const void * buf, unsigned int count) {

    UHIDASYNC_CHECK_DEVICE(device, -1)

    if (count > UHID_DATA_MAX) {

        PRINT_ERROR_OTHER("count is higher than UHID_DATA_MAX")
        return -1;
    }

    struct uhid_event ev = { .type = UHID_INPUT, .u.input.size = count };
    memcpy(ev.u.input.data, buf, count);

    return uhid_write(uhidasync_devices[device].fd, &ev);
}

