/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>

#include <gimx.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <mainloop.h>
#include <display.h>
#include <stats.h>
#include <connectors/protocol.h>
#include <connectors/gpp_con.h>
#include <connectors/usb_con.h>
#include "connectors/sixaxis.h"
#ifndef WIN32
#include "connectors/btds4.h"
#endif
#include <gimxcontroller/include/controller.h>
#include <gimxpoll/include/gpoll.h>
#include <gimxtime/include/gtime.h>

#include <haptic/haptic_core.h>

static const int baudrates[] = { 2000000, 1000000, 500000 }; //bps
#define BAUDRATE 500000 //bps
#define ADAPTER_TIMEOUT 1000 //millisecond
/*
 * The adapter restarts about 15ms after receiving the reset command.
 * This time is doubled so as to include the reset command transfer duration.
 */
#define ADAPTER_RESET_TIME 30000 //microseconds
/*
 * The number of times the adapter is queried for its type, before it is assumed as unreachable.
 */
#define ADAPTER_INIT_RETRIES 10
/*
 * The number of times the adapter is queried for its baudrate before assuming baudrate is not supported.
 */
#define ADAPTER_BAUDRATE_RETRIES 3

static s_adapter adapters[MAX_CONTROLLERS] = {};

/*
 * These tables are used to retrieve the default controller for a device and vice versa.
 */
static int adapter_device[E_DEVICE_TYPE_NB][MAX_CONTROLLERS];
static int device_adapter[E_DEVICE_TYPE_NB][MAX_DEVICES];

void adapter_init_static(void) __attribute__((constructor));
void adapter_init_static(void)
{
  unsigned int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapters[i].atype = E_ADAPTER_TYPE_NONE;
    adapters[i].ctype = C_TYPE_NONE;
    adapters[i].haptic_sink_joystick = -1;
    adapters[i].serial.bread = 0;
    for(j = 0; j < MAX_REPORTS; ++j)
    {
      adapters[i].report[j].type = BYTE_IN_REPORT;
    }
    adapters[i].status = 0;
    adapters[i].joystick = -1;
    adapters[i].mperiod = -1;
  }
  for(j=0; j<E_DEVICE_TYPE_NB; ++j)
  {
    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
      adapter_device[j][i] = -1;
    }
    for(i=0; i<MAX_DEVICES; ++i)
    {
      device_adapter[j][i] = -1;
    }
  }
}

static void dump(unsigned char * packet, unsigned char length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    if(i && !(i%8))
    {
      ginfo("\n");
    }
    ginfo("0x%02x ", packet[i]);
  }
  ginfo("\n");
}

#define DEBUG_PACKET(TYPE, DATA, LENGTH) \
  if(gimx_params.debug.controller) \
  { \
    ginfo("%s\n", __func__); \
    ginfo("type: 0x%02x, length: %u\n", TYPE, LENGTH); \
    dump(DATA, LENGTH); \
  }

static int is_gimx_adapter(int adapter) {
    return (adapters[adapter].atype == E_ADAPTER_TYPE_DIY_USB && adapters[adapter].serial.portname)
                || (adapters[adapter].atype == E_ADAPTER_TYPE_PROXY && adapters[adapter].proxy.remote.ip);
}

/*
 * Set the port of the controller.
 * If it's already used for another controller, do nothing.
 */
int adapter_set_port(unsigned char adapter, char* portname)
{
  unsigned char i;
  if(adapter >= MAX_CONTROLLERS)
  {
    return -1;
  }
  for(i = 0; i < adapter; ++i)
  {
    if(adapters[i].serial.portname && !strcmp(adapters[i].serial.portname, portname))
    {
      return -1;
    }
  }
  adapters[adapter].serial.portname = portname;
  return 0;
}

inline s_adapter * adapter_get(unsigned char adapter)
{
  if(adapter < MAX_CONTROLLERS)
  {
    return adapters + adapter;
  }
  return NULL;
}

void adapter_set_axis(unsigned char adapter, int axis, int value)
{
  if(axis >= 0 && axis < AXIS_MAX)
  {
    adapters[adapter].axis[axis] = value;
  }
}

static void adapter_dump_state(int adapter)
{
  int i;
  gtime now = gtime_gettime();

  int* axis = adapters[adapter].axis;

  // we don't use gstatus to avoid flushing stdout multiple times

  printf("%d %lu.%06lu", adapter, GTIME_SECPART(now), GTIME_USECPART(now));

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", controller_get_axis_name(adapters[adapter].ctype, i), axis[i]);
  }

  printf("\n");

  fflush(stdout);
}

static int adapter_write(int adapter, const void * buf, unsigned int count)
{
  int ret = 0;
  if (adapters[adapter].atype == E_ADAPTER_TYPE_DIY_USB && adapters[adapter].serial.device != NULL)
  {
    ret = gserial_write(adapters[adapter].serial.device, buf, count);
  }
  else if (adapters[adapter].atype == E_ADAPTER_TYPE_PROXY && adapters[adapter].proxy.socket != NULL)
  {
    ret = gudp_send(adapters[adapter].proxy.socket, buf, count, adapters[adapter].proxy.remote);
  }
  return ret;
}

static int adapter_start_serialasync(int adapter);
static e_gimx_status adapter_open(int i, unsigned int baudrate);

