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
#include "haptic/haptic_core.h"
#include <gimx.h>

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
    struct {
        int index;
        char* bdaddr_dst;
    } bt;
    struct {
        char* portname;
        struct gserial_device * device;
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
    in_addr_t src_ip;
    unsigned short src_port;
    int src_fd;
    e_controller_type ctype;
  struct {
    e_controller_axis_index index;
    int pressed;
  } activation_button;
    int event;
    int axis[AXIS_MAX];
    int change;
    int send_command;
    int ts_axis[AXIS_MAX][2]; //issue 15
    s_report_packet report[2]; //the xbox one guide button needs a dedicated report
    int status;
    int joystick;
    unsigned char forward_out_reports;
    unsigned char process_ffb;
    int haptic_sink_joystick;
    struct haptic_core * ff_core;
    struct {
      unsigned int counter;
      unsigned int timeout;
    } inactivity;
    struct stats * s;
} s_adapter;

int adapter_detect();
int adapter_start();
int adapter_send();
e_gimx_status adapter_clean();

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

void adapter_set_haptic_sink(int adapter, int joystick, int force);
void adapter_set_haptic_tweaks(int adapter, const s_haptic_core_tweaks * tweaks);

#endif /* CONTROLLER_H_ */
