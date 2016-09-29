/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <adapter.h>
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
#include <controller2.h>
#include <gpoll.h>

#include <haptic/ff_lg.h>
#include <haptic/ff_conv.h>
#include <ghid.h>

#define BAUDRATE 500000 //bps
#define SERIAL_TIMEOUT 1000 //millisecond
/*
 * The adapter restarts about 15ms after receiving the reset command.
 * This time is doubled so as to include the reset command transfer duration.
 */
#define ADAPTER_RESET_TIME 30000 //microseconds

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static s_adapter adapters[MAX_CONTROLLERS] = {};

static struct
{
  unsigned char has_rumble;
  unsigned char weak;
  unsigned char strong;
} rumble_props[C_TYPE_MAX] =
{
  [C_TYPE_DS4]     = { .has_rumble = 1, .weak = 4, .strong = 5 },
  [C_TYPE_360_PAD] = { .has_rumble = 1, .weak = 4, .strong = 3 },
  [C_TYPE_SIXAXIS] = { .has_rumble = 1, .weak = 3, .strong = 5 },
  [C_TYPE_XONE_PAD] = { .has_rumble = 1, .weak = 7, .strong = 6 },
};

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
    adapters[i].dst_fd = -1;
    adapters[i].src_fd = -1;
    adapters[i].serialdevice = -1;
    adapters[i].joystick.hid.id = -1;
    adapters[i].bread = 0;
    for(j = 0; j < MAX_REPORTS; ++j)
    {
      adapters[i].report[j].type = BYTE_IN_REPORT;
    }
    adapters[i].status = 0;
    adapters[i].joystick.id = -1;
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
    if(adapters[i].portname && !strcmp(adapters[i].portname, portname))
    {
      return -1;
    }
  }
  adapters[adapter].portname = portname;
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

  printf("%d %ld.%06ld", adapter, tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", controller_get_axis_name(adapters[adapter].ctype, i), axis[i]);
  }

  printf("\n");
}

/*
 * Read a packet from a remote GIMX client.
 * The packet can be:
 * - a "get controller type" request
 * - a report to be sent.
 * Note that the socket operations should not block.
 */
static int network_read_callback(int adapter)
{
  static unsigned char buf[256+2];
  int nread = 0;
  struct sockaddr_in sa;
  socklen_t salen = sizeof(sa);
  // retrieve the packet and the address of the client
  if((nread = udp_recvfrom(adapters[adapter].src_fd, buf, sizeof(buf), (struct sockaddr*) &sa, &salen)) <= 0)
  {
    return 0;
  }
  if(nread < 2)
  {
    fprintf(stderr, "invalid packet size: %d\n", nread);
    return 0;
  }
  switch(buf[0])
  {
  case BYTE_TYPE:
    {
      // send the answer
      unsigned char answer[3] = {BYTE_TYPE, BYTE_LEN_1_BYTE, adapters[adapter].ctype};
      if (udp_sendto(adapters[adapter].src_fd, answer, sizeof(answer), (struct sockaddr*) &sa, salen) < 0)
      {
        fprintf(stderr, "adapter_network_read: can't send controller type\n");
        return 0;
      }
    }
    break;
  case BYTE_IN_REPORT:
    if(buf[1] != sizeof(adapters->axis))
    {
      fprintf(stderr, "adapter_network_read: wrong packet size\n");
      return 0;
    }
    // store the report (no answer)
    memcpy(adapters[adapter].axis, buf+2, sizeof(adapters->axis));
    adapters[adapter].send_command = 1;
    break;
  }
  // require a report to be sent immediately, except for a Sixaxis controller working over bluetooth
  if(adapters[adapter].ctype == C_TYPE_SIXAXIS && adapters[adapter].atype == E_ADAPTER_TYPE_BLUETOOTH)
  {
    return 0;
  }
  return 1;
}

int adapter_close_callback(int adapter __attribute__((unused)))
{
  set_done();

  return 0;
}

/*
 * Set the default device for a controller.
 */