static int proxy_src_read_callback(void * user, const void * buf, int status, struct gudp_address address)
{
    int i = (intptr_t) user;

    s_adapter * adapter = adapters + i;

    if (status <= 0)
    {
        gwarn("empty packet\n");
        return 0;
    }

    if ((size_t)status < sizeof(s_header))
    {
        gwarn("invalid packet size: %d\n", status);
        return 0;
    }

    uint8_t type = ((uint8_t *)buf)[0];
    uint8_t length = ((uint8_t *)buf)[1];
    uint8_t * data = ((uint8_t *)buf) + 2;

    if (length + sizeof(s_header) != (size_t)status)
    {
        gwarn("invalid packet size: %d\n", status);
        return 0;
    }

    if (adapter->proxy.remote.ip && adapter->proxy.remote.port)
    {
        if (adapter->proxy.remote.ip != address.ip || adapter->proxy.remote.port != address.port)
        {
            gwarn("reject packet from %s:%hu\n", gudp_ip_str(address.ip), address.port);
            return 0;
        }
    }

    if (type != BYTE_RESET) {
        adapter->proxy.remote.ip = address.ip;
        adapter->proxy.remote.port = address.port;
    } else {
        adapter->proxy.remote.ip = 0;
        adapter->proxy.remote.port = 0;
    }

    DEBUG_PACKET(type, data, length);

    if (adapter_write(i, buf, status) < 0)
    {
        return -1;
    }

    if (type == BYTE_BAUDRATE && length == 1)
    {
        if (adapter_open(i, data[0] * 100000U) != E_GIMX_STATUS_SUCCESS)
        {
          return -1;
        }
        if (adapter->atype == E_ADAPTER_TYPE_DIY_USB && adapter->serial.device)
        {
          if(adapter_start_serialasync((intptr_t) user) < 0)
          {
            gerror(_("failed to start the GIMX adapter asynchronous processing.\n"));
            return -1;
          }
        }
    }

    if (type == BYTE_RESET)
    {
        if (adapter_open(i, BAUDRATE) != E_GIMX_STATUS_SUCCESS)
        {
          return -1;
        }
        if (adapter->atype == E_ADAPTER_TYPE_DIY_USB && adapter->serial.device)
        {
          if(adapter_start_serialasync((intptr_t) user) < 0)
          {
            gerror(_("failed to start the GIMX adapter asynchronous processing.\n"));
            return -1;
          }
        }
    }

    return 0;
}

static int adapter_process_packet(int adapter, s_packet* packet);

static int proxy_dst_read_callback(void * user, const void * buf, int status, struct gudp_address address) {

    int i = (intptr_t) user;

    s_adapter * adapter = adapters + i;

    if (adapter->proxy.remote.ip && adapter->proxy.remote.port)
    {
        if (adapter->proxy.remote.ip != address.ip || adapter->proxy.remote.port != address.port)
        {
            gwarn("reject packet from %s:%hu\n", gudp_ip_str(address.ip), address.port);
            return 0;
        }
    }

    if (status <= 0)
    {
        return 0;
    }
    if ((size_t) status < sizeof(s_header))
    {
        gwarn("invalid packet size: %d\n", status);
        return 0;
    }
    uint8_t length = ((uint8_t *)buf)[1];
    if (length != status - sizeof(s_header))
    {
        gwarn("invalid packet length: %hu (received %zu)\n", length, status - sizeof(s_header));
        return 0;
    }

    return adapter_process_packet(i, (s_packet*) buf);
}

/*
 * Read a packet from a remote GIMX client.
 * The packet can be:
 * - a "get controller type" request
 * - a report to be sent.
 * Note that the socket operations should not block.
 */
static int network_read_callback(void * user, const void * buf, int status, struct gudp_address address)
{
  int adapter = (intptr_t) user;

  if (status <= 0)
  {
      return 0;
  }
  if(status < 1)
  {
    gwarn("invalid packet size: %d\n", status);
    return 0;
  }
  uint8_t type = ((uint8_t *)buf)[0];
  switch(type)
  {
  case E_NETWORK_PACKET_CONTROLLER:
    {
      // send the answer
      s_network_packet_controller ctype =
      {
        .packet_type = E_NETWORK_PACKET_CONTROLLER,
        .controller_type = adapters[adapter].ctype
      };
      if (gudp_send(adapters[adapter].src_socket, (void *)&ctype, sizeof(ctype), address) < 0)
      {
        gwarn("%s: can't send controller type\n", __func__);
        return 0;
      }
    }
    break;
  case E_NETWORK_PACKET_IN_REPORT:
    {
      s_network_packet_in_report * report = (s_network_packet_in_report *) buf;
      if((unsigned int) status != sizeof(* report) + report->nbAxes * sizeof(* report->axes))
      {
        gwarn("%s: wrong packet size: %u %zu\n", __func__, status, sizeof(* report) + report->nbAxes * sizeof(* report->axes));
        return 0;
      }
      // store the report (no answer)
      unsigned char i;
      for (i = 0; i < report->nbAxes; ++i)
      {
        unsigned char offset = ((report->axes[i].index & 0x80) ? abs_axis_0 : 0) + (report->axes[i].index & 0x7f);
        if (offset < AXIS_MAX)
        {
          adapters[adapter].axis[offset] = gudp_ntohl(report->axes[i].value);
        }
        else
        {
          gwarn("%s: bad axis index: %s %hu\n", __func__, (report->axes[i].index & 0x80) ? "abs" : "rel", report->axes[i].index & 0x7f);
        }
      }
      adapters[adapter].send_command = 1;
    }
    break;
  }
  // require a report to be sent immediately, except for a Sixaxis controller working over bluetooth
  if(adapters[adapter].ctype == C_TYPE_SIXAXIS && adapters[adapter].atype == E_ADAPTER_TYPE_BLUETOOTH)
  {
    return 0;
  }
  return 1;
}

int adapter_close_callback(void * user __attribute__((unused)))
{
  set_done();

  return 0;
}

/*
 * Set the default device for a controller.
 */
