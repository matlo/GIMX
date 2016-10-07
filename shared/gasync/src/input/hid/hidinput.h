/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef HIDINPUT_H_
#define HIDINPUT_H_

#include <ginput.h>
#include <ghid.h>
#include <gpoll.h>

#define HIDINPUT_MAX_DEVICES GHID_MAX_DEVICES

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
    int interface_number; // -1 means any interface
} s_hidinput_ids;

typedef struct {
    s_hidinput_ids * ids;
    // Give the event callback to the driver
    int (* init)(int(*callback)(GE_Event*));
    // Open a device.
    // Synchronous transfers are allowed in this function.
    int (* open)(const struct ghid_device * dev);
    // Process a report.
    int (* process)(int device, const void * report, unsigned int size);
    // Close a device.
    int (* close)(int device);
} s_hidinput_driver;

int hidinput_register(s_hidinput_driver * driver);

int hidinput_init(const GPOLL_INTERFACE * gpoll_interface, int(*callback)(GE_Event*));
int hidinput_poll();
void hidinput_quit();

int hidinput_set_callbacks(int device, int user, int (* write_cb)(int user, int transfered), int (* close_cb)(int user));

#endif /* HIDINPUT_H_ */
