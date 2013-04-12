/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include "connector.h"
#include "emuclient.h"
#include "tcp_con.h"
#include "serial_con.h"
#include "gpp_con.h"

int connector_init()
{
  int ret = 0;
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
      if(!strstr(emuclient_params.portname, "none") && serial_con_connect(emuclient_params.portname) < 0)
      {
        ret = -1;
      }
      break;
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
