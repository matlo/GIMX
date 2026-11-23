/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef GIMX_MACOS_DS4_FEATURE_H
#define GIMX_MACOS_DS4_FEATURE_H

#include <stddef.h>
#include <stdint.h>

typedef int (*ds4_feature_get_fn)(void *device, unsigned char *data, size_t size);
typedef int (*ds4_feature_set_fn)(void *device, const unsigned char *data, size_t size);

/* Translate a USB HID feature-report control request. Returns response length
 * for GET, zero for successful SET, and -1 on invalid or unsupported input. */
int ds4_feature_transfer(void *device, ds4_feature_get_fn get_report,
                         ds4_feature_set_fn set_report, const uint8_t *request,
                         size_t request_size, uint8_t *response,
                         size_t response_capacity);

#endif
