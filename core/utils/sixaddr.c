/*
 * Derived from sixpair.c version 2007-04-18
 * Compile with: gcc -o sixaddr sixaddr.c -lusb
 * Displays the bdaddr of the PS3 and the bdaddr of the sixaxis.
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <usb.h>

#define VENDOR 0x054c
#define PRODUCT 0x0268

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

void fatal(char *msg) { perror(msg); exit(1); }

void show_master(usb_dev_handle *devh, int itfnum) {
  
  printf("Current Bluetooth master: ");
  unsigned char msg[8];
  int res = usb_control_msg
    (devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
     0x01, 0x03f5, itfnum, (void*)msg, sizeof(msg), 5000);
  if ( res < 0 ) { perror("USB_REQ_GET_CONFIGURATION"); return; }
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	 msg[2], msg[3], msg[4], msg[5], msg[6], msg[7]);
}

void show_sixaxis_bdaddr(usb_dev_handle *devh, int itfnum)
{
  char buf[18];
  usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE, 0x01,
				(3 << 8) | 0xf2, itfnum, buf, 17, 5000);
  printf("Current Bluetooth Device Address: ");
  printf("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n", buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
}

void process_device(struct usb_device *dev,
		    struct usb_config_descriptor *cfg, int itfnum) {


  usb_dev_handle *devh = usb_open(dev);
  if ( ! devh ) fatal("usb_open");

  usb_detach_kernel_driver_np(devh, itfnum);

  int res = usb_claim_interface(devh, itfnum);
  if ( res < 0 ) fatal("usb_claim_interface");

  show_master(devh, itfnum);

  show_sixaxis_bdaddr(devh, itfnum);
  
  usb_close(devh);
}

int main(int argc, char *argv[]) {  

  usb_init();
  if ( usb_find_busses() < 0 ) fatal("usb_find_busses");
  if ( usb_find_devices() < 0 ) fatal("usb_find_devices");
  struct usb_bus *busses = usb_get_busses();
  if ( ! busses ) fatal("usb_get_busses");

  int found = 0;

  struct usb_bus *bus;
  for ( bus=busses; bus; bus=bus->next ) {
    struct usb_device *dev;
    for ( dev=bus->devices; dev; dev=dev->next) {
      struct usb_config_descriptor *cfg;
      for ( cfg = dev->config;
	    cfg < dev->config + dev->descriptor.bNumConfigurations;
	    ++cfg ) {
	int itfnum;
	for ( itfnum=0; itfnum<cfg->bNumInterfaces; ++itfnum ) {
	  struct usb_interface *itf = &cfg->interface[itfnum];
	  struct usb_interface_descriptor *alt;
	  for ( alt = itf->altsetting;
		alt < itf->altsetting + itf->num_altsetting;
		++alt ) {
	    if ( dev->descriptor.idVendor == VENDOR &&
		 dev->descriptor.idProduct == PRODUCT &&
		 alt->bInterfaceClass == 3 ) {
	      process_device(dev, cfg, itfnum);
	      ++found;
	    }
	  }
	}
      }
    }
  }

  if ( ! found ) printf("No controller found on USB busses.\n");
  return 0;

}

