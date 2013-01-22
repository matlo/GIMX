/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serial.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "emuclient.h"

#ifndef __ARM_ARCH_6__
#include <termios.h>

/*
 * The serial connection.
 */
static int serial = -1;

/*
 * The baud rate in bps.
 */
#define BAUDRATE B500000

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
  struct termios options;
  int ret = 0;

  gprintf(_("connecting to %s\n"), portname);

  if ((serial = open(portname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
    ret = -1;
  }
  else
  {
    tcgetattr(serial, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_oflag &= ~OPOST;
    if(tcsetattr(serial, TCSANOW, &options) < 0)
    {
      printf(_("can't set serial port options\n"));
      ret = -1;
    }
    else
    {
      gprintf(_("connected\n"));
    }
  }

  return ret;
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(void* pdata, unsigned int size)
{
  return write(serial, (uint8_t*)pdata, size);
}

void serial_close()
{
  close(serial);
}

#else
#include <sys/mman.h>
#include <stdlib.h>

#define BCM2708_BASE    0x20000000                      // 0x7e000000
#define GPIO_BASE       (BCM2708_BASE + 0x200000)       // 0x7e200000
#define AUX_BASE        (BCM2708_BASE + 0x215000)       // 0x7e215000

#define PAGE_SIZE       0x1000  // 4K
#define BLOCK_SIZE      0x1000  // 4K

enum eAuxWordOffsets {
  kAuxIRQ               = 0x00, // AUX_IRQ
  kAuxEnables           = 0x01, // AUX_ENABLES
  kAuxMiniUartIO        = 0x10, // AUX_MU_IO_REG
  kAuxMiniUartIntEnable = 0x11, // AUX_MU_IER_REG
  kAuxMiniUartIntId     = 0x12, // AUX_MU_IIR_REG
  kAuxMiniUartLineCtrl  = 0x13, // AUX_MU_LCR_REG
  kAuxMiniUartModemCtrl = 0x14, // AUX_MU_MCR_REG
  kAuxMiniUartLineStat  = 0x15, // AUX_MU_LSR_REG
  kAuxMiniUartModemStat = 0x16,
  kAuxMiniUartScratch   = 0x17,
  kAuxMiniUartExtraCtrl = 0x18, // AUX_MU_CNTL_REG
  kAuxMiniUartExtraStat = 0x19, // AUX_MU_STAT_REG
  kAuxMiniUartBaudRate  = 0x1a
};

enum eAuxEnableMask {
  kAuxEnableMiniUart    = 0x01,
  kAuxEnableSPI1        = 0x02,
  kAuxEnableSPI2        = 0x04
};

enum eUART1LineCtrlMask {
  kUART1DLAB            = 0b10000000,
  kUART1Break           = 0b01000000,
  kUART18Bit            = 0b00000011
};

enum eUART1ExtraCtrlMask {
  kUART1CTSAssert       = 0b10000000,
  kUART1RTSAssert       = 0b01000000,
  kUART1RTSAuto         = 0b00110000,
  kUART1UseCTS          = 0b00001000,
  kUART1RTSIsRTR        = 0b00000100,
  kUART1TxEnable        = 0b00000010,
  kUART1RxEnable        = 0b00000001
};

static volatile unsigned long *mAux = NULL;

static volatile unsigned long *mapMem (unsigned long armAddr, unsigned int memSize)
{
  char *mem, *map;
  long pageSize = sysconf ( _SC_PAGE_SIZE );

  /* open /dev/mem */
  int memFD = open("/dev/mem", O_RDWR|O_SYNC);
  if ( memFD < 0 ) {
    perror ( "can't open /dev/mem" );
    return NULL;
  }

  // Allocate MAP block
  int status = posix_memalign ( (void **) &mem, pageSize, memSize );
  if ( status != 0 )
  {
    perror ( "Can't get aligned memory" );
    close(memFD);
    return NULL;
  }

  // Now map it
  map = (char *) mmap (
    (caddr_t) mem,              /* start */
    memSize,                    /* length */
    PROT_READ|PROT_WRITE,       /* prot */
    MAP_SHARED|MAP_FIXED,       /* flags */
    memFD,                      /* fd */
    armAddr                     /* offset */
  );
  if ( (long) map < 0)
  {
    perror ( "Can't map memory" );
    close(memFD);
    return NULL;
  }

  close(memFD);

  // Always use volatile pointer!
  return (volatile unsigned long*) map;
}

#define BAUDRATE 500000//bps
#define FREQ 500000000//Hz
#define BAUDRATE_REG_VALUE FREQ/(BAUDRATE*8)-1

static int initUART()
{
  mAux = mapMem ( AUX_BASE, 0x1000 );
  if(mAux == NULL)
  {
    return -1;
  }
  // Enable only the mini uart
  *(mAux + kAuxEnables) = kAuxEnableMiniUart;
  // Set Speed
  *(mAux + kAuxMiniUartBaudRate) = BAUDRATE_REG_VALUE;
  // 8-bit mode - SEE ERRATA!!
  *(mAux + kAuxMiniUartLineCtrl) = kUART18Bit;
  // Enable Transmitter and Receiver
  *(mAux + kAuxMiniUartExtraCtrl) = kUART1TxEnable | kUART1RxEnable;
  return 0;
}

// GPIO setup macros.
// Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(mGPIO+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(mGPIO+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(mGPIO+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

enum eGPIOPinsUsed {
  kBufferDirectionPin   = 23,
  kScopeTriggerPin      = 18,
  kUART1TxPin           = 14,
  kUART1RxPin           = 15,
  kUART1CTSPin          = 16,
  kUART1RTSPin          = 17
};

volatile unsigned long *mGPIO;

static int initGPIO() {
  // Map the GPIO and Aux memory
  mGPIO = mapMem ( GPIO_BASE, 0x1000 );

  // UART1 = Mini-UART, UART0 - PL011
  // Set GPIO15 and 16 for UART1 to ALT5 Mini-Uart
  INP_GPIO ( kBufferDirectionPin );
  OUT_GPIO ( kBufferDirectionPin );
  INP_GPIO ( kScopeTriggerPin );
  OUT_GPIO ( kScopeTriggerPin );
  INP_GPIO ( kUART1TxPin );
  SET_GPIO_ALT ( kUART1TxPin, 5 );
  INP_GPIO ( kUART1RxPin );
  SET_GPIO_ALT ( kUART1RxPin, 5 );
  INP_GPIO ( kUART1RTSPin);
  SET_GPIO_ALT ( kUART1RTSPin, 5 );
  return 0;
}

#define FIFO_FULL 8

static void sendByte (unsigned char byte)
{
  // Wait till FIFO has only 3 bytes in it
  while ( ( *(mAux + kAuxMiniUartExtraStat) >> 24 ) == FIFO_FULL )
    ;
  // Write the character
  *(mAux + kAuxMiniUartIO) = byte;
}

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
  if(!mAux)
  {
    initGPIO();
    return initUART();
  }
  return -1;
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(void* pdata, unsigned int size)
{
  int i;
  for(i=0; i<size; ++i)
  {
    sendByte(((char*)pdata)[i]);
  }
  return size;
}

void serial_close()
{
}
#endif
