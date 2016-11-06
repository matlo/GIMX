/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "hidinput.h"
#ifdef UHID
#include <guhid.h>
#endif
#include <common/gerror.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USB_VENDOR_ID_LOGITECH                  0x046d

#define USB_PRODUCT_ID_LOGITECH_FORMULA_YELLOW   0xc202 // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_GP       0xc20e // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE    0xc291
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP 0xc293
#define USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE    0xc294
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL       0xc295
#define USB_PRODUCT_ID_LOGITECH_DFP_WHEEL        0xc298
#define USB_PRODUCT_ID_LOGITECH_G25_WHEEL        0xc299
#define USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL       0xc29a
#define USB_PRODUCT_ID_LOGITECH_G27_WHEEL        0xc29b
#define USB_PRODUCT_ID_LOGITECH_WII_WHEEL        0xc29c // rumble only
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2      0xca03
#define USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL  0xca04 // rumble only
#define USB_PRODUCT_ID_LOGITECH_G920_WHEEL       0xc262 // does not support classic format
#define USB_PRODUCT_ID_LOGITECH_G29_WHEEL        0xc24f

#define FF_LG_OUTPUT_REPORT_SIZE 7

static struct {
    int opened;
#ifdef UHID
    int uhid;
#endif
} hid_devices[HIDINPUT_MAX_DEVICES] = {};

#define MAKE_IDS(USB_PRODUCT_ID) \
    { .vendor_id = USB_VENDOR_ID_LOGITECH, .product_id = USB_PRODUCT_ID }

static s_hidinput_ids ids[] = {
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_DFP_WHEEL),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_G25_WHEEL),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_G27_WHEEL),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2),
        MAKE_IDS(USB_PRODUCT_ID_LOGITECH_G29_WHEEL),
        { .vendor_id = 0, .product_id = 0 },
};

static void clear_device(int device) {

  memset(hid_devices + device, 0x00, sizeof(*hid_devices));
#ifdef UHID
  hid_devices[device].uhid = -1;
#endif
}

static int close_device(int device) {

#ifdef UHID
    if (hid_devices[device].uhid >= 0) {
        guhid_close(hid_devices[device].uhid);
    }
#endif

    clear_device(device);

    return 0;
}

static int init(int(*callback)(GE_Event*) __attribute__((unused))) {

    return 0;
}

#ifdef UHID
static int process(int device, const void * report, unsigned int size) {

    int ret = guhid_write(hid_devices[device].uhid, report, size);

    return ret < 0 ? -1 : 0;
}
#else
static int process(int device __attribute__((unused)), const void * report __attribute__((unused)),
    unsigned int size __attribute__((unused))) {

    return 0;
}
#endif

#ifdef UHID
/* Fixed report descriptors for Logitech Driving Force (and Pro)
 * wheel controllers
 *
 * The original descriptors hide the separate throttle and brake axes in
 * a custom vendor usage page, providing only a combined value as
 * GenericDesktop.Y.
 * These descriptors remove the combined Y axis and instead report
 * separate throttle (Y) and brake (RZ).
 */
