/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serial_con.h>
#include <serial.h>
#include <dump.h>
#include <emuclient.h>
#include <report.h>

/*
 * Connect to a serial port.
 */
int serial_con_connect(char* portname)
{
  return serial_connect(portname);
}

/*
 * Send a usb report to the serial port.
 */
void serial_con_send(e_controller_type ctype, int force_update)
{
  unsigned int size;
  s_report report;
  if (force_update || controller[0].send_command)
  {
    size = report_build(&report, ctype);
    
    serial_send(&report, size);

    if(controller[0].send_command)
    {
      if (emuclient_params.status)
      {
        sixaxis_dump_state(state, 0);
      }

      controller[0].send_command = 0;
    }
  }
}

void serial_con_close()
{
  serial_close();
}
