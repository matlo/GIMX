/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <gimxusb/include/gusb.h>

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif

#define VENDOR 0x054c

static unsigned short products[] = { 0x05c4, 0x09cc };

#define TYPE_DS4 0
#define TYPE_TEENSY 1

static unsigned char type = TYPE_DS4;

static char* master = NULL;
static char* linkkey = NULL;
static char* slave = NULL;

static struct {
    unsigned char slave[6];
    unsigned char master[6];
    unsigned char linkkey[16];
} in;

static struct {
    unsigned char slave[6];
    unsigned char master[6];
    unsigned char linkkey[16];
} out;

static void usage() {
    fprintf(stderr, "Usage: ds4tool [-t] [-l lk] [-m master] [-s slave]\n");
    fprintf(stderr, "  -t: Teensy mode\n");
    fprintf(stderr, "  -l lk: the link key to set\n");
    fprintf(stderr, "  -m master: the master bdaddr to set\n");
    fprintf(stderr, "  -s slave: when -t is used, the slave bdaddr to set\n");
    fprintf(stderr, "            when -t is not used, the slave bdaddr to select\n");
}

int get_bdaddrs(struct gusb_device * device) {

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[16];
    } transfer = {
            .req = {
                    .bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_CLEAR_FEATURE,
                    .wValue = 0x0312,
                    .wIndex = 0x0000,
                    .wLength = sizeof(transfer.data)
            }
    };

    int res = gusb_write_timeout(device, 0, &transfer, sizeof(transfer.req), 5000);

    if (res > 0) {

        out.slave[0] = transfer.data[6];
        out.slave[1] = transfer.data[5];
        out.slave[2] = transfer.data[4];
        out.slave[3] = transfer.data[3];
        out.slave[4] = transfer.data[2];
        out.slave[5] = transfer.data[1];

        out.master[0] = transfer.data[15];
        out.master[1] = transfer.data[14];
        out.master[2] = transfer.data[13];
        out.master[3] = transfer.data[12];
        out.master[4] = transfer.data[11];
        out.master[5] = transfer.data[10];
    }

    return res;
}

void set_master(struct gusb_device * device) {

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[23];
    } transfer = {
            .req = {
                    .bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_SET_CONFIGURATION,
                    .wValue = 0x0313,
                    .wIndex = 0x0000,
                    .wLength = sizeof(transfer.data)
            },
            .data = {
                    0x13,
                    in.master[5], in.master[4], in.master[3], in.master[2], in.master[1], in.master[0],
                    in.linkkey[0], in.linkkey[1], in.linkkey[2], in.linkkey[3], in.linkkey[4], in.linkkey[5], in.linkkey[6], in.linkkey[7],
                    in.linkkey[8], in.linkkey[9], in.linkkey[10], in.linkkey[11], in.linkkey[12], in.linkkey[13], in.linkkey[14], in.linkkey[15]
            }
    };

    gusb_write_timeout(device, 0, &transfer, sizeof(transfer.req), 5000);
}

void set_slave(struct gusb_device * device) {

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[sizeof(in.slave)];
    } transfer = {
            .req = {
                    .bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_SET_CONFIGURATION,
                    .wValue = 0x0312,
                    .wIndex = 0x0000,
                    .wLength = sizeof(in.slave)
            },
            .data = {
                    in.slave[5], in.slave[4], in.slave[3], in.slave[2], in.slave[1], in.slave[0]
            }
    };

    gusb_write_timeout(device, 0, &transfer, sizeof(transfer.req), 5000);
}

int get_link_key(struct gusb_device * device) {

    struct {
        struct usb_ctrlrequest req;
        unsigned char data[sizeof(out.linkkey)];
    } transfer = {
            .req = {
                    .bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    .bRequest = USB_REQ_CLEAR_FEATURE,
                    .wValue = 0x0313,
                    .wIndex = 0x0000,
                    .wLength = sizeof(transfer.data)
            }
    };

    int res = gusb_write_timeout(device, 0, &transfer, sizeof(transfer), 5000);

    if (res > 0) {
        memcpy(out.linkkey, transfer.data, sizeof(transfer.data));
    }

    return res;
}