void adapter_set_device(int adapter, e_device_type device_type, int device_id)
{
  int type_index = device_type - 1;

  if(adapter < 0 || adapter >= MAX_CONTROLLERS)
  {
    gwarn("invalid adapter: %d\n", adapter);
    return;
  }
  if(type_index < 0 || type_index >= E_DEVICE_TYPE_NB)
  {
    gwarn("invalid device type: %d", device_type);
    return;
  }
  if(device_id < 0 || device_id > MAX_DEVICES)
  {
    gwarn("invalid device id: %d\n", device_id);
    return;
  }
  if(adapter_device[type_index][adapter] < 0)
  {
    if (device_type == E_DEVICE_TYPE_MOUSE) {
        adapters[adapter].mstats = stats_init(E_STATS_TYPE_MOUSE);
    }
    adapter_device[type_index][adapter] = device_id;
    device_adapter[type_index][device_id] = adapter;
  }
  else if(adapter_device[type_index][adapter] != device_id)
  {
    static unsigned char warned[E_DEVICE_TYPE_NB][MAX_DEVICES] = {};
    if(warned[type_index][device_id] == 0)
    {
      warned[type_index][device_id] = 1;
      const char * name = NULL;
      const char * type = NULL;
      int id = -1;
      if(device_type == E_DEVICE_TYPE_KEYBOARD)
      {
        type = "keyboard";
        name = ginput_keyboard_name(device_id);
        id = ginput_keyboard_virtual_id(device_id);
      }
      else if(device_type == E_DEVICE_TYPE_MOUSE)
      {
        type = "mouse";
        name = ginput_mouse_name(device_id);
        id = ginput_mouse_virtual_id(device_id);
      }
      else if(device_type == E_DEVICE_TYPE_JOYSTICK)
      {
        type = "joystick";
        name = ginput_joystick_name(device_id);
        id = ginput_joystick_virtual_id(device_id);
      }
      gwarn(_("macros are not available for: %s %s (%d)\n"), type, name, id);
    }
  }
}

/*
 * Get the default device for a controller.
 */
int adapter_get_device(e_device_type device_type, int adapter)
{
  return adapter_device[device_type-1][adapter];
}

/*
 * Get the default controller for a device.
 */
int adapter_get_controller(e_device_type device_type, int device_id)
{
  return device_adapter[device_type-1][device_id];
}

