/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ds4_feature.h"
#include <assert.h>
#include <stdio.h>

static int read_report(void *device, unsigned char *data, size_t size)
{
    (void)device;
    assert(size == 16 && data[0] == 0xf2);
    data[1] = 0x42;
    return 16;
}

static int write_report(void *device, const unsigned char *data, size_t size)
{
    (void)device;
    assert(size == 3 && data[0] == 0xf0 && data[2] == 2);
    return (int)size;
}

int main(void)
{
    int device = 1;
    unsigned char response[16];
    unsigned char get[] = {0xa1, 1, 0xf2, 3, 0, 0, 16, 0};
    unsigned char set[] = {0x21, 9, 0xf0, 3, 0, 0, 3, 0, 0xf0, 1, 2};
    assert(ds4_feature_transfer(&device, read_report, write_report, get, sizeof(get), response, sizeof(response)) == 16);
    assert(response[1] == 0x42);
    assert(ds4_feature_transfer(&device, read_report, write_report, set, sizeof(set), response, sizeof(response)) == 0);
    assert(ds4_feature_transfer(&device, read_report, write_report, set, sizeof(set)-1, response, sizeof(response)) == -1);
    get[4] = 1;
    assert(ds4_feature_transfer(&device, read_report, write_report, get, sizeof(get), response, sizeof(response)) == -1);
    puts("DS4 feature request tests passed");
    return 0;
}