void adapter_set_device(int adapter, e_device_type device_type, int device_id)
{
  int type_index = device_type-1;

  if(adapter < 0 || adapter >= MAX_CONTROLLERS
  || type_index < 0 || type_index >= E_DEVICE_TYPE_NB
  || device_id < 0 || type_index > MAX_DEVICES)
  {
    fprintf(stderr, "set_controller_device error\n");
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
      gprintf(_("macros are not available for: "));
      if(device_type == E_DEVICE_TYPE_KEYBOARD)
      {
        gprintf(_("keyboard %s (%d)\n"), ginput_keyboard_name(device_id), ginput_keyboard_virtual_id(device_id));
      }
      else if(device_type == E_DEVICE_TYPE_MOUSE)
      {
        gprintf(_("mouse %s (%d)\n"), ginput_mouse_name(device_id), ginput_mouse_virtual_id(device_id));
      }
      else if(device_type == E_DEVICE_TYPE_JOYSTICK)
      {
        gprintf(_("joystick %s (%d)\n"), ginput_joystick_name(device_id), ginput_joystick_virtual_id(device_id));
      }
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

static int debug = 0;

static void dump(unsigned char * packet, unsigned char length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    if(i && !(i%8))
    {
      gprintf("\n");
    }
    gprintf("0x%02x ", packet[i]);
  }
  gprintf("\n");
}

#define DEBUG_PACKET(PACKET, LENGTH) \
  if(debug) \
  { \
    gprintf("%s\n", __func__); \
    dump(data, length); \
  }

static int adapter_forward(int adapter, unsigned char type, unsigned char* data, unsigned char length)
{
  if(adapters[adapter].serialdevice >= 0)
  {
    DEBUG_PACKET(data, length)
    s_packet packet =
    {
      .header =
      {
        .type = type,
        .length = length
      }
    };
    memcpy(packet.value, data, length);
    if(gserial_write(adapters[adapter].serialdevice, &packet, sizeof(packet.header)+packet.header.length) < 0)
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
    fprintf(stderr, "no serial port opened for adapter %d\n", adapter);
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
  DEBUG_PACKET(data, length)
  return usb_send_control(adapter, data, length);
}

static int adapter_forward_interrupt_out(int adapter, unsigned char* data, unsigned char length)
{
  DEBUG_PACKET(data, length)
  if(adapters[adapter].ctype == C_TYPE_XONE_PAD && data[0] == 0x06 && data[1] == 0x20)
  {
    adapters[adapter].status = 1;
    if (adapters[adapter].joystick.id >= 0 && adapters[adapter].joystick.id != usb_get_joystick(adapter))
    {
      adapters[adapter].forward_out_reports = 0;
    }
  }
  return usb_send_interrupt_out(adapter, data, length);
}

static void adapter_send_next_hid_report(int adapter)
{
  if(!adapters[adapter].joystick.hid.write_pending)
  {
    s_ff_lg_report report;
    int ret = ff_lg_get_report(adapter, &report);
    if(ret == 1)
    {
      if(ghid_write(adapters[adapter].joystick.hid.id, report.data, sizeof(report.data)) == 0)
      {
        adapters->joystick.hid.write_pending = 1;
      }
    }
  }
}

static void adapter_send_next_ffb_update(int adapter, unsigned char data[FF_LG_OUTPUT_REPORT_SIZE]) {

    GE_Event events[FF_LG_FSLOTS_NB] = {};
    int events_nb = ff_conv(adapter, data, events);
    int i;
    for (i = 0; i < events_nb; ++i) {
        events[i].which = adapters[adapter].joystick.id;
        ginput_joystick_set_haptic(events + i);
    }
}

static int adapter_process_packet(int adapter, s_packet* packet)
{
  unsigned char type = packet->header.type;
  unsigned char length = packet->header.length;
  unsigned char* data = packet->value;

  int ret = 0;

  if(type == BYTE_CONTROL_DATA)
  {
    ret = adapter_forward_control_out(adapter, data, length);

    if(ret < 0)
    {
      fprintf(stderr, "adapter_forward_data_out failed\n");
    }
  }
  else if(type == BYTE_OUT_REPORT)
  {
    if (adapters[adapter].forward_out_reports)
    {
      ret = adapter_forward_interrupt_out(adapter, data, length);
      if(ret < 0)
      {
        fprintf(stderr, "adapter_forward_interrupt_out failed\n");
      }
    }
    else if(rumble_props[adapters[adapter].ctype].has_rumble && adapters[adapter].joystick.has_rumble)
    {
      GE_Event event =
      {
        .jrumble =
        {
          .type = GE_JOYRUMBLE,
          .which = adapters[adapter].joystick.id,
          .weak = data[rumble_props[adapters[adapter].ctype].weak] << 8,
          .strong = data[rumble_props[adapters[adapter].ctype].strong] << 8
        }
      };
      ginput_queue_push(&event);
    }
    unsigned char * logitech_ffb_report = NULL;
    switch(adapters[adapter].ctype)
    {
      case C_TYPE_T300RS_PS4:
        //TODO MLA: decode and process T300RS FFB reports
        break;
      case C_TYPE_GTF_PS2:
      case C_TYPE_DF_PS2:
      case C_TYPE_DFP_PS2:
      case C_TYPE_G27_PS3:
        logitech_ffb_report = data;
        break;
      case C_TYPE_G29_PS4:
        if(data[0] == 0x30)
        {
          logitech_ffb_report = data + 1;
        }
        break;
      default:
        break;
    }
    if (logitech_ffb_report != NULL)
    {
      if (adapters[adapter].joystick.hid.id >= 0)
      {
        ff_lg_process_report(adapter, logitech_ffb_report);
        adapter_send_next_hid_report(adapter);
      }
      else if (adapters[adapter].joystick.id >= 0 && adapters[adapter].joystick.has_ffb)
      {
        adapter_send_next_ffb_update(adapter, logitech_ffb_report);
      }
    }
  }
  else if(type == BYTE_DEBUG)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    gprintf("%ld.%06ld debug packet received (size = %d bytes)\n", tv.tv_sec, tv.tv_usec, length);
    dump(packet->value, length);
  }
  else
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    fprintf(stderr, "%ld.%06ld ", tv.tv_sec, tv.tv_usec);
    fprintf(stderr, "unhandled packet (type=0x%02x)\n", type);
  }

  return ret;
}

