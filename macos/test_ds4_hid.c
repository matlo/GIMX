/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ds4_feature.h"
#include "hidapi.h"
#include <stdio.h>

static int read_feature(void *device, unsigned char *data, size_t size)
{
    return hid_get_feature_report(device, data, size);
}

int main(void)
{
    hid_device *device = hid_open(0x054c, 0x09cc, NULL);
    if (!device)
        device = hid_open(0x054c, 0x05c4, NULL);
    if (!device) {
        fputs("Could not open a DS4. Connect it over USB.\n", stderr);
        return 1;
    }
    unsigned char request[] = {0xa1, 1, 0xf2, 3, 0, 0, 16, 0};
    unsigned char response[16];
    int length = ds4_feature_transfer(device, read_feature, NULL, request,
                                      sizeof(request), response, sizeof(response));
    hid_close(device);
    if (length != 16) {
        fprintf(stderr, "Signing-status feature read failed (result %d).\n", length);
        return 1;
    }
    printf("DS4 signing-status feature report: %d bytes, ID 0x%02x\n", length, response[0]);
    return 0;
}