int process_device(struct gusb_device * device) {
    int i;

    if (get_bdaddrs(device) < 0) {
        return -1;
    }

    if (slave && type == TYPE_DS4) {
        if (memcmp(in.slave, out.slave, sizeof(in.slave))) {
            return -1;
        }
    }

    if (get_link_key(device) < 0) {
        if (type == TYPE_TEENSY) {
            return -1;
        }
        printf("Failed to get the link key (this is probably a Ds4).\n");
    } else {
        if (type == TYPE_DS4) {
            return -1;
        }
    }

    printf("Current Bluetooth master: ");
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", out.master[0], out.master[1], out.master[2], out.master[3], out.master[4], out.master[5]);

    printf("Current Bluetooth Device Address: ");
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", out.slave[0], out.slave[1], out.slave[2], out.slave[3], out.slave[4], out.slave[5]);

    if (type == TYPE_TEENSY) {
        printf("Current link key: ");
        for (i = 0; i < 16; ++i) {
            printf("%02X", out.linkkey[i]);
        }
        printf("\n");
    }

    if (master) {
        printf("Setting master bdaddr to %s\n", master);
        printf("Setting link key to %s\n", linkkey);
        set_master(device);
    }
    if (slave && type == TYPE_TEENSY) {
        printf("Setting slave bdaddr to %s\n", slave);
        set_slave(device);
    }
    return 0;
}

int bdaddr2hex(const char * str, unsigned char hex[6]) {

    // there's no hh conversion in Windows!
    unsigned int tmp[6];
    if (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5) != 6) {
        return -1;
    }
    int i;
    for (i = 0; i < 6; ++i) {
        hex[i] = tmp[i];
    }
    return 0;
}

int linkkey2hex(const char * str, unsigned char hex[16]) {

    // there's no hh conversion in Windows!
    unsigned int tmp[16] = { };
    if (sscanf(str, "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6, tmp + 7, tmp + 8,
            tmp + 9, tmp + 10, tmp + 11, tmp + 12, tmp + 13, tmp + 14, tmp + 15) != 16) {
        return -1;
    }
    int i;
    for (i = 0; i < 16; ++i) {
        hex[i] = tmp[i];
    }
    return 0;
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "l:m:s:t::")) != -1) {
        switch (opt) {
        case 'm':
            master = optarg;
            break;
        case 's':
            slave = optarg;
            break;
        case 'l':
            linkkey = optarg;
            break;
        case 't':
            type = TYPE_TEENSY;
            break;
        default: /* '?' */
            usage();
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (slave) {
        if (bdaddr2hex(slave, in.slave) < 0) {
            usage();
            exit(EXIT_FAILURE);
        }
    }
    if (master) {
        if (bdaddr2hex(master, in.master) < 0) {
            usage();
            exit(EXIT_FAILURE);
        }
    }
    if (linkkey) {
        if (linkkey2hex(linkkey, in.linkkey) < 0) {
            usage();
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {

    int ret = 0;
    int status;

    read_args(argc, argv);

    if (gusb_init(&((GPOLL_INTERFACE){ REGISTER_FUNCTION, REMOVE_FUNCTION })) < 0) {
        return -1;
    }

    struct gusb_device_info * devs = gusb_enumerate(VENDOR, 0x0000);

    struct gusb_device_info * current;
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

        struct gusb_device * device = gusb_open_path(current->path);

        if (device != NULL) {

            status = process_device(device);

            gusb_close(device);

            if (status == 0 && (slave || master)) {
                break;
            }
        }
    }

    gusb_free_enumeration(devs);

    gusb_exit();

    return ret;
}

