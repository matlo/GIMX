/*
 Copyright (c) 2017 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <controller.h>

#include <gimx.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <mainloop.h>
#include <display.h>
#include <stats.h>
#include <connectors/protocol.h>
#include <connectors/udp_con.h>
#include <connectors/gpp_con.h>
#include <connectors/usb_con.h>
#include "connectors/sixaxis.h"
#ifndef WIN32
#include "connectors/btds4.h"
#endif
#include <gimxcontroller/include/controller.h>
#include <gimxpoll/include/gpoll.h>

#include <haptic/haptic_core.h>

#define BAUDRATE 500000 //bps
#define SERIAL_TIMEOUT 1000 //millisecond
/*
 * The adapter restarts about 15ms after receiving the reset command.
 * This time is doubled so as to include the reset command transfer duration.
 */
#define ADAPTER_RESET_TIME 30000 //microseconds
/*
 * The number of times the adapter is queried for its type, before it is assumed as unreachable.
 */
#define ADAPTER_INIT_RETRIES 10

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
    adapters[i].remote.fd = -1;
    adapters[i].src_fd = -1;
    adapters[i].haptic_sink_joystick = -1;
    adapters[i].serial.bread = 0;
    for(j = 0; j < MAX_REPORTS; ++j)
    {
      adapters[i].report[j].type = BYTE_IN_REPORT;
    }
    adapters[i].status = 0;
    adapters[i].joystick = -1;
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
  struct timeval tv;
  gettimeofday(&tv, NULL);

  int* axis = adapters[adapter].axis;

  gstatus("%d %ld.%06ld", adapter, tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          gstatus(", %s (%d)", controller_get_axis_name(adapters[adapter].ctype, i), axis[i]);
  }

  gstatus("\n");
}

/*
 * Read a packet from a remote GIMX client.
 * The packet can be:
 * - a "get controller type" request
 * - a report to be sent.
 * Note that the socket operations should not block.
 */
