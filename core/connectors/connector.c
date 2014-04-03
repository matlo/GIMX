/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <stdio.h>
#include "emuclient.h"
#include "connectors/connector.h"
#include "connectors/sixaxis.h"
#include "connectors/btds4.h"
#include "connectors/udp_con.h"
#include "connectors/gpp_con.h"
#include "connectors/usb_spoof.h"
#include <adapter.h>
#include <report.h>
#ifndef WIN32
#include <netinet/in.h>
#include <poll.h>
#endif

int connector_init()
{
  int ret = 0;
  int i;
  s_adapter* controller;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = adapter_get(i);
    if(controller->portname)
    {
      if(!strstr(controller->portname, "none"))
      {
        if((controller->serial = serial_connect(controller->portname)) < 0)
        {
          ret = -1;
        }
        else
        {
          int rtype = usb_spoof_get_adapter_type(controller->serial);

          if(rtype >= 0)
          {
            printf(_("Detected USB adapter: %s.\n"), controller_get_name(rtype));

            if(controller->type == C_TYPE_DEFAULT)
            {
              controller->type = rtype;
            }
            else if(controller->type != rtype)
            {
              fprintf(stderr, _("Wrong controller type.\n"));
              ret = -1;
            }

            if(controller->type == C_TYPE_360_PAD)
            {
              if(usb_spoof_spoof_360_controller(controller->serial) < 0)
              {
                fprintf(stderr, _("Spoof failed.\n"));
                ret = -1;
              }
            }
            else if(controller->type == C_TYPE_XONE_PAD)
            {
              /*
               * TODO XONE
               */
            }
            else if(controller->type == C_TYPE_DS4)
            {
              controller->report.value.ds4.report_id = 0x01;
              controller->report.value_len = DS4_USB_INTERRUPT_PACKET_SIZE;
              ds4_init_report(&controller->report.value.ds4);
            }
          }
        }
      }
      if(controller->type == C_TYPE_DEFAULT)
      {
        fprintf(stderr, _("No controller detected.\n"));
        ret = -1;
      }
    }
    else if(controller->type == C_TYPE_GPP)
    {
      int rtype = gpp_connect();
      if (rtype < 0)
      {
        fprintf(stderr, _("No controller detected.\n"));
        ret = -1;
      }
      else if(rtype < C_TYPE_MAX)
      {
        printf(_("Detected controller: %s.\n"), controller_get_name(rtype));
        controller_gpp_set_params(rtype);
      }
      else
      {
        fprintf(stderr, _("Unknown GPP controller type.\n"));
        ret = -1;
      }
    }
    else
    {
      if(controller->dst_ip)
      {
        controller->dst_fd = udp_connect(controller->dst_ip, controller->dst_port);
        if(controller->dst_fd < 0)
        {
          fprintf(stderr, _("Can't connect to port: %d.\n"), controller->dst_port);
          ret = -1;
        }
      }
#ifndef WIN32
      else if(controller->bdaddr_dst)
      {
        if(controller->type == C_TYPE_SIXAXIS
            || controller->type == C_TYPE_DEFAULT)
        {
          sixaxis_set_dongle(i, controller->dongle_index);
          sixaxis_set_bdaddr(i, controller->bdaddr_dst);
          if(sixaxis_connect(i) < 0)
          {
            fprintf(stderr, _("Can't initialize sixaxis.\n"));
            ret = -1;
          }
        }
        else if(controller->type == C_TYPE_DS4)
        {
          btds4_set_dongle(i, controller->dongle_index);
          btds4_set_bdaddr(i, controller->bdaddr_dst);
          if(btds4_init(i) < 0)
          {
            fprintf(stderr, _("Can't initialize btds4.\n"));
            ret = -1;
          }
          ds4_init_report(&controller->report.value.ds4);
        }
        else
        {
          fprintf(stderr, _("Wrong controller type.\n"));
        }
      }
#endif
      
    }
    /*
     * TODO MLA: Windows implementation.
     */
#ifndef WIN32
    if(controller->src_ip)
    {
      controller->src_fd = udp_listen(controller->src_ip, controller->src_port);
      if(controller->src_fd < 0)
      {
        fprintf(stderr, _("Can't listen on port: %d.\n"), controller->src_port);
        ret = -1;
      }
      else
      {
        GE_AddSource(controller->src_fd, i, adapter_network_read, NULL, udp_close);
      }
    }
#endif
  }
  return ret;
}

void connector_clean()
{
  int i;
  s_adapter* controller;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = adapter_get(i);
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
          if(controller->type == C_TYPE_SIXAXIS
              || controller->type == C_TYPE_DEFAULT)
          {
            sixaxis_close(i);
          }
          else if(controller->type == C_TYPE_DS4)
          {
            btds4_close(i);
          }
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
  s_adapter* controller;
  s_report* report;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = adapter_get(i);
    report = &controller->report;

    if (emuclient_params.force_updates || controller->send_command)
    {
      report->value_len = report_build(controller->type, controller->axis, report);

      switch(controller->type)
      {
        case C_TYPE_DEFAULT:
          if(controller->dst_fd >= 0)
          {
            /*
             * Do not send useless reports over the network.
             * The remote gimx can force updates.
             */
            if(controller->send_command)
            {
              ret = udp_send(controller->dst_fd, (unsigned char*)controller->axis, sizeof(controller->axis));
            }
          }
#ifndef WIN32
          else if(controller->bdaddr_dst)
          {
            ret = sixaxis_send_interrupt(i, &report->value.ds3);
          }
#endif
          break;
        case C_TYPE_SIXAXIS:
          if(controller->bdaddr_dst)
          {
            ret = sixaxis_send_interrupt(i, &report->value.ds3);
          }
          else if(controller->serial >= 0)
          {
            ret = serial_send(controller->serial, report, 2+report->value_len);
          }
          break;
        case C_TYPE_DS4:
          if(controller->bdaddr_dst)
          {
            ret = btds4_send_interrupt(i, &report->value.ds4);
          }
          else if(controller->serial >= 0)
          {
            ret = serial_send(controller->serial, &report, 2+report->value_len);
          }
          break;
        case C_TYPE_GPP:
          ret = gpp_send(controller->axis);
          break;
        default:
          if(controller->serial >= 0)
          {
            if(controller->type != C_TYPE_PS2_PAD)
            {
              ret = serial_send(controller->serial, &report, 2+report->value_len);
            }
            else
            {
              ret = serial_send(controller->serial, &report->value.ds2, report->value_len);
            }
          }
          break;
      }

      if (controller->send_command)
      {
        if(emuclient_params.status)
        {
          adapter_dump_state(controller);
        }

        controller->send_command = 0;
      }
    }
  }
  return ret;
}