static int adapter_forward(int adapter, unsigned char type, unsigned char* data, unsigned char length)
{
  DEBUG_PACKET(type, data, length)
  s_packet packet =
  {
    .header =
    {
      .type = type,
      .length = length
    }
  };
  memcpy(packet.value, data, length);
  if(adapter_write(adapter, &packet, sizeof(packet.header)+packet.header.length) < 0)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

int adapter_forward_control_in(int adapter, unsigned char* data, unsigned char length)
{
  return adapter_forward(adapter, BYTE_CONTROL_DATA, data, length);
}

int adapter_forward_interrupt_in(int adapter, unsigned char* data, unsigned char length)
{
  return adapter_forward(adapter, BYTE_IN_REPORT, data, length);
}

static int adapter_forward_control_out(int adapter, unsigned char* data, unsigned char length)
{
  return usb_send_control(adapter, data, length);
}

static int adapter_forward_interrupt_out(int adapter, unsigned char* data, unsigned char length)
{
  if(adapters[adapter].ctype == C_TYPE_XONE_PAD && data[0] == 0x06 && data[1] == 0x20)
  {
    adapters[adapter].status = 1;
    if (adapters[adapter].ff_core == NULL)
    {
      adapters[adapter].forward_out_reports = usb_forward_output(adapter, adapters[adapter].joystick);
    }
    else
    {
      adapters[adapter].forward_out_reports = 0;
    }
  }
  return usb_send_interrupt_out(adapter, data, length);
}

static int adapter_process_packet(int adapter, s_packet* packet)
{
  unsigned char type = packet->header.type;
  unsigned char length = packet->header.length;
  unsigned char* data = packet->value;

  int ret = 0;

  if (type != BYTE_DEBUG) // always dumped, see below
  {
    DEBUG_PACKET(type, data, length)
  }

  if(type == BYTE_CONTROL_DATA)
  {
    ret = adapter_forward_control_out(adapter, data, length);

    if(ret < 0)
    {
      gerror("failed to forward control packet to game controller\n");
    }
  }
  else if(type == BYTE_OUT_REPORT)
  {
    if (adapters[adapter].forward_out_reports)
    {
      ret = adapter_forward_interrupt_out(adapter, data, length);
      if(ret < 0)
      {
        gerror("failed to forward interrupt out packet to game controller\n");
      }
    }
    else
    {
      haptic_core_process_report(adapters[adapter].ff_core, length, data);
      haptic_core_update(adapters[adapter].ff_core);
    }
  }
  else if(type == BYTE_DEBUG)
  {
    gtime now = gtime_gettime();
    ginfo("%lu.%06lu debug packet received (size = %d bytes)\n", GTIME_SECPART(now), GTIME_USECPART(now), length);
    dump(packet->value, length);
  }
  else
  {
    gtime now = gtime_gettime();
    gwarn("%lu.%06lu unhandled packet (type=0x%02x)\n", GTIME_SECPART(now), GTIME_USECPART(now), type);
  }

  return ret;
}

static int adapter_serial_read_cb(void * user, const void * buf, int status) {

  int adapter = (intptr_t) user;

  if (status < 0) {
    // reading with no timeout
    set_done();
    return -1;
  }

  int ret = 0;
  
  if(adapters[adapter].serial.bread + status < sizeof(s_packet)) {
    memcpy((unsigned char *)&adapters[adapter].serial.packet + adapters[adapter].serial.bread, buf, status);
    adapters[adapter].serial.bread += status;
    unsigned int remaining;
    if(adapters[adapter].serial.bread < sizeof(s_header))
    {
      remaining = sizeof(s_header) - adapters[adapter].serial.bread;
    }
    else
    {
      remaining = adapters[adapter].serial.packet.header.length - (adapters[adapter].serial.bread - sizeof(s_header));
    }
    if(remaining == 0)
    {
      DEBUG_PACKET(adapters[adapter].serial.packet.header.type, adapters[adapter].serial.packet.value, adapters[adapter].serial.packet.header.length);

      if (adapters[adapter].proxy.socket == NULL)
      {
        ret = adapter_process_packet(adapter, &adapters[adapter].serial.packet);
      }
      else
      {
        ret = gudp_send(adapters[adapter].proxy.socket, &adapters[adapter].serial.packet,
                sizeof(adapters[adapter].serial.packet.header) + adapters[adapter].serial.packet.header.length,
                adapters[adapter].proxy.remote);
      }
      adapters[adapter].serial.bread = 0;
      gserial_set_read_size(adapters[adapter].serial.device, sizeof(s_header));
    }
    else
    {
      gserial_set_read_size(adapters[adapter].serial.device, remaining);
    }
  }
  else
  {
    // this is a critical error (no possible recovering)
    gerror("%s:%d %s: invalid data size (count=%u, available=%zu)\n", __FILE__, __LINE__, __func__, status, sizeof(s_packet) - adapters[adapter].serial.bread);
    ret = -1;
  }
  if (ret < 0)
  {
    set_done();
  }
  return ret;
}

static int adapter_serial_write_cb(void * user __attribute__((unused)), int transfered)
{
  return (transfered > 0) ? 0 : -1;
}

static int adapter_serial_close_cb(void * user __attribute__((unused)))
{
  set_done();
  return 0;
}

static int adapter_start_serialasync(int adapter)
{
  if(gserial_set_read_size(adapters[adapter].serial.device, sizeof(s_header)) < 0)
  {
    return -1;
  }
  GSERIAL_CALLBACKS serial_callbacks = {
          .fp_read = adapter_serial_read_cb,
          .fp_write = adapter_serial_write_cb,
          .fp_close = adapter_serial_close_cb,
          .fp_register = REGISTER_FUNCTION,
          .fp_remove = REMOVE_FUNCTION
  };
  if(gserial_register(adapters[adapter].serial.device, (void *)(intptr_t) adapter, &serial_callbacks) < 0)
  {
    return -1;
  }
  return 0;
}

static int adapter_read_timeout(int adapter, unsigned char* buf, unsigned int buflen, unsigned int len, unsigned int timeout)
{
  int ret = 0;
  if (adapters[adapter].atype == E_ADAPTER_TYPE_DIY_USB && adapters[adapter].serial.device)
  {
    ret = gserial_read_timeout(adapters[adapter].serial.device, buf, len, timeout);
  }
  else if (adapters[adapter].atype == E_ADAPTER_TYPE_PROXY && adapters[adapter].proxy.socket)
  {
    struct gudp_address address;
    ret = gudp_recv(adapters[adapter].proxy.socket, buf, buflen, timeout, &address);
  }
  return ret;
}

int adapter_read_reply(int adapter, s_packet * packet, int permissive)
{
  uint8_t type = packet->header.type;

  /*
   * The adapter may send a packet before it processes the command,
   * so it is possible to receive a packet that is not the command response.
   */
  while(1)
  {
    int ret = adapter_read_timeout(adapter, (void *) &packet->header, sizeof(packet), sizeof(packet->header), ADAPTER_TIMEOUT);
    if(ret < 0 || (size_t)ret < sizeof(packet->header))
    {
      if (!permissive)
      {
        gerror("failed to read packet header from the GIMX adapter\n");
      }
      return -1;
    }

    if (ret == sizeof(packet->header))
    {
      ret = adapter_read_timeout(adapter, (void *) &packet->value, sizeof(packet), packet->header.length, ADAPTER_TIMEOUT);
      if(ret < 0 || (size_t)ret < packet->header.length)
      {
        if (!permissive)
        {
          gerror("failed to read packet data from the GIMX adapter\n");
        }
        return -1;
      }
    }

    //Check this packet is the command response.
    if(packet->header.type == type)
    {
      return 0;
    }
  }

  return -1;
}

static int adapter_write_timeout(int adapter, s_packet* packet)
{
  int ret = 0;
  if (adapters[adapter].atype == E_ADAPTER_TYPE_DIY_USB && adapters[adapter].serial.device != NULL)
  {
    ret = gserial_write_timeout(adapters[adapter].serial.device, packet, sizeof(packet->header) + packet->header.length, ADAPTER_TIMEOUT);
  }
  else if (adapters[adapter].atype == E_ADAPTER_TYPE_PROXY && adapters[adapter].proxy.socket != NULL)
  {
    ret = gudp_send(adapters[adapter].proxy.socket, packet, sizeof(packet->header) + packet->header.length,
            adapters[adapter].proxy.remote);
  }
  return ret;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
static int adapter_send_short_command(int adapter, unsigned char type)
{
  s_packet packet =
  {
    .header =
    {
      .type = type,
      .length = BYTE_LEN_0_BYTE
    }
  };

  int ret = adapter_write_timeout(adapter, &packet);
  if(ret < 0 || (size_t)ret < sizeof(packet.header))
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  ret = adapter_read_reply(adapter, &packet, 0);

  if(ret == 0)
  {
    if(packet.header.length != BYTE_LEN_1_BYTE)
    {
      gerror("bad response from the GIMX adapter (invalid length)\n");
      return -1;
    }
    return packet.value[0];
  }

  return -1;
}

static int adapter_get_version(int adapter, int *major, int *minor)
{
  s_packet packet = { .header = { .type = BYTE_VERSION, .length = 0 }, .value = {} };

  int ret = adapter_write_timeout(adapter, &packet);
  if (ret < 0 || (size_t)ret != sizeof(packet.header))
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  ret = adapter_read_reply(adapter, &packet, 1);

  if(ret == 0 && packet.header.length == 2)
  {
    *major = packet.value[0];
    *minor = packet.value[1];
  }
  else
  {
    *major = 5;
    *minor = 8;
  }

  ginfo(_("Firmware version: %d.%d\n"), *major, *minor);

  return 0;
}

static int adapter_get_baudrate(int adapter)
{
  s_packet packet = { .header = { .type = BYTE_BAUDRATE, .length = 0 }, .value = {} };

  int ret = adapter_write_timeout(adapter, &packet);
  if (ret < 0 || (size_t)ret != sizeof(packet.header))
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  ret = adapter_read_reply(adapter, &packet, 1);

  int baudrate = (ret != -1) ? packet.value[0] * 100000 : -1;

  return baudrate;
}

static int adapter_get_baudrate_retry(int adapter, int retries)
{
  int baudrate = -1;
  int i;
  for (i = 0; i < retries && baudrate == -1 && get_done() == 0; ++i)
  {
    baudrate = adapter_get_baudrate(adapter);
  }
  return baudrate;
}

static int adapter_set_baudrate(int adapter, int baudrate)
{
  s_packet packet = { .header = { .type = BYTE_BAUDRATE, .length = 1 }, .value = { baudrate / 100000 } };

  int ret = adapter_write_timeout(adapter, &packet);
  if (ret < 0 || (size_t)ret != sizeof(packet.header) + packet.header.length)
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  return 0;
}

static int adapter_send_reset(int adapter)
{
  s_packet packet = { .header = { .type = BYTE_RESET, .length = BYTE_LEN_0_BYTE } };

  int ret = adapter_write_timeout(adapter, &packet);
  if(ret < 0 || (size_t)ret != sizeof(packet.header))
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  return 0;
}

static e_gimx_status adapter_open(int i, unsigned int baudrate)
{
  e_gimx_status ret = E_GIMX_STATUS_SUCCESS;
  s_adapter* adapter = adapter_get(i);;
  if (adapter->atype == E_ADAPTER_TYPE_DIY_USB)
  {
    if (adapter->serial.portname)
    {
      if (adapter->serial.device != NULL)
      {
          gserial_close(adapter->serial.device);
      }
      adapter->serial.device = gserial_open(adapter->serial.portname, baudrate);
      if (adapter->serial.device == NULL)
      {
        gerror(_("failed to open the GIMX adapter\n"));
        ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
      }
    }
  }
  else if (adapter->atype == E_ADAPTER_TYPE_PROXY)
  {
    if (adapter->proxy.is_client && adapter->proxy.socket == NULL)
    {
      adapter->proxy.socket = gudp_open(GUDP_MODE_CLIENT, adapter->proxy.remote);
      if(adapter->proxy.socket == NULL)
      {
        gerror(_("failed to connect to network destination: %s:%d.\n"), gudp_ip_str(adapter->proxy.remote.ip), adapter->proxy.remote.port);
        ret = E_GIMX_STATUS_GENERIC_ERROR;
      }
    }
  }
  return ret;
}

static void adapter_close(int i)
{
  s_adapter* adapter = adapter_get(i);;
  if (adapter->atype == E_ADAPTER_TYPE_DIY_USB)
  {
    if (adapter->serial.device != NULL)
    {
      gserial_close(adapter->serial.device);
    }
  }
  else if (adapter->atype == E_ADAPTER_TYPE_PROXY)
  {
    if (adapter->proxy.socket != NULL)
    {
      gudp_close(adapter->proxy.socket);
    }
  }
}

e_gimx_status adapter_detect()
{
  e_gimx_status ret = E_GIMX_STATUS_SUCCESS;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      int rtype = gpp_connect(i, adapter->serial.portname);
      if (rtype < 0)
      {
        gerror(_("no GPP detected.\n"));
        ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
      }
      else if(rtype < C_TYPE_MAX)
      {
        ginfo(_("GPP detected, controller type is: %s.\n"), controller_get_name(rtype));
        adapter->ctype = rtype;
      }
      else
      {
        gerror(_("unknown GPP controller type.\n"));
        ret = E_GIMX_STATUS_GENERIC_ERROR;
      }
    }
    else if (is_gimx_adapter(i))
    {
      ret = adapter_open(i, BAUDRATE);
      if(ret == E_GIMX_STATUS_SUCCESS)
      {
        int rtype = -1;
        int j;
        for (j = 0; j < ADAPTER_INIT_RETRIES && rtype == -1 && get_done() == 0; ++j)
        {
          rtype = adapter_send_short_command(i, BYTE_TYPE);
        }

        if(rtype >= 0 && rtype < C_TYPE_NONE)
        {
          ginfo(_("GIMX adapter detected, controller type is: %s.\n"), controller_get_name(rtype));

          if(adapter->ctype == C_TYPE_NONE)
          {
            adapter->ctype = rtype;
          }
          else if(adapter->ctype != (e_controller_type) rtype)
          {
            gerror(_("wrong controller type.\n"));
            ret = E_GIMX_STATUS_GENERIC_ERROR;
          }

          int major, minor;
          if (ret == E_GIMX_STATUS_SUCCESS)
          {
            if (adapter_get_version(i, &major, &minor) < 0)
            {
              ret = E_GIMX_STATUS_GENERIC_ERROR;
            }
          }

          if(ret == E_GIMX_STATUS_SUCCESS)
          {
            if(adapter_send_reset(i) < 0)
            {
              gerror(_("failed to reset the GIMX adapter.\n"));
              ret = E_GIMX_STATUS_GENERIC_ERROR;
            }
            else
            {
              ginfo(_("Reset sent to the GIMX adapter.\n"));
              //Leave time for the adapter to reinitialize.
              usleep(ADAPTER_RESET_TIME);
            }
          }

          if (ret == E_GIMX_STATUS_SUCCESS && !adapter->proxy.is_proxy && major >= 8)
          {
            int baudrate = adapter_get_baudrate_retry(i, ADAPTER_BAUDRATE_RETRIES);

            ginfo(_("Current baudrate: %d bps.\n"), baudrate);

            if (baudrate > 0) {

              unsigned int b;
              for (b = 0; b < sizeof(baudrates) / sizeof(*baudrates); ++b)
              {
                if (baudrate == baudrates[b]) {
                  break;
                }
                adapter_set_baudrate(i, baudrates[b]);

                ginfo(_("Trying baudrate: %d bps.\n"), baudrates[b]);

                ret = adapter_open(i, baudrates[b]);
                if(ret != E_GIMX_STATUS_SUCCESS)
                {
                  continue;
                }
                baudrate = adapter_get_baudrate_retry(i, ADAPTER_BAUDRATE_RETRIES);
                if (baudrate == baudrates[b]) {
                  break;
                }
              }

              if (b == sizeof(baudrates) / sizeof(*baudrates))
              {
                ret = E_GIMX_STATUS_GENERIC_ERROR;
              }

              if (ret == E_GIMX_STATUS_SUCCESS){
                ginfo(_("Using baudrate: %d bps.\n"), baudrate);
              }
            }
          }

          if(ret == E_GIMX_STATUS_SUCCESS && !adapter->proxy.is_proxy)
          {
            int usb_res = usb_init(i, adapter->ctype);
            if(usb_res < 0)
            {
              gerror(_("No game controller was found on USB ports.\n"));
              switch(adapter->ctype)
              {
              case C_TYPE_360_PAD:
                  ret = E_GIMX_STATUS_AUTH_MISSING_X360;
                  break;
              case C_TYPE_DS4:
              case C_TYPE_G29_PS4:
              case C_TYPE_T300RS_PS4:
                  ret = E_GIMX_STATUS_AUTH_MISSING_PS4;
                  break;
              case C_TYPE_XONE_PAD:
                  ret = E_GIMX_STATUS_AUTH_MISSING_XONE;
                  break;
              default:
                  ret = E_GIMX_STATUS_GENERIC_ERROR;
                  break;
              }
            }
          }

          if(ret == E_GIMX_STATUS_SUCCESS)
          {
            controller_init_report(adapter->ctype, &adapter->report[0].value);

            if (adapter->ctype == C_TYPE_G27_PS3)
            {
              ginfo(_("If target is a PS3, start the game with a dualshock 3, and then reassign game controllers.\n"));
            }
          }
        }
        else
        {
          ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
        }
      }
      if(adapter->ctype == C_TYPE_NONE && ret == E_GIMX_STATUS_SUCCESS)
      {
        ret = E_GIMX_STATUS_GENERIC_ERROR;
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->remote.address.ip)
      {
        adapter->remote.socket = gudp_open(GUDP_MODE_CLIENT, adapter->remote.address);
        if(adapter->remote.socket == NULL)
        {
          gerror(_("failed to connect to network destination: %s:%d\n"), gudp_ip_str(adapter->remote.address.ip), adapter->remote.address.port);
          ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
        }
        if (ret == E_GIMX_STATUS_SUCCESS)
        {
          unsigned char request[] = { E_NETWORK_PACKET_CONTROLLER };

          if (gudp_send(adapter->remote.socket, request, sizeof(request), adapter->remote.address) == -1)
          {
            ret = E_GIMX_STATUS_GENERIC_ERROR;
          }

          if (ret == E_GIMX_STATUS_SUCCESS)
          {
            s_network_packet_controller controller;
            struct gudp_address address = { 0, 0 };
            int res = gudp_recv(adapter->remote.socket, &controller, sizeof(controller), 2000, &address);
            if (res == (int) sizeof(controller))
            {
              adapter->ctype = controller.controller_type;
              ginfo(_("Remote GIMX detected, controller type is: %s.\n"), controller_get_name(adapter->ctype));
            }
            else if (res > 0)
            {
              gerror("invalid reply from remote gimx (size=%d)\n", res);
              ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
            }
            else
            {
              gerror("can't get controller type from remote gimx\n");
              ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
            }
          }
        }
      }
    }
#ifndef WIN32
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if (adapter->ctype == C_TYPE_DS4)
      {
        if(btds4_init(i, adapter->bt.index, adapter->bt.bdaddr_dst) < 0)
        {
          ret = E_GIMX_STATUS_GENERIC_ERROR;
        }
        controller_init_report(C_TYPE_DS4, &adapter->report[0].value);
      }
    }
#endif
  }
  return ret;
}