static int network_read_callback(void * user)
{
  int adapter = (intptr_t) user;

  static unsigned char buf[sizeof(s_network_packet_in_report) + AXIS_MAX * sizeof(* ((s_network_packet_in_report *) NULL)->axes)];
  int nread = 0;
  struct sockaddr_in sa;
  socklen_t salen = sizeof(sa);
  // retrieve the packet and the address of the client
  if((nread = udp_recvfrom(adapters[adapter].src_fd, buf, sizeof(buf), (struct sockaddr*) &sa, &salen)) <= 0)
  {
    return 0;
  }
  if(nread < 1)
  {
    gwarn("invalid packet size: %d\n", nread);
    return 0;
  }
  switch(buf[0])
  {
  case E_NETWORK_PACKET_CONTROLLER:
    {
      // send the answer
      s_network_packet_controller ctype =
      {
        .packet_type = E_NETWORK_PACKET_CONTROLLER,
        .controller_type = adapters[adapter].ctype
      };
      if (udp_sendto(adapters[adapter].src_fd, (void *)&ctype, sizeof(ctype), (struct sockaddr*) &sa, salen) < 0)
      {
        gwarn("%s: can't send controller type\n", __func__);
        return 0;
      }
    }
    break;
  case E_NETWORK_PACKET_IN_REPORT:
    {
      s_network_packet_in_report * report = (s_network_packet_in_report *) buf;
      if((unsigned int) nread != sizeof(* report) + report->nbAxes * sizeof(* report->axes))
      {
        gwarn("%s: wrong packet size: %u %zu\n", __func__, nread, sizeof(* report) + report->nbAxes * sizeof(* report->axes));
        return 0;
      }
      // store the report (no answer)
      unsigned char i;
      for (i = 0; i < report->nbAxes; ++i)
      {
        unsigned char offset = ((report->axes[i].index & 0x80) ? abs_axis_0 : 0) + (report->axes[i].index & 0x7f);
        if (offset < AXIS_MAX)
        {
          adapters[adapter].axis[offset] = ntohl(report->axes[i].value);
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
    ginfo("type: 0x%02x\n", TYPE); \
    dump(DATA, LENGTH); \
  }

static int adapter_forward(int adapter, unsigned char type, unsigned char* data, unsigned char length)
{
  if(adapters[adapter].serial.device != NULL)
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
    if(gserial_write(adapters[adapter].serial.device, &packet, sizeof(packet.header)+packet.header.length) < 0)
    {
      return -1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    gerror("no serial port opened for adapter %d\n", adapter);
    return -1;
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
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ginfo("%ld.%06ld debug packet received (size = %d bytes)\n", tv.tv_sec, tv.tv_usec, length);
    dump(packet->value, length);
  }
  else
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    gwarn("%ld.%06ld unhandled packet (type=0x%02x)\n", tv.tv_sec, tv.tv_usec, type);
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
      ret = adapter_process_packet(adapter, &adapters[adapter].serial.packet);
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

  int ret = gserial_write_timeout(adapters[adapter].serial.device, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
  if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
  {
    gerror("failed to send data to the GIMX adapter\n");
    return -1;
  }

  /*
   * The adapter may send a packet before it processes the command,
   * so it is possible to receive a packet that is not the command response.
   */
  while(1)
  {
    ret = gserial_read_timeout(adapters[adapter].serial.device, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
    {
      gerror("failed to read packet header from the GIMX adapter\n");
      return -1;
    }

    ret = gserial_read_timeout(adapters[adapter].serial.device, &packet.value, packet.header.length, SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < packet.header.length)
    {
      gerror("failed to read packet data from the GIMX adapter\n");
      return -1;
    }

    //Check this packet is the command response.
    if(packet.header.type == type)
    {
      if(packet.header.length != BYTE_LEN_1_BYTE)
      {
        gerror("bad response from the GIMX adapter (invalid length)\n");
        return -1;
      }

      return packet.value[0];
    }
  }

  return 0;
}

static int adapter_send_reset(int adapter)
{
  s_header header =
  {
    .type = BYTE_RESET,
    .length = BYTE_LEN_0_BYTE
  };

  if(gserial_write_timeout(adapters[adapter].serial.device, &header, sizeof(header), SERIAL_TIMEOUT) != sizeof(header))
  {
    return -1;
  }

  return 0;
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
    else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serial.portname)
      {
        adapter->serial.device = gserial_open(adapter->serial.portname, BAUDRATE);
        if(adapter->serial.device == NULL)
        {
          gerror(_("failed to open the GIMX adapter\n"));
          ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
        }
        else
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

            int status = adapter_send_short_command(i, BYTE_STATUS);

            if(status < 0)
            {
              gerror(_("failed to get the GIMX adapter status.\n"));
              ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
            }

            if(ret != -1)
            {
              switch(adapter->ctype)
              {
                case C_TYPE_DS4:
                case C_TYPE_T300RS_PS4:
                case C_TYPE_G29_PS4:
                case C_TYPE_G27_PS3:
                case C_TYPE_XONE_PAD:
                case C_TYPE_360_PAD:
                  if(status == BYTE_STATUS_STARTED)
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
                  break;
                default:
                  break;
              }
            }

            if(ret == E_GIMX_STATUS_SUCCESS)
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
                ginfo(_("Start the game with a dualshock 3, and then reassign game controllers.\n"));
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
    }
    else if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->remote.ip)
      {
        adapter->remote.fd = udp_connect(adapter->remote.ip, adapter->remote.port, (int *)&adapter->ctype);
        if(adapter->remote.fd < 0)
        {
          struct in_addr addr = { .s_addr = adapter->remote.ip };
          gerror(_("failed to connect to network destination: %s:%d.\n"), inet_ntoa(addr), adapter->remote.port);
          ret = E_GIMX_STATUS_ADAPTER_NOT_DETECTED;
        }
        else
        {
          ginfo(_("Remote GIMX detected, controller type is: %s.\n"), controller_get_name(adapter->ctype));
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
    if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
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

    if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serial.device != NULL)
      {
        if (adapter->ff_core == NULL)
        {
          adapter->forward_out_reports = usb_forward_output(i, adapter->joystick);
        }

        switch(adapter->ctype)
        {
          case C_TYPE_XONE_PAD:
            if(!adapter->status)
            {
                adapter->forward_out_reports = 1; // force forwarding out reports until the authentication is successful.
            }
            break;
          default:
            break;
        }

        if(adapter_send_short_command(i, BYTE_START) < 0)
        {
          gerror(_("failed to start the GIMX adapter.\n"));
          ret = -1;
        }
        else if(adapter_start_serialasync(i) < 0)
        {
          gerror(_("failed to start the GIMX adapter asynchronous processing.\n"));
          ret = -1;
        }
        adapter->activation_button.index = controller_get_activation_button(adapter->ctype);
        if (adapter->activation_button.index != 0)
        {
          ginfo(_("Press the %s button to activate the controller.\n"), controller_get_axis_name(adapter->ctype, adapter->activation_button.index));
        }
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

    if(adapter->src_ip)
    {
      adapter->src_fd = udp_listen(adapter->src_ip, adapter->src_port);
      if(adapter->src_fd < 0)
      {
        struct in_addr addr = { .s_addr = adapter->src_ip };
        gerror(_("failed to listen on network source: %s:%d.\n"), inet_ntoa(addr), adapter->src_port);
        ret = -1;
      }
      else
      {
        GPOLL_CALLBACKS callbacks = {
                .fp_read = network_read_callback,
                .fp_write = NULL,
                .fp_close = adapter_close_callback,
        };
        gpoll_register_fd(adapter->src_fd, (void *)(intptr_t) i, &callbacks);
      }
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
        if(adapter->remote.fd >= 0)
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
              report->axes[report->nbAxes].value = htonl(adapter->axis[i]);
              ++report->nbAxes;
            }
          }
          ret = udp_send(adapter->remote.fd, adapter->remote.buf, sizeof(* report) + report->nbAxes * sizeof(* report->axes));
          // backup so that we can send changes only
          memcpy(adapter->remote.last_axes, adapter->axis, AXIS_MAX * sizeof(* adapter->axis));
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
      {
        if(adapter->serial.device != NULL)
        {
          if (adapter->activation_button.index != 0)
          {
            if (adapter->axis[adapter->activation_button.index] != 0)
            {
              adapter->activation_button.pressed = 1;
            }
          }

          unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

          s_report_packet* report = adapter->report+index;

          switch(adapter->ctype)
          {
          case C_TYPE_SIXAXIS:
            ret = gserial_write(adapter->serial.device, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_DS4:
            report->value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = gserial_write(adapter->serial.device, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_T300RS_PS4:
          case C_TYPE_G29_PS4:
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = gserial_write(adapter->serial.device, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_XONE_PAD:
            if(adapter->status)
            {
              ret = gserial_write(adapter->serial.device, report, HEADER_SIZE+report->length);
            }
            break;
          default:
            if(adapter->ctype != C_TYPE_PS2_PAD)
            {
              ret = gserial_write(adapter->serial.device, report, HEADER_SIZE+report->length);
            }
            else
            {
              ret = gserial_write(adapter->serial.device, &report->value.ds2, report->length);
            }
            break;
          }
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
#ifdef WIN32
          //There is no setlinebuf(stdout) in windows.
          fflush(stdout);
#endif
        }
      }
      else if(gimx_params.curses)
      {
        stats_update(i);
        display_run(adapter_get(0)->ctype, adapter->send_command ? adapter_get(0)->axis : NULL);
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
    if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->remote.fd >= 0)
      {
        gpoll_remove_fd(adapter->src_fd);
        udp_close(adapter->remote.fd);
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
    else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serial.device != NULL)
      {
        if (adapter->activation_button.index != 0)
        {
          if (adapter->activation_button.pressed == 0)
          {
            status = E_GIMX_STATUS_NO_ACTIVATION;
          }
        }
        switch(adapter->ctype)
        {
          case C_TYPE_360_PAD:
          case C_TYPE_XONE_PAD:
          case C_TYPE_DS4:
          case C_TYPE_T300RS_PS4:
          case C_TYPE_G29_PS4:
          case C_TYPE_G27_PS3:
            usb_close(i);
            adapter_send_reset(i);
            break;
          default:
            break;
        }
        gserial_close(adapter->serial.device);
      }
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
