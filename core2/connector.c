/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include "connector.h"
#include "tcp_con.h"
#include "serial_con.h"
#include "gpp_con.h"

static e_controller_type type;

int connector_init(e_controller_type t, char* port)
{
  int ret = 0;
  switch(t)
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
      if(!strstr(port, "none") && serial_con_connect(port) < 0)
      {
        ret = -1;
      }
      break;
  }
  return ret;
}

void connector_clean()
{
  switch(type)
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