static int gpp_read_callback(void * user, const void * buf, int status)
{
  int adapter = (intptr_t) user;

  if (status < 0) {
    // reading with no timeout
    set_done();
    return -1;
  }

  haptic_core_process_report(adapters[adapter].ff_core, status, buf + 7);

  return 0;
}

static int gpp_write_callback(void * user __attribute__((unused)), int status)
{
  if (status < 0) {
    set_done();
    return -1;
  }
  return 0;
}

static int gpp_close_callback(void * user __attribute__((unused)))
{
  set_done();
  return 0;
}

int adapter_start()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);

    if (adapter->ctype != C_TYPE_NONE && gimx_params.refresh_period > 0)
    {
      adapter->inactivity.timeout = gimx_params.inactivity_timeout * 60000000L / gimx_params.refresh_period;
    }

    adapter->joystick = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, i);

    s_haptic_core_ids source = { 0 };
    if(is_gimx_adapter(i))
    {
      controller_get_ids(adapter->ctype, &source.vid, &source.pid);
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      source.vid = 0x2508;
      source.pid = 0x0001;
    }

    if (source.vid != 0x0000 && adapter->haptic_sink_joystick != -1)
    {
      adapter->ff_core = haptic_core_init(source, adapter->haptic_sink_joystick);
      const s_haptic_core_tweaks * tweaks = cfg_get_ffb_tweaks(i);
      adapter_set_haptic_tweaks(i, tweaks);
    }

    if(is_gimx_adapter(i))
    {
      if (adapter->ff_core == NULL)
      {
        adapter->forward_out_reports = usb_forward_output(i, adapter->joystick);
      }

      if (adapter->ctype == C_TYPE_XONE_PAD && !adapter->status)
      {
        adapter->forward_out_reports = 1; // force forwarding out reports until the authentication is successful.
      }

      if (!adapter->proxy.is_proxy)
      {
        if(adapter_send_short_command(i, BYTE_START) < 0)
        {
          gerror(_("failed to start the GIMX adapter.\n"));
          ret = -1;
        }
      }
      if (ret != -1 && adapter->atype == E_ADAPTER_TYPE_DIY_USB && adapter->serial.device)
      {
        if (adapter_start_serialasync(i) < 0)
        {
          gerror(_("failed to start the GIMX adapter asynchronous processing.\n"));
          ret = -1;
        }
      }
      switch(adapter->ctype)
      {
        case C_TYPE_DS4:
        case C_TYPE_T300RS_PS4:
        case C_TYPE_G29_PS4:
          ginfo(_("Press the key/button assigned to PS.\n"));
          break;
        case C_TYPE_XONE_PAD:
        case C_TYPE_360_PAD:
          ginfo(_("Press the guide button of the controller for 2 seconds.\n"));
          break;
        default:
          break;
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bt.bdaddr_dst)
      {
        if(adapter->ctype == C_TYPE_SIXAXIS)
        {
          if(sixaxis_connect(i, adapter->bt.index, adapter->bt.bdaddr_dst) < 0)
          {
            gerror(_("failed to initialize the sixaxis emulation.\n"));
            ret = -1;
          }
        }
#ifndef WIN32
        else if(adapter->ctype == C_TYPE_DS4)
        {
          if(btds4_listen(i) < 0)
          {
            gerror(_("failed to initialize the dualshock 4 emulation.\n"));
            ret = -1;
          }
        }
#endif
        else
        {
          gerror(_("unsupported bluetooth controller type.\n"));
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      GHID_CALLBACKS callbacks = {
              .fp_read = gpp_read_callback,
              .fp_write = gpp_write_callback,
              .fp_close = gpp_close_callback,
              .fp_register = REGISTER_FUNCTION,
              .fp_remove = REMOVE_FUNCTION,
      };
      ret = gpp_start_async(i, &callbacks);
      if (ret < 0)
      {
        gerror(_("failed to start the GPP asynchronous processing.\n"));
      }
    }

    if(adapter->src.ip)
    {
      adapter->src_socket = gudp_open(GUDP_MODE_SERVER, adapter->src);
      if(adapter->src_socket == NULL)
      {
        gerror(_("failed to listen on network source: %s:%d.\n"), gudp_ip_str(adapter->src.ip), adapter->src.port);
        ret = -1;
      }
      else
      {
        GUDP_CALLBACKS callbacks = {
                .fp_read = network_read_callback,
                .fp_close = adapter_close_callback,
                .fp_register = gpoll_register_fd,
                .fp_remove = gpoll_remove_fd,
        };
        if (gudp_register(adapter->src_socket, (void *)(intptr_t) i, &callbacks) < 0)
        {
          gerror(_("failed to register event source.\n"));
          ret = -1;
        }
      }
    }

    if(adapter->proxy.is_proxy)
    {
      adapter->proxy.socket = gudp_open(GUDP_MODE_SERVER, adapter->proxy.local);
      if(adapter->proxy.socket == NULL)
      {
        gerror(_("failed to open proxy source: %s:%d.\n"), gudp_ip_str(adapter->proxy.local.ip), adapter->proxy.local.port);
        ret = -1;
      }
      else
      {
        GUDP_CALLBACKS callbacks = {
                .fp_read = proxy_src_read_callback,
                .fp_close = adapter_close_callback,
                .fp_register = gpoll_register_fd,
                .fp_remove = gpoll_remove_fd,
        };
        if (gudp_register(adapter->proxy.socket, (void *)(intptr_t) i, &callbacks) < 0)
        {
          gerror(_("failed to register event source.\n"));
          ret = -1;
        }
      }
    }

    if(adapter->proxy.is_client)
    {
      GUDP_CALLBACKS callbacks = {
                .fp_read = proxy_dst_read_callback,
                .fp_close = adapter_close_callback,
                .fp_register = gpoll_register_fd,
                .fp_remove = gpoll_remove_fd,
      };
      if (gudp_register(adapter->proxy.socket, (void *)(intptr_t) i, &callbacks) < 0)
      {
        gerror(_("failed to register event source.\n"));
        ret = -1;
      }
    }

    if (ret != -1) {
        adapter->cstats = stats_init(E_STATS_TYPE_CONTROLLER);
    }
  }

  return ret;
}

