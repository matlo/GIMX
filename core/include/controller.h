/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <connectors/protocol.h>
#include <gimx-network-protocol/protocol.h>
#include <config.h>
#include <gimxserial/include/gserial.h>
#include <gimxcontroller/include/controller.h>

#ifndef WIN32
#include <netinet/in.h>
#else
#include <connectors/windows/sockets.h>
#endif

#include <stdio.h>

typedef enum {
    E_ADAPTER_TYPE_NONE,
    E_ADAPTER_TYPE_BLUETOOTH,
    E_ADAPTER_TYPE_DIY_USB,
    E_ADAPTER_TYPE_REMOTE_GIMX,
    E_ADAPTER_TYPE_GPP,
} e_adapter_type;

typedef struct {
    e_adapter_type atype;
    union {
        struct {
            int index;
            char* bdaddr_dst;
        } bt;
        struct {
            char* portname;
            int device;
            s_packet packet;
            unsigned int bread;
        } serial;
        struct {
            in_addr_t ip;
            unsigned short port;
            int fd;
            union {
                unsigned char buf[sizeof(s_network_packet_in_report) + AXIS_MAX * sizeof(* ((s_network_packet_in_report * )NULL)->axes)];
                s_network_packet_in_report report;
            };
            int last_axes[AXIS_MAX];
        } remote;
    };
    in_addr_t src_ip;
    unsigned short src_port;
    int src_fd;
    e_controller_type ctype;
    int event;
    int axis[AXIS_MAX];
    int change;
    int send_command;
    int ts_axis[AXIS_MAX][2]; //issue 15
    s_report_packet report[2]; //the xbox one guide button needs a dedicated report
    int status;
    int joystick;
    struct {
        int joystick;
        struct
        {
            unsigned short vendor;
            unsigned short product;
        } usb_ids;
        unsigned char has_rumble;
        unsigned char has_ffb;
        struct {
            int id;
            int write_pending;
            int read_pending;
        } hid;
    int ff_lg;
    int ff_conv;
    } haptic;
    unsigned char forward_out_reports;
    unsigned char process_ffb;
} s_adapter;

int adapter_detect();
int adapter_start();
int adapter_send();
void adapter_clean();

s_adapter* adapter_get(unsigned char index);
int adapter_set_port(unsigned char index, char* portname);

void adapter_set_device(int adapter, e_device_type device_type, int device_id);
int adapter_get_device(e_device_type device_type, int adapter);
int adapter_get_controller(e_device_type device_type, int device_id);

#ifndef WIN32
int adapter_hid_poll();
#endif

void adapter_set_axis(unsigned char adapter, int axis, int value);

int adapter_forward_control_in(int adapter, unsigned char* data, unsigned char length);
int adapter_forward_interrupt_in(int adapter, unsigned char* data, unsigned char length);

int adapter_is_usb_auth_required(int adapter);

void adapter_set_haptic(s_config_entry * entry, int force);
void adapter_set_ffb_tweaks(int adapter);

#endif /* CONTROLLER_H_ */
