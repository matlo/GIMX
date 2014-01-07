/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <stdio.h>
#include "connector.h"
#include "emuclient.h"
#include "sixaxis.h"
#include "controllers/controller.h"
#include "report.h"
#include "udp_con.h"
#include "gpp_con.h"
#include "usb_spoof.h"
#ifndef WIN32
#include <netinet/in.h>
#endif

inline int clamp(int min, int val, int max)
{
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

static const char* controller_name[C_TYPE_MAX] =
{
  "joystick",
  "360pad",
  "Sixaxis",
  "PS2pad",
  "XboxPad",
  "DS4",
  "GPP",
  "none"
};

int connector_init()
{
  int ret = 0;
  int i;
  s_controller* control;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    control = get_controller(i);
    if(control->portname)
    {
      if(!strstr(control->portname, "none"))
      {
        if((control->serial = serial_connect(control->portname)) < 0)
        {
          ret = -1;
        }
        else
        {
          int rtype = usb_spoof_get_adapter_type(control->serial);

          if(rtype >= 0)
          {
            printf(_("Detected USB adapter: %s.\n"), controller_name[rtype]);

            if(control->type == C_TYPE_DEFAULT)
            {
              control->type = rtype;
            }
            else if(control->type != rtype)
            {
              fprintf(stderr, _("Wrong controller type.\n"));
              ret = -1;
            }

            if(control->type == C_TYPE_360_PAD)
            {
              if(usb_spoof_spoof_360_controller(control->serial) < 0)
              {
                fprintf(stderr, _("Spoof failed.\n"));
                ret = -1;
              }
            }
          }
        }
      }
      if(control->type == C_TYPE_DEFAULT)
      {
        fprintf(stderr, _("No controller detected.\n"));
        ret = -1;
      }
    }
    else if(control->type == C_TYPE_GPP)
    {
      int rtype = gpp_connect();
      if (rtype < 0)
      {
        ret = -1;
      }
      else if(rtype < C_TYPE_MAX)
      {
        controller_gpp_set_refresh_periods(rtype);
      }
      else
      {
        fprintf(stderr, _("Unknown GPP controller type.\n"));
        ret = -1;
      }
    }
    else
    {
      if(control->type != C_TYPE_DEFAULT)
      {
        fprintf(stderr, _("Wrong controller type.\n"));
      }
      if(control->dst_ip)
      {
        control->dst_fd = udp_connect(control->dst_ip, control->dst_port);
        if(control->dst_fd < 0)
        {
          fprintf(stderr, _("Can't connect to port: %d.\n"), control->dst_port);
          ret = -1;
        }
      }
#ifndef WIN32
      else if(control->bdaddr_dst)
      {
        sixaxis_set_dongle(i, control->dongle_index);
        sixaxis_set_bdaddr(i, control->bdaddr_dst);
        if(sixaxis_connect(i) < 0)
        {
          fprintf(stderr, _("Can't initialize sixaxis.\n"));
          ret = -1;
        }
      }
#endif
      
    }
    /*
     * TODO MLA: Windows implementation.
     */
#ifndef WIN32
    if(control->src_ip)
    {
      control->src_fd = udp_listen(control->src_ip, control->src_port);
      if(control->src_fd < 0)
      {
        fprintf(stderr, _("Can't listen on port: %d.\n"), control->src_port);
        ret = -1;
      }
      else
      {
        GE_AddSource(control->src_fd, i, controller_network_read, udp_close);
      }
    }
#endif
  }
  return ret;
}

void connector_clean()
{
  int i;
  s_controller* controller;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = get_controller(i);
    switch(controller->type)
    {
      case C_TYPE_DEFAULT:
        if(controller->dst_fd >= 0)
        {
          udp_close(controller->dst_fd);
        }
#ifndef WIN32
        else if(controller->bdaddr_dst)
        {
          sixaxis_close(i);
        }
#endif
        break;
      case C_TYPE_GPP:
        gpp_disconnect();
        break;
      default:
        serial_close(controller->serial);
        break;
    }
  }
}

int connector_send()
{
  int ret = 0;
  int i;
  s_controller* controller;
  s_report report = {.packet_type = BYTE_SEND_REPORT};

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = get_controller(i);

    if (emuclient_params.force_updates || controller->send_command)
    {
      report.value_len = report_build(controller, &report);

      switch(controller->type)
      {
        case C_TYPE_DEFAULT:
          if(controller->dst_fd >= 0)
          {
            ret = udp_send(controller->dst_fd, (unsigned char*)controller->axis, sizeof(controller->axis));
          }
#ifndef WIN32
          else if(controller->bdaddr_dst)
          {
            ret = send_interrupt(i, &report.value.ds3);
          }
#endif
          break;
        case C_TYPE_GPP:
          ret = gpp_send(controller->axis);
          break;
        default:
          if(controller->serial >= 0)
          {
            if(controller->type != C_TYPE_PS2_PAD)
            {
              ret = serial_send(controller->serial, &report, 2+report.value_len);
            }
            else
            {
              ret = serial_send(controller->serial, &report.value.ds2, report.value_len);
            }
          }
          break;
      }

      if (controller->send_command)
      {
        if(emuclient_params.status)
        {
          controller_dump_state(controller);
        }

        controller->send_command = 0;
      }
    }
  }
  return ret;
}
