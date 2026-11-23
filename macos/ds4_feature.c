/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ds4_feature.h"
#include <string.h>

static unsigned int le16(const uint8_t *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

int ds4_feature_transfer(void *device, ds4_feature_get_fn get_report,
                         ds4_feature_set_fn set_report, const uint8_t *request,
                         size_t request_size, uint8_t *response,
                         size_t response_capacity)
{
    if (!device || !request || request_size < 8)
        return -1;
    unsigned int value = le16(request + 2);
    unsigned int index = le16(request + 4);
    unsigned int length = le16(request + 6);
    unsigned int report_id = value & 0xff;
    if ((value >> 8) != 3 || index != 0 || !report_id || !length || length > 254)
        return -1;

    if (request[0] == 0xa1 && request[1] == 0x01) {
        if (request_size != 8 || !response || response_capacity < length || !get_report)
            return -1;
        memset(response, 0, length);
        response[0] = (uint8_t)report_id;
        int n = get_report(device, response, length);
        if (n < 1 || (unsigned int)n > length || response[0] != report_id)
            return -1;
        return n;
    }
    if (request[0] == 0x21 && request[1] == 0x09) {
        if (!set_report || request_size != 8 + length || request[8] != report_id)
            return -1;
        int n = set_report(device, request + 8, length);
        return n == (int)length ? 0 : -1;
    }
    return -1;
}