int adapter_send()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  int active = 0;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);

    if(adapter->ctype == C_TYPE_NONE)
    {
      continue;
    }

    if (adapter->inactivity.timeout > 0)
    {
      ++(adapter->inactivity.counter);
      if (adapter->send_command)
      {
        adapter->inactivity.counter = 0;
      }
      if (adapter->inactivity.counter < adapter->inactivity.timeout)
      {
        active = 1;
      }
    }
    else
    {
      active = 1;
    }

    if (gimx_params.force_updates || adapter->send_command)
    {
      if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
      {
        if(adapter->remote.socket != NULL)
        {

          s_network_packet_in_report * report = &adapter->remote.report;
          report->packet_type = E_NETWORK_PACKET_IN_REPORT;
          report->nbAxes = 0;
          unsigned char i;
          for (i = 0; i < AXIS_MAX; ++i)
          {
            // send all axes if --event argument is used
            // otherwise only send changes
            if (adapter->event || adapter->remote.last_axes[i] != adapter->axis[i])
            {
              report->axes[report->nbAxes].index = (i >= abs_axis_0) ? (0x80 | (i - abs_axis_0)) : i;
              report->axes[report->nbAxes].value = gudp_htonl(adapter->axis[i]);
              ++report->nbAxes;
            }
          }
          ret = gudp_send(adapter->remote.socket, adapter->remote.buf, sizeof(* report) + report->nbAxes * sizeof(* report->axes), adapter->remote.address);
          // backup so that we can send changes only
          memcpy(adapter->remote.last_axes, adapter->axis, AXIS_MAX * sizeof(* adapter->axis));
        }
      }
      else if(is_gimx_adapter(i))
      {
        if (adapter->activation_button.index != 0)
        {
          if (adapter->axis[adapter->activation_button.index] != 0)
          {
            adapter->activation_button.pressed = 1;
          }
        }

        unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

        s_report_packet* report = adapter->report + index;

        switch(adapter->ctype)
        {
        case C_TYPE_SIXAXIS:
          ret = adapter_write(i, report, HEADER_SIZE+report->length);
          break;
        case C_TYPE_DS4:
          report->value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
          report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
          ret = adapter_write(i, report, HEADER_SIZE+report->length);
          break;
        case C_TYPE_T300RS_PS4:
        case C_TYPE_G29_PS4:
          report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
          ret = adapter_write(i, report, HEADER_SIZE+report->length);
          break;
        case C_TYPE_XONE_PAD:
          if(adapter->status)
          {
            ret = adapter_write(i, report, HEADER_SIZE+report->length);
          }
          break;
        default:
          if(adapter->ctype != C_TYPE_PS2_PAD)
          {
            ret = adapter_write(i, report, HEADER_SIZE+report->length);
          }
          else
          {
            ret = adapter_write(i, &report->value.ds2, report->length);
          }
          break;
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
      {
        if(adapter->bt.bdaddr_dst)
        {
          unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

          s_report_packet* report = adapter->report+index;

          switch(adapter->ctype)
          {
          case C_TYPE_SIXAXIS:
            ret = sixaxis_send_interrupt(i, &report->value.ds3);
            break;
#ifndef WIN32
          case C_TYPE_DS4:
            ret = btds4_send_interrupt(i, &report->value.ds4, adapter->send_command);
            break;
#endif
          default:
            break;
          }
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_GPP)
      {
        ret = gpp_send(i, adapter->ctype, adapter->axis);
      }

      if(gimx_params.status)
      {
        if (adapter->send_command)
        {
          adapter_dump_state(i);
        }
      }
      else if(gimx_params.curses)
      {
        stats_update(adapter->cstats);
        display_run(adapter_get(0)->ctype, adapter->send_command ? adapter_get(0)->axis : NULL, adapter->cstats);
      }

      adapter->send_command = 0;

      if(adapter->ctype == C_TYPE_DS4)
      {
        adapter->axis[ds4a_finger1_x] = 0;
        adapter->axis[ds4a_finger1_y] = 0;
        adapter->axis[ds4a_finger2_x] = 0;
        adapter->axis[ds4a_finger2_y] = 0;
      }
    }

    if (adapter->ff_core != NULL)
    {
      haptic_core_update(adapter->ff_core);
    }

    if (adapter->mperiod == -1 && adapter->mstats != NULL)
    {
      adapter->mperiod = stats_get_period(adapter->mstats);
      if (adapter->mperiod != -1)
      {
        ginfo(_("Mouse frequency is %dHz.\n"), 1000000 / adapter->mperiod);
        if (adapter->mperiod >= gimx_params.refresh_period)
        {
          while (gimx_params.refresh_period <= adapter->mperiod)
          {
            gimx_params.refresh_period += 1000;
          }
        }
      }
    }
  }

  if (active == 0)
  {
    ret = -1;
  }

  return ret;
}

e_gimx_status adapter_clean()
{
  e_gimx_status status = E_GIMX_STATUS_SUCCESS;
  int active = 0;
  int i;
  s_adapter* adapter;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);

    stats_clean(adapter->cstats);
    stats_clean(adapter->mstats);

    if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->remote.socket != NULL)
      {
        gudp_close(adapter->remote.socket);
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bt.bdaddr_dst)
      {
        if(adapter->ctype == C_TYPE_SIXAXIS)
        {
          sixaxis_close(i);
        }
#ifndef WIN32
        else if(adapter->ctype == C_TYPE_DS4)
        {
          btds4_close(i);
        }
#endif
      }
    }
    else if(is_gimx_adapter(i))
    {
      if (adapter->activation_button.index != 0)
      {
        if (adapter->activation_button.pressed == 0)
        {
          status = E_GIMX_STATUS_NO_ACTIVATION;
        }
      }
      usb_close(i);
      adapter_send_reset(i);
      adapter_close(i);
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      gpp_disconnect(i);
    }
    if (adapter->ff_core != NULL) {
      haptic_core_clean(adapter->ff_core);
    }
    if (adapter->ctype != C_TYPE_NONE) {
      if (adapter->inactivity.timeout > 0) {
        if (adapter->inactivity.counter < adapter->inactivity.timeout) {
          active = 1;
        }
      } else {
        active = 1;
      }
    }
  }

  if (status == E_GIMX_STATUS_SUCCESS && active == 0) {
    status = E_GIMX_STATUS_INACTIVITY_TIMEOUT;
  }

  return status;
}

void adapter_set_haptic_sink(int adapter, int joystick, int force)
{
  if (force == 1 || adapters[adapter].haptic_sink_joystick == -1)
  {
    adapters[adapter].haptic_sink_joystick = joystick;
  }
}

void adapter_set_haptic_tweaks(int adapter, const s_haptic_core_tweaks * tweaks)
{
  haptic_core_set_tweaks(adapters[adapter].ff_core, tweaks);
}
