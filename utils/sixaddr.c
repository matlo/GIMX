/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <gusb.h>

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif

#define VENDOR 0x054c

static unsigned short products[] = { 0x0268 };

static struct {
    unsigned char slave[6];
    unsigned char master[6];
} out;

int get_bdaddrs(int device) {

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[8];
    } transfer_master = {
            .req = {
                    .bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_CLEAR_FEATURE,
                    .wValue = 0x03f5,
                    .wIndex = 0x0000,
                    .wLength = sizeof(transfer_master.data)
            }
    };

    int res = gusb_write_timeout(device, 0, &transfer_master, sizeof(transfer_master.req), 5000);

    if (res > 0) {
        out.master[0] = transfer_master.data[2];
        out.master[1] = transfer_master.data[3];
        out.master[2] = transfer_master.data[4];
        out.master[3] = transfer_master.data[5];
        out.master[4] = transfer_master.data[6];
        out.master[5] = transfer_master.data[7];
    }

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[18];
    } transfer_slave = {
            .req = {
                    .bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_CLEAR_FEATURE,
                    .wValue = 0x3f2,
                    .wIndex = 0x0000,
                    .wLength = sizeof(transfer_slave.data)
            }
    };

    res = gusb_write_timeout(device, 0, &transfer_slave, sizeof(transfer_slave.req), 5000);

    if (res > 0) {
        out.slave[0] = transfer_slave.data[4];
        out.slave[1] = transfer_slave.data[5];
        out.slave[2] = transfer_slave.data[6];
        out.slave[3] = transfer_slave.data[7];
        out.slave[4] = transfer_slave.data[8];
        out.slave[5] = transfer_slave.data[9];
    }

    return res;
}

int process_device(int device) {

    if (get_bdaddrs(device) < 0) {
        return -1;
    }

    printf("Current Bluetooth master: ");
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", out.master[0], out.master[1], out.master[2], out.master[3], out.master[4], out.master[5]);

    printf("Current Bluetooth Device Address: ");
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", out.slave[0], out.slave[1], out.slave[2], out.slave[3], out.slave[4], out.slave[5]);

    return 0;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {

    int ret = 0;
    int status;

    if (gusb_init(&((GPOLL_INTERFACE){ REGISTER_FUNCTION, REMOVE_FUNCTION })) < 0) {
        return -1;
    }

    struct gusb_device * devs = gusb_enumerate(VENDOR, 0x0000);

    struct gusb_device * current;
    for (current = devs; current != NULL; current = current->next) {

        unsigned int i;
        for (i = 0; i < sizeof(products) / sizeof(*products); ++i) {
            if (current->product_id == products[i]) {
                break;
            }
        }

        if (i == sizeof(products) / sizeof(*products)) {
            continue;
        }

        int device = gusb_open_path(current->path);

        if (device >= 0) {

            status = process_device(device);

            gusb_close(device);

            if (status == 0) {
                break;
            }
        }
    }

    gusb_free_enumeration(devs);

    gusb_exit();

    return ret;
}

