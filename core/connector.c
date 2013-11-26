/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <stdio.h>
#include "connector.h"
#include "emuclient.h"
#include "controllers/controller.h"
#include "report.h"
#include "tcp_con.h"
#include "gpp_con.h"
#include "usb_spoof.h"

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
  s_controller* controller;
  unsigned short port = TCP_PORT;
  char* localhost = "127.0.0.1";

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    controller = get_controller(i);
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
            printf(_("Detected USB adapter: %s.\n"), controller_name[rtype]);

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
              if(usb_spoof_spoof_360_controller() < 0)
              {
                fprintf(stderr, _("Spoof failed.\n"));
                ret = -1;
              }
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
      if (gpp_connect() < 0)
      {
        ret = -1;
      }
    }
    else
    {
      if(controller->type != C_TYPE_DEFAULT)
      {
        fprintf(stderr, _("Wrong controller type.\n"));
      }
      if(!controller->ip)
      {
        controller->ip = localhost;
        controller->port = port;
        ++port;
      }
      controller->netfd = tcp_connect(controller->ip, controller->port);
    }
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
        tcp_close(controller->netfd);
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
          if(controller->netfd >= 0)
          {
            ret = tcp_send(controller->netfd, (unsigned char*)&report.value.ds3, report.value_len);
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