static int adapter_hid_write_cb(int adapter, int status)
{
  adapters[adapter].joystick.hid.write_pending = 0;
  if(status != -1) {
    ff_lg_ack(adapter);
  }
  adapter_send_next_hid_report(adapter);
  return 0;
}

static int adapter_hid_close_cb(int adapter)
{
  adapters[adapter].joystick.hid.id = -1;
  return 1;
}

#ifndef WIN32
void adapter_set_hid(int adapter, int hid)
{
  if(adapter < 0 || adapter >= MAX_CONTROLLERS)
  {
    fprintf(stderr, "%s: invalid controller\n", __func__);
    return;
  }

  if(adapters[adapter].joystick.hid.id < 0)
  {
      const s_hid_info * info = ghid_get_hid_info(hid);
      if (info != NULL && ff_lg_is_logitech_wheel(info->vendor_id, info->product_id))
      {
        adapters[adapter].joystick.hid.id = hid;
        adapters[adapter].joystick.usb_ids.vendor = info->vendor_id;
        adapters[adapter].joystick.usb_ids.product = info->product_id;
        ginput_joystick_set_hid_callbacks(hid, adapter, adapter_hid_write_cb, adapter_hid_close_cb);
      }
  }
}
#else
void adapter_set_usb_ids(int adapter, unsigned short vendor, unsigned short product)
{
  if(adapter < 0 || adapter >= MAX_CONTROLLERS)
  {
    fprintf(stderr, "%s: invalid adapter\n", __func__);
    return;
  }

  if(adapter_device[E_DEVICE_TYPE_JOYSTICK - 1][adapter] >= 0)
  {
    adapters[adapter].joystick.usb_ids.vendor = vendor;
    adapters[adapter].joystick.usb_ids.product = product;
  }
}

