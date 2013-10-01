/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serial_con.h>
#include <serial.h>
#include <dump.h>
#include <emuclient.h>
#include <report.h>
#include <usb_spoof.h>

/*
 * Connect to a serial port.
 */
int serial_con_connect(char* portname)
{
  return serial_connect(portname);
}

e_controller_type serial_con_get_type()
{
  unsigned char get_type_request[] = {BYTE_TYPE, BYTE_LEN_0_BYTE};

  if(serial_send(get_type_request, sizeof(get_type_request)) == sizeof(get_type_request))
  {
    unsigned char get_type_answer[3];

    if(serial_recv(get_type_answer, sizeof(get_type_answer)) == sizeof(get_type_answer))
    {
      if(get_type_answer[0] == BYTE_TYPE && get_type_answer[1] == BYTE_LEN_1_BYTE)
      {
        return get_type_answer[2];
      }
    }
  }

  return C_TYPE_DEFAULT;
}

/*
 * Send a usb report to the serial port.
 */
int serial_con_send(e_controller_type ctype, int force_update)
{
  unsigned int size;
  s_report report;
  int ret = 0;
  if (force_update || controller[0].send_command)
  {
    size = report_build(&report, ctype);
    
    if(ctype != C_TYPE_PS2_PAD)
    {
      ret = serial_send(&report, size);
    }
    else
    {
      ret = serial_send(&report.value.ps2, size);
    }

    if(controller[0].send_command)
    {
      if (emuclient_params.status)
      {
        sixaxis_dump_state(state, 0);
      }

      controller[0].send_command = 0;
    }
  }
  return ret;
}

void serial_con_close()
{
  serial_close();
}
