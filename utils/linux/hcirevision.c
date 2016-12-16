/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 
 This has been extracted from the hciconfig.c and csr.c files of the bluez tools.
 */

#include <stdlib.h>
#include <stdio.h>

#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define CSR_VARID_CHIPVER     0x281a  /* uint16 */
#define CSR_VARID_CHIPREV     0x281b  /* uint16 */

int csr_read_pskey_complex(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint8_t *value, uint16_t length)
{
  unsigned char cmd[] = { 0x00, 0x00, ((length / 2) + 8) & 0xff, ((length / 2) + 8) >> 8,
        seqnum & 0xff, seqnum >> 8, 0x03, 0x70, 0x00, 0x00,
        pskey & 0xff, pskey >> 8,
        (length / 2) & 0xff, (length / 2) >> 8,
        stores & 0xff, stores >> 8, 0x00, 0x00 };

  unsigned char cp[254], rp[254];
  struct hci_request rq;

  memset(&cp, 0, sizeof(cp));
  cp[0] = 0xc2;
  memcpy(cp + 1, cmd, sizeof(cmd));

  memset(&rq, 0, sizeof(rq));
  rq.ogf    = OGF_VENDOR_CMD;
  rq.ocf    = 0x00;
  rq.event  = EVT_VENDOR;
  rq.cparam = cp;
  rq.clen   = sizeof(cmd) + length - 1;
  rq.rparam = rp;
  rq.rlen   = sizeof(rp);

  if (hci_send_req(dd, &rq, 2000) < 0)
    return -1;

  if (rp[0] != 0xc2) {
    errno = EIO;
    return -1;
  }

  if ((rp[9] + (rp[10] << 8)) != 0) {
    errno = ENXIO;
    return -1;
  }

  memcpy(value, rp + 17, length);

  return 0;
}

int csr_read_pskey_uint16(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint16_t *value)
{
  uint8_t array[2] = { 0x00, 0x00 };
  int err;

  err = csr_read_pskey_complex(dd, seqnum, pskey, stores, array, 2);

  *value = array[0] + (array[1] << 8);

  return err;
}

char *csr_chipvertostr(uint16_t ver, uint16_t rev)
{
  switch (ver) {
  case 0x00:
    return "BlueCore01a";
  case 0x01:
    switch (rev) {
    case 0x64:
      return "BlueCore01b (ES)";
    case 0x65:
    default:
      return "BlueCore01b";
    }
    break;
  case 0x02:
    switch (rev) {
    case 0x89:
      return "BlueCore02-External (ES2)";
    case 0x8a:
      return "BlueCore02-External";
    case 0x28:
      return "BlueCore02-ROM/Audio/Flash";
    default:
      return "BlueCore02";
    }
    break;
  case 0x03:
    switch (rev) {
    case 0x43:
      return "BlueCore3-MM";
    case 0x15:
      return "BlueCore3-ROM";
    case 0xe2:
      return "BlueCore3-Flash";
    case 0x26:
      return "BlueCore4-External";
    case 0x30:
      return "BlueCore4-ROM";
    default:
      return "BlueCore3 or BlueCore4";
    }
    break;
  case 0x07:
    switch (rev) {
    case 0x00:
      return "CSR8510";
    default:
      return "CSR8510 or later";
    }
    break;
  default:
    return "Unknown";
  }
}

int csr_read_varid_uint16(int dd, uint16_t seqnum, uint16_t varid, uint16_t *value)
{
  unsigned char cmd[] = { 0x00, 0x00, 0x09, 0x00,
        seqnum & 0xff, seqnum >> 8, varid & 0xff, varid >> 8, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  unsigned char cp[254], rp[254];
  struct hci_request rq;

  memset(&cp, 0, sizeof(cp));
  cp[0] = 0xc2;
  memcpy(cp + 1, cmd, sizeof(cmd));

  memset(&rq, 0, sizeof(rq));
  rq.ogf    = OGF_VENDOR_CMD;
  rq.ocf    = 0x00;
  rq.event  = EVT_VENDOR;
  rq.cparam = cp;
  rq.clen   = sizeof(cmd) + 1;
  rq.rparam = rp;
  rq.rlen   = sizeof(rp);

  if (hci_send_req(dd, &rq, 2000) < 0)
    return -1;

  if (rp[0] != 0xc2) {
    errno = EIO;
    return -1;
  }

  if ((rp[9] + (rp[10] << 8)) != 0) {
    errno = ENXIO;
    return -1;
  }

  *value = rp[11] + (rp[12] << 8);

  return 0;
}

static void print_rev_csr(int dd, uint16_t rev __attribute__((unused)))
{
  uint16_t chipver, chiprev;

  if (!csr_read_varid_uint16(dd, 1, CSR_VARID_CHIPVER, &chipver)) {
    if (csr_read_varid_uint16(dd, 2, CSR_VARID_CHIPREV, &chiprev) < 0)
      chiprev = 0;
    printf("\tChip version: %s\n", csr_chipvertostr(chipver, chiprev));
  }
}

static void cmd_revision(int ctl __attribute__((unused)), int hdev, char *opt __attribute__((unused)))
{
  struct hci_version ver;
  int dd;

  dd = hci_open_dev(hdev);
  if (dd < 0) {
    fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
            hdev, strerror(errno), errno);
    return;
  }

  if (hci_read_local_version(dd, &ver, 1000) < 0) {
    fprintf(stderr, "Can't read version info for hci%d: %s (%d)\n",
            hdev, strerror(errno), errno);
    return;
  }

  switch (ver.manufacturer) {
  case 10:
    print_rev_csr(dd, ver.hci_rev);
    break;
  default:
    break;
  }
  return;
}

int main(int argc, char *argv[])
{
  int i;

  if(argc > 1 && sscanf(argv[1], "hci%d", &i) > 0)
  {
    cmd_revision(0, i, NULL);
  }

  return 0;
}