static int start_hid(int adapter)
{
  adapters[adapter].joystick.hid.id = ghid_open_ids(adapters[adapter].joystick.usb_ids.vendor, adapters[adapter].joystick.usb_ids.product);
  if(adapters[adapter].joystick.hid.id >= 0)
  {
    if(ghid_register(adapters[adapter].joystick.hid.id, adapter, NULL, adapter_hid_write_cb, adapter_hid_close_cb, REGISTER_FUNCTION) < 0)
    {
      return -1;
    }
  }

  return 0;
}
#endif

static int adapter_serial_read_cb(int adapter, const void * buf, int status) {

  if (status < 0) {
    // reading with no timeout
    set_done();
    return -1;
  }

  int ret = 0;
  
  if(adapters[adapter].bread + status < sizeof(s_packet)) {
    memcpy((unsigned char *)&adapters[adapter].packet + adapters[adapter].bread, buf, status);
    adapters[adapter].bread += status;
    unsigned int remaining;
    if(adapters[adapter].bread < sizeof(s_header))
    {
      remaining = sizeof(s_header) - adapters[adapter].bread;
    }
    else
    {
      remaining = adapters[adapter].packet.header.length - (adapters[adapter].bread - sizeof(s_header));
    }
    if(remaining == 0)
    {
      ret = adapter_process_packet(adapter, &adapters[adapter].packet);
      adapters[adapter].bread = 0;
      gserial_set_read_size(adapters[adapter].serialdevice, sizeof(s_header));
    }
    else
    {
      gserial_set_read_size(adapters[adapter].serialdevice, remaining);
    }
  }
  else
  {
    // this is a critical error (no possible recovering)
    fprintf(stderr, "%s:%d %s: invalid data size (count=%u, available=%zu)\n", __FILE__, __LINE__, __func__, status, sizeof(s_packet) - adapters[adapter].bread);
    return -1;
  }
  return ret;
}

static int adapter_serial_write_cb(int adapter __attribute__((unused)), int transfered)
{
  return (transfered > 0) ? 0 : -1;
}

static int adapter_serial_close_cb(int adapter __attribute__((unused)))
{
  set_done();
  return 0;
}

