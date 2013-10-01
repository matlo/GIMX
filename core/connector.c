/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <stdio.h>
#include "connector.h"
#include "emuclient.h"
#include "tcp_con.h"
#include "serial_con.h"
#include "gpp_con.h"
#include "usb_spoof.h"

static const char* controller_name[C_TYPE_MAX] =
{
  "joystick",
  "360pad",
  "Sixaxis",
  "PS2pad",
  "XboxPad",
  "GPP",
  "none"
};

int connector_init()
{
  int ret = 0;
  if(!emuclient_params.portname)
  {
    switch(emuclient_params.ctype)
    {
      case C_TYPE_DEFAULT:
        if(tcp_connect() < 0)
        {
          ret = -1;
        }
        break;
      case C_TYPE_GPP:
        if (gpp_connect() < 0)
        {
          ret = -1;
        }
        break;
      default:
        break;
    }
  }
  else
  {
    if(!strstr(emuclient_params.portname, "none"))
    {
      if(serial_con_connect(emuclient_params.portname) < 0)
      {
        ret = -1;
      }
      else
      {
        e_controller_type type = serial_con_get_type();
        printf(_("Detected USB adapter: %s.\n"), controller_name[type]);

        if(emuclient_params.ctype == C_TYPE_DEFAULT)
        {
          emuclient_params.ctype = type;
        }
        else if(emuclient_params.ctype != type)
        {
          fprintf(stderr, _("Wrong controller type.\n"));
          ret = -1;
        }

        if(emuclient_params.ctype == C_TYPE_360_PAD)
        {
          if(usb_spoof_spoof_360_controller() < 0)
          {
            fprintf(stderr, _("Spoof failed.\n"));
            ret = -1;
          }
        }
      }
    }
    if(emuclient_params.ctype == C_TYPE_DEFAULT)
    {
      fprintf(stderr, _("No controller detected.\n"));
      ret = -1;
    }
  }
  return ret;
}

void connector_clean()
{
  switch(emuclient_params.ctype)
  {
    case C_TYPE_DEFAULT:
      tcp_close();
      break;
    case C_TYPE_GPP:
      gpp_disconnect();
      break;
    default:
      serial_con_close();
      break;
  }
}

int connector_send()
{
  int ret = 0;
  switch(emuclient_params.ctype)
  {
    case C_TYPE_DEFAULT:
      ret = tcp_send(emuclient_params.force_updates);
      break;
    case C_TYPE_GPP:
      ret = gpp_send(emuclient_params.force_updates);
      break;
    default:
      ret = serial_con_send(emuclient_params.ctype, emuclient_params.force_updates);
      if(strstr(emuclient_params.portname, "none"))
      {
        ret = 0;
      }
      break;
  }
  return ret;
}