static __u8 df_rdesc_fixed[] = {
        0x05, 0x01, /*  Usage Page (Desktop),                   */
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

static __u8 dfp_rdesc_fixed[] = {
        0x05, 0x01, /*  Usage Page (Desktop),                   */
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

static __u8 fv_rdesc_fixed[] = {
        0x05, 0x01, /*  Usage Page (Desktop),                   */
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

static __u8 momo_rdesc_fixed[] = {
        0x05, 0x01, /*  Usage Page (Desktop),               */
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
        //0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,        //     Usage (0x01)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x09, 0x31,        //     Usage (Y)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        //0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
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

static __u8 momo2_rdesc_fixed[] = {
        0x05, 0x01, /*  Usage Page (Desktop),               */
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
static __u8 wii_rdesc_fixed[] = {
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

#define MAKE_RDESC(USB_PRODUCT_ID, RDESC) \
        { USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID, RDESC,  sizeof(RDESC) }

static struct {
    unsigned short vendor;
    unsigned short product;
    unsigned char * rdesc;
    unsigned short length;
} rdesc_fixed[] = {
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, ffgp_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE,    df_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL,       momo_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2,      momo2_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL,  fv_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,        dfp_rdesc_fixed),
        MAKE_RDESC(USB_PRODUCT_ID_LOGITECH_WII_WHEEL,        wii_rdesc_fixed),
};

static void fix_rdesc(s_hid_info * hid_info) {

    unsigned int i;
    for (i = 0; i < sizeof(rdesc_fixed) / sizeof(*rdesc_fixed); ++i) {
        if (rdesc_fixed[i].vendor == hid_info->vendor_id && rdesc_fixed[i].product == hid_info->product_id) {
            hid_info->reportDescriptor = rdesc_fixed[i].rdesc;
            hid_info->reportDescriptorLength = rdesc_fixed[i].length;
        }
    }
}
#endif

typedef struct
{
    unsigned short product_id;
    unsigned char command[FF_LG_OUTPUT_REPORT_SIZE];
} s_native_mode;

static s_native_mode native_modes[] =
{
    { USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, { 0x00, 0xf8, 0x09, 0x03, 0x01 } },
    { USB_PRODUCT_ID_LOGITECH_G27_WHEEL,  { 0x00, 0xf8, 0x09, 0x04, 0x01 } },
    { USB_PRODUCT_ID_LOGITECH_G25_WHEEL,  { 0x00, 0xf8, 0x10 } },
    { USB_PRODUCT_ID_LOGITECH_DFP_WHEEL,  { 0x00, 0xf8, 0x01 } },
};

static s_native_mode * get_native_mode_command(unsigned short product, unsigned short bcdDevice)
{
  unsigned short native = 0x0000;

  if(((USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE == product) || (USB_PRODUCT_ID_LOGITECH_DFP_WHEEL == product))
      && (0x1300 == (bcdDevice & 0xff00))) {
    native = USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL;
  } else if(((USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE == product) || (USB_PRODUCT_ID_LOGITECH_DFP_WHEEL == product) || (USB_PRODUCT_ID_LOGITECH_G25_WHEEL == product))
      && (0x1230 == (bcdDevice & 0xfff0))) {
    native = USB_PRODUCT_ID_LOGITECH_G27_WHEEL;
  } else if(((USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE == product) || (USB_PRODUCT_ID_LOGITECH_DFP_WHEEL == product))
      && (0x1200 == (bcdDevice & 0xff00))) {
    native = USB_PRODUCT_ID_LOGITECH_G25_WHEEL;
  } else if((USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE == product)
      && (0x1000 == (bcdDevice & 0xf000))) {
    native = USB_PRODUCT_ID_LOGITECH_DFP_WHEEL;
  }

  unsigned int i;
  for (i = 0; i < sizeof(native_modes) / sizeof(*native_modes); ++i) {
    if (native_modes[i].product_id == native) {
      return native_modes + i;
    }
  }

  return NULL;
}

#ifndef WIN32
static int send_native_mode(const struct ghid_device * dev, const s_native_mode * native_mode) {

    int device = ghid_open_path(dev->path);
    if (device < 0) {
        return -1;
    }
    int ret = ghid_write_timeout(device, native_mode->command, sizeof(native_mode->command), 1000);
    if (ret <= 0) {
        fprintf(stderr, "failed to send native mode command for HID device %s (PID=%04x)\n", dev->path, dev->product_id);
        ret = -1;
    } else {
        printf("native mode command sent to HID device %s (PID=%04x)\n", dev->path, dev->product_id);
        ret = 0;
    }
    ghid_close(device);
    return ret;
}

static int check_native_mode(const struct ghid_device * dev, unsigned short product_id) {

    // wait up to 5 seconds for the device to enable native mode
    int reset = 0;
    int cpt = 0;
    do {
        // sleep 1 second between each retry
        int i;
        for (i = 0; i < 10; ++i) {
            usleep(100000);
        }
        ++cpt;
        struct ghid_device * hid_devs = ghid_enumerate(USB_VENDOR_ID_LOGITECH, product_id);
        struct ghid_device * current;
        for (current = hid_devs; current != NULL && reset == 0; current = current->next) {
            if (strcmp(current->path, dev->path) == 0) {
                printf("native mode enabled for HID device %s (PID=%04x)\n", dev->path, product_id);
                reset = 1;
            }
        }
        ghid_free_enumeration(hid_devs);
    } while (cpt < 5 && !reset);

    return (reset == 1) ? 0 : -1;
}

static int set_native_mode(const struct ghid_device * dev, const s_native_mode * native_mode) {

    if (native_mode) {
        if (send_native_mode(dev, native_mode) < 0) {
            return -1;
        }
        if (check_native_mode(dev, native_mode->product_id) < 0) {
            fprintf(stderr, "failed to enable native mode for HID device %s\n", dev->path);
            return -1;
        }
    } else {
        printf("native mode is already enabled for HID device %s (PID=%04x)\n", dev->path, dev->product_id);
    }
    return 0;
}
#else
static int set_native_mode(const struct ghid_device * dev __attribute__((unused)), const s_native_mode * native_mode) {

    if (native_mode) {
        printf("Found Logitech wheel not in native mode.\n");
        const char * download = NULL;
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        switch (info.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
            case PROCESSOR_ARCHITECTURE_IA64:
            download = "http://gimx.fr/download/LGS64";
            break;
            case PROCESSOR_ARCHITECTURE_INTEL:
            download = "http://gimx.fr/download/LGS32";
            break;
        }
        if (download != NULL) {
            printf("Please install Logitech Gaming Software from: %s.\n", download);
        }
    }
    return 0;
}
#endif

static int open_device(const struct ghid_device * dev) {

    s_native_mode * native_mode = get_native_mode_command(dev->product_id, dev->bcdDevice);
    if (set_native_mode(dev, native_mode) < 0) {
        return -1;
    }

#ifndef WIN32
    int device = ghid_open_path(dev->path);
    if (device < 0) {
        return -1;
    }

    if (hid_devices[device].opened != 0) {
        ghid_close(device);
        return -1;
    }

    hid_devices[device].opened = 1;

#ifdef UHID
    const s_hid_info * hid_info = ghid_get_hid_info(device);
    if (hid_info == NULL) {
        close_device(device);
        return -1;
    }

    s_hid_info fixed_hid_info = *hid_info;

    // Some devices have a bad report descriptor, so fix it just like the kernel does.
    fix_rdesc(&fixed_hid_info);

    hid_devices[device].uhid = guhid_create(&fixed_hid_info, device);
    if (hid_devices[device].uhid < 0) {
        close_device(device);
        return -1;
    }
#endif

    return device;
#else
    return -1;
#endif
}

static s_hidinput_driver driver = {
        .ids = ids,
        .init = init,
        .open = open_device,
        .process = process,
        .close = close_device,
};

void logitechwheel_constructor(void) __attribute__((constructor));
void logitechwheel_constructor(void) {
    unsigned int device;
    for (device = 0; device < sizeof(hid_devices) / sizeof(*hid_devices); ++device) {
        clear_device(device);
    }
    if (hidinput_register(&driver) < 0) {
        exit(-1);
    }
}