static int adapter_start_serialasync(int adapter)
{
  if(gserial_set_read_size(adapters[adapter].serialdevice, sizeof(s_header)) < 0)
  {
    return -1;
  }
  if(gserial_register(adapters[adapter].serialdevice, adapter, adapter_serial_read_cb, adapter_serial_write_cb, adapter_serial_close_cb, REGISTER_FUNCTION) < 0)
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

  int ret = gserial_write_timeout(adapters[adapter].serialdevice, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
  if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
  {
    fprintf(stderr, "serial_send\n");
    return -1;
  }

  /*
   * The adapter may send a packet before it processes the command,
   * so it is possible to receive a packet that is not the command response.
   */
  while(1)
  {
    ret = gserial_read_timeout(adapters[adapter].serialdevice, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
    {
      fprintf(stderr, "can't read packet header\n");
      return -1;
    }

    ret = gserial_read_timeout(adapters[adapter].serialdevice, &packet.value, packet.header.length, SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < packet.header.length)
    {
      fprintf(stderr, "can't read packet data\n");
      return -1;
    }

    //Check this packet is the command response.
    if(packet.header.type == type)
    {
      if(packet.header.length != BYTE_LEN_1_BYTE)
      {
        fprintf(stderr, "bad response\n");
        return -1;
      }

      return packet.value[0];
    }
  }

  return 0;
}

static int adapter_send_reset(int device)
{
  s_header header =
  {
    .type = BYTE_RESET,
    .length = BYTE_LEN_0_BYTE
  };

  if(gserial_write_timeout(device, &header, sizeof(header), SERIAL_TIMEOUT) != sizeof(header))
  {
    return -1;
  }

  return 0;
}

int adapter_detect()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      int rtype = gpp_connect(i, adapter->portname);
      if (rtype < 0)
      {
        fprintf(stderr, _("No GPP detected.\n"));
        ret = -1;
      }
      else if(rtype < C_TYPE_MAX)
      {
        printf(_("GPP detected, controller type is: %s.\n"), controller_get_name(rtype));
        adapter->ctype = rtype;
      }
      else
      {
        fprintf(stderr, _("Unknown GPP controller type.\n"));
        ret = -1;
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->portname)
      {
        adapter->serialdevice = gserial_open(adapter->portname, BAUDRATE);
        if(adapter->serialdevice < 0)
        {
          fprintf(stderr, _("Check the wiring (maybe you swapped Rx and Tx?).\n"));
          ret = -1;
        }
        else
        {
          int rtype = adapter_send_short_command(i, BYTE_TYPE);

          if(rtype >= 0)
          {
            printf(_("USB adapter detected, controller type is: %s.\n"), controller_get_name(rtype));

            if(adapter->ctype == C_TYPE_NONE)
            {
              adapter->ctype = rtype;
            }
            else if(adapter->ctype != (e_controller_type) rtype)
            {
              fprintf(stderr, _("Wrong controller type.\n"));
              ret = -1;
            }

            int status = adapter_send_short_command(i, BYTE_STATUS);

            if(status < 0)
            {
              fprintf(stderr, _("Can't get adapter status.\n"));
              ret = -1;
            }

            if(ret != -1)
            {
              switch(adapter->ctype)
              {
                case C_TYPE_DS4:
                case C_TYPE_T300RS_PS4:
                case C_TYPE_G29_PS4:
                case C_TYPE_G27_PS3:
                  if(status == BYTE_STATUS_STARTED)
                  {
                    if(adapter_send_reset(i) < 0)
                    {
                      fprintf(stderr, _("Can't reset the adapter.\n"));
                      ret = -1;
                    }
                    else
                    {
                      printf(_("Reset sent to the adapter.\n"));
                      //Leave time for the adapter to reinitialize.
                      usleep(ADAPTER_RESET_TIME);
                    }
                  }
                  break;
                case C_TYPE_XONE_PAD:
                case C_TYPE_360_PAD:
                  if(status == BYTE_STATUS_SPOOFED)
                  {
                    adapter->status = 1;
                  }
                  else
                  {
                    if(adapter_send_reset(i) < 0)
                    {
                      fprintf(stderr, _("Can't reset the adapter.\n"));
                      ret = -1;
                    }
                    else
                    {
                      printf(_("Reset sent to the adapter.\n"));
                      //Leave time for the adapter to reinitialize.
                      usleep(ADAPTER_RESET_TIME);
                    }
                  }
                  break;
                case C_TYPE_SIXAXIS:
                  //TODO MLA: fix the EMUPS3 firmware and remove this!
                  gimx_params.force_updates = 0;
                  printf(_("Disable force updates to work-around #335.\n"));
                  break;
                default:
                  break;
              }
            }

            if(ret != -1)
            {
              int usb_res = usb_init(i, adapter->ctype);
              if(usb_res < 0)
              {
                if((adapter->ctype != C_TYPE_360_PAD
                    && adapter->ctype != C_TYPE_XONE_PAD)
                    || status != BYTE_STATUS_SPOOFED)
                {
                  fprintf(stderr, _("No controller was found on USB buses.\n"));
                  ret = -1;
                }
              }
            }

            if(ret != -1)
            {
              controller_init_report(adapter->ctype, &adapter->report[0].value);
            }
          }
        }
        if(adapter->ctype == C_TYPE_NONE)
        {
          ret = -1;
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->dst_ip)
      {
        adapter->dst_fd = udp_connect(adapter->dst_ip, adapter->dst_port, (int *)&adapter->ctype);
        if(adapter->dst_fd < 0)
        {
          struct in_addr addr = { .s_addr = adapter->dst_ip };
          fprintf(stderr, _("Can't connect to: %s:%d.\n"), inet_ntoa(addr), adapter->dst_port);
          ret = -1;
        }
        else
        {
          printf(_("Remote GIMX detected, controller type is: %s.\n"), controller_get_name(adapter->ctype));
        }
      }
    }
#ifndef WIN32
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if (adapter->ctype == C_TYPE_DS4)
      {
        if(btds4_init(i, adapter->dongle_index, adapter->bdaddr_dst) < 0)
        {
          ret = -1;
        }
        controller_init_report(C_TYPE_DS4, &adapter->report[0].value);
      }
    }
#endif
  }
  return ret;
}

