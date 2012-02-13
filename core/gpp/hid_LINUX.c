#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <linux/hidraw.h>

#include <fcntl.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <errno.h>

//#define printf(...)  // comment this out for lots of info

int rhd[HIDRAW_MAX_DEVICES];
int nrhd = 0;

//  rawhid_recv - receive a packet
//    Inputs:
//  num = device to receive from (zero based)
//  buf = buffer to receive packet
//  len = buffer's size
//  timeout = time to wait, in milliseconds
//    Output:
//  number of bytes received, or -1 on error
//
int rawhid_recv(int num, void *buf, int len, int timeout)
{
  fd_set rfds;
  struct timeval tm;
  int ret;
  if (num < 0 || num >= nrhd)
  {
    fprintf(stderr, "Device not opened\n");
    exit(-1);
  }
  FD_ZERO(&rfds);
  FD_SET(rhd[num], &rfds);
  tm.tv_sec = timeout / 1000;
  tm.tv_usec = (timeout - tm.tv_sec * 1000) * 1000;
  ret = select(rhd[num] + 1, &rfds, NULL, NULL, &tm);
  if (ret > 0)
  {
    ret = read(rhd[num], buf, len);
    /*for(i=0; i<ret; ++i)
     {
     printf("%02x", ((unsigned char*)buf)[i]);
     }
     printf("\n");*/
  }
  return ret;
}

//  rawhid_send - send a packet
//    Inputs:
//  num = device to transmit to (zero based)
//  buf = buffer containing packet to send
//  len = number of bytes to transmit
//  timeout = time to wait, in milliseconds
//    Output:
//  number of bytes sent, or -1 on error
//
int rawhid_send(int num, void *buf, int len, int timeout)
{
  fd_set wfds;
  struct timeval tm;
  int ret;
  if (num < 0 || num >= nrhd)
  {
    fprintf(stderr, "Device not opened\n");
    exit(-1);
  }
  FD_ZERO(&wfds);
  FD_SET(rhd[num], &wfds);
  tm.tv_sec = timeout / 1000;
  tm.tv_usec = (timeout - tm.tv_sec * 1000) * 1000;
  //ret = select(rhd[num]+1, NULL, &wfds, NULL, &tm);
  //if(ret > 0)
  {
    ret = write(rhd[num], buf, len);
    //printf("write: %d\n", ret);
  }
  return ret;
}

//  rawhid_open - open 1 or more devices
//
//    Inputs:
//  max = maximum number of devices to open
//  vid = Vendor ID, or -1 if any
//  pid = Product ID, or -1 if any
//  usage_page = top level usage page, or -1 if any
//  usage = top level usage number, or -1 if any
//    Output:
//  actual number of devices opened
//
int rawhid_open(int max, int vid, int pid, int usage_page, int usage)
{
  int i;
  char buf[32];
  int r;
  struct hidraw_devinfo info;
  int fd;
  
  if(nrhd > 0)
  {
    fprintf(stderr, "Device alread opened\n");
    exit(-1);
  }
  
  for(i=0; i<HIDRAW_MAX_DEVICES; ++i)
  {
    snprintf(buf, sizeof(buf), "/dev/hidraw%d", i);
	fd = open(buf, O_RDWR|O_NONBLOCK);
	if (fd < 0) continue;
	r = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (r < 0) continue;	
	if(info.vendor != vid || info.product != pid) continue;
	
	rhd[nrhd] = fd;
    nrhd++;
  }
  
  return nrhd;
}

//  rawhid_close - close all opened devices
//
//    Inputs:
//  num = device to close (zero based)
//    Output
//  (nothing)
//
void rawhid_close(int num)
{
  int i;
  for(i=0; i<nrhd; ++i)
  {
    close(rhd[i]);
	rhd[i] = -1;
  }
  nrhd = 0;
}
