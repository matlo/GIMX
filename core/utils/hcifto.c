/*
 * This small program allows to set the flush timeout for a bluetooth device.
 * It only modifies the flush timeout for opened connections.
 * 
 * Usage: ./hcifto <bluetooth device address> <flush timeout - unit: 0.625 ms>
 * 
 * Requires libbluetooth-dev
 * Compile with: gcc -o hcifto hcifto.c -lbluetooth
 * 
 * set_flush_timeout function was copied from:
 * http://people.csail.mit.edu/albert/bluez-intro/x559.html#bzi-l2cap-unreliable
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

static int set_flush_timeout(bdaddr_t *ba, int timeout)
{
    int err = 0, dd;
    struct hci_conn_info_req *cr = 0;
    struct hci_request rq = { 0 };

    struct {
        uint16_t handle;
        uint16_t flush_timeout;
    } cmd_param;

    struct {
        uint8_t  status;
        uint16_t handle;
    } cmd_response;

    // find the connection handle to the specified bluetooth device
    cr = (struct hci_conn_info_req*) malloc(
            sizeof(struct hci_conn_info_req) + 
            sizeof(struct hci_conn_info));
    bacpy( &cr->bdaddr, ba );
    cr->type = ACL_LINK;
    dd = hci_open_dev( hci_get_route( &cr->bdaddr ) );
    if( dd < 0 ) {
        err = dd;
        goto cleanup;
    }
    err = ioctl(dd, HCIGETCONNINFO, (unsigned long) cr );
    if( err ) goto cleanup;

    // build a command packet to send to the bluetooth microcontroller
    cmd_param.handle = cr->conn_info->handle;
    cmd_param.flush_timeout = htobs(timeout);
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = 0x28;
    rq.cparam = &cmd_param;
    rq.clen = sizeof(cmd_param);
    rq.rparam = &cmd_response;
    rq.rlen = sizeof(cmd_response);
    rq.event = EVT_CMD_COMPLETE;

    // send the command and wait for the response
    err = hci_send_req( dd, &rq, 0 );
    if( err ) goto cleanup;

    if( cmd_response.status ) {
        err = -1;
        errno = bt_error(cmd_response.status);
    }

cleanup:
    free(cr);
    if( dd >= 0) close(dd);
    return err;
}

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    printf("usage: ./hcifto <bluetooth device address> <flush timeout - unit: 0.625 ms>\n");
    exit(-1);
  }

  bdaddr_t ba;
  str2ba(argv[1], &ba);
  int fto = atoi(argv[2]);
  if(set_flush_timeout(&ba, fto) < 0)
  {
    fprintf(stderr, "can't set flush timeout: %s\n", strerror(errno));
    return -1;
  }
  else
  {
    fprintf(stdout, "flush timeout set for %s: %s\n", argv[1], fto ? argv[2] : "infinite");
  }
  
  return 0;
}