static int gpp_read_callback(int adapter, const void * buf, int status)
{
  if (status < 0) {
    // reading with no timeout
    set_done();
    return -1;
  }

  s_adapter * padapter = adapter_get(adapter);
  if (padapter == NULL) {
    return -1;
  }

  GCAPI_REPORT * report = (GCAPI_REPORT *)(buf + 7);

  if(padapter->joystick.id >= 0 && padapter->joystick.has_rumble)
  {
    GE_Event event =
    {
      .jrumble =
      {
        .type = GE_JOYRUMBLE,
        .which = padapter->joystick.id,
        .weak = report->rumble[0] << 8,
        .strong = report->rumble[1] << 8
      }
    };
    ginput_queue_push(&event);
  }

  return 0;
}

static int gpp_write_callback(int adapter __attribute__((unused)), int status)
{
  if (status < 0) {
    set_done();
    return -1;
  }
  return 0;
}

static int gpp_close_callback(int adapter __attribute__((unused)))
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

    // get the default joystick and its haptic properties
    adapter->joystick.id = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, i);
    if (adapter->joystick.id >= 0)
    {
      int haptic = ginput_joystick_get_haptic(adapter->joystick.id);
      adapter->joystick.has_rumble = haptic & GE_HAPTIC_RUMBLE;
      adapter->joystick.has_ffb = haptic & (GE_HAPTIC_CONSTANT | GE_HAPTIC_SPRING | GE_HAPTIC_DAMPER);
      printf("%d has_ffb: %02x\n", adapter->joystick.id, adapter->joystick.has_ffb);
    }

    if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serialdevice >= 0)
      {
        if (adapter->joystick.id >= 0)
        {
          if (usb_get_joystick(i) == adapter->joystick.id)
          {
              adapter->forward_out_reports = 1;
          }
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

        unsigned short vid = 0;
        unsigned short pid = 0;
        controller_get_ids(adapter->ctype, &vid, &pid);
        if (ff_lg_is_logitech_wheel(vid, pid))
        {
          // emulated controller is a Logitech wheel with FFB support
          if (ff_lg_is_logitech_wheel(adapter->joystick.usb_ids.vendor, adapter[i].joystick.usb_ids.product))
          {
            // default joystick is a Logitech wheel with FFB support
            ff_lg_init(i, pid, adapter->joystick.usb_ids.product);
#ifdef WIN32
            start_hid(i);
#endif
          }
          else if (adapter->joystick.has_ffb)
          {
            // default joystick is a non-Logitech wheel with FFB support
            ff_conv_init(i, pid);
          }
        }
        if(adapter_send_short_command(i, BYTE_START) < 0)
        {
          fprintf(stderr, _("Can't start the adapter.\n"));
          ret = -1;
        }
        else if(adapter_start_serialasync(i) < 0)
        {
          fprintf(stderr, _("Can't start the serial asynchronous processing.\n"));
          ret = -1;
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bdaddr_dst)
      {
        if(adapter->ctype == C_TYPE_SIXAXIS)
        {
          if(sixaxis_connect(i, adapter->dongle_index, adapter->bdaddr_dst) < 0)
          {
            fprintf(stderr, _("Can't initialize sixaxis.\n"));
            ret = -1;
          }
        }
#ifndef WIN32
        else if(adapter->ctype == C_TYPE_DS4)
        {
          if(btds4_listen(i) < 0)
          {
            ret = -1;
          }
        }
#endif
        else
        {
          fprintf(stderr, _("Wrong controller type.\n"));
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      ret = gpp_start_async(i, gpp_read_callback, gpp_write_callback, gpp_close_callback, REGISTER_FUNCTION);
    }

    if(adapter->src_ip)
    {
      adapter->src_fd = udp_listen(adapter->src_ip, adapter->src_port);
      if(adapter->src_fd < 0)
      {
        struct in_addr addr = { .s_addr = adapter->src_ip };
        fprintf(stderr, _("Can't listen on: %s:%d.\n"), inet_ntoa(addr), adapter->src_port);
        ret = -1;
      }
      else
      {
        gpoll_register_fd(adapter->src_fd, i, network_read_callback, NULL, adapter_close_callback);
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

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);

    if(adapter->ctype == C_TYPE_NONE)
    {
      continue;
    }

    if (gimx_params.force_updates || adapter->send_command)
    {
      if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
      {
        if(adapter->dst_fd >= 0)
        {
          static unsigned char report[sizeof(adapter->axis)+2] = { BYTE_IN_REPORT, sizeof(adapter->axis) };
          memcpy(report+2, adapter->axis, sizeof(adapter->axis));
          ret = udp_send(adapter->dst_fd, report, sizeof(report));
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
      {
        if(adapter->serialdevice >= 0)
        {
          unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

          s_report_packet* report = adapter->report+index;

          switch(adapter->ctype)
          {
          case C_TYPE_SIXAXIS:
            ret = gserial_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_DS4:
            report->value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = gserial_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_T300RS_PS4:
          case C_TYPE_G29_PS4:
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = gserial_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_XONE_PAD:
            if(adapter->status)
            {
              ret = gserial_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            }
            break;
          default:
            if(adapter->ctype != C_TYPE_PS2_PAD)
            {
              ret = gserial_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            }
            else
            {
              ret = gserial_write(adapter->serialdevice, &report->value.ds2, report->length);
            }
            break;
          }
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
      {
        if(adapter->bdaddr_dst)
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


      if(gimx_params.curses)
      {
        stats_update(i);
      }

      if (adapter->send_command)
      {
        if(gimx_params.status)
        {
          adapter_dump_state(i);
#ifdef WIN32
          //There is no setlinebuf(stdout) in windows.
          fflush(stdout);
#endif
        }
        if(gimx_params.curses)
        {
          display_run(adapter_get(0)->ctype, adapter_get(0)->axis);
        }

        adapter->send_command = 0;
      }

      if(adapter->ctype == C_TYPE_DS4)
      {
        adapter->axis[ds4a_finger1_x] = 0;
        adapter->axis[ds4a_finger1_y] = 0;
        adapter->axis[ds4a_finger2_x] = 0;
        adapter->axis[ds4a_finger2_y] = 0;
      }
    }
  }
  return ret;
}

void adapter_clean()
{
  int i;
  s_adapter* adapter;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->dst_fd >= 0)
      {
        gpoll_remove_fd(adapter->src_fd);
        udp_close(adapter->dst_fd);
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bdaddr_dst)
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
      if(adapter->serialdevice >= 0)
      {
        switch(adapter->ctype)
        {
          case C_TYPE_360_PAD:
          case C_TYPE_XONE_PAD:
            usb_close(i);
            break;
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
        gserial_close(adapter->serialdevice);
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      gpp_disconnect(i);
    }
  }
}

int adapter_is_usb_auth_required(int adapter)
{
  if (adapters[adapter].atype != E_ADAPTER_TYPE_DIY_USB)
  {
    return 0;
  }
  if (controller_is_auth_required(adapters[adapter].ctype))
  {
    if (adapters[adapter].ctype != C_TYPE_360_PAD
        && adapters[adapter].ctype != C_TYPE_XONE_PAD)
    {
      return 1;
    }
    else if(adapters[adapter].status == 0)
    {
      return 1;
    }
  }
  return 0;

}
