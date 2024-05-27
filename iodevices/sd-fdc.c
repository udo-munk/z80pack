//
// Z80SIM  -  a Z80-CPU simulator
//
// Common I/O devices used by various simulated machines
//
// Copyright (C) 2024, by Udo Munk
//
// This FDC does not emulate any existing hardware,
// special not these with some LSI chip, it is designed
// for bare metal with disk images on MicroSD and such.
// It needs one single I/O port, send command to the port,
// read result what it did, done. It does DMA transfer of
// a complete sector. It has a rock solid state engine,
// that can not get stuck by some software sending random
// data, is all ignored. It is platform independend, the
// platform must provide the low level functions, which
// depend on what storage hardware actually is used.
//
// History:
// 23-MAY-2024 implemented FDC, CP/M boot code & CBIOS
//

#include <stdint.h>
#include "simcore.h"
#include "sd-fdc.h"

// offsets in disk command descriptor
#define DD_TRACK	0 // track number
#define DD_SECTOR	1 // sector number
#define DD_DMAL		2 // DMA address low
#define DD_DMAH		3 // DMA address high

// controller commands: MSB command, LSB drive number
#define FDC_SETDD	0x10  // set address of disk descriptor from
			                  // following two OUT's
#define FDC_READ	0x20  // read sector from disk
#define FDC_WRITE	0x40  // write sector to disk

// extern functions that the platform must provide
extern BYTE read_sec(int, int, int, WORD);
extern BYTE write_sec(int, int, int, WORD);
void get_fdccmd(BYTE *, WORD);

static BYTE fdc_cmd[4];    // FDC command
static BYTE fdc_state;     // state of the FDC state machine
static BYTE fdc_stat;      // status of last FDC command
static WORD fdc_cmd_addr;  // address of the disk command
static WORD fdc_dma_addr;  // address for a DMA transfer

// I/O out interface to the 8080 CPU
const void fdc_out(BYTE data)
{
  fdc_stat = FDC_STAT_OK;

  // FDC state machine
  switch (fdc_state) {
  case 0:	// start of command phase
 
    // FDC command dispatcher, command in MSB, drive in LSB
    switch (data & 0xf0) {
    case FDC_SETDD:
      fdc_state++;
      break;

    case FDC_READ:
      get_fdccmd(&fdc_cmd[0], fdc_cmd_addr);
      fdc_dma_addr = fdc_cmd[DD_DMAL] + (fdc_cmd[DD_DMAH] << 8);
      fdc_stat = read_sec(data & 0x0f, fdc_cmd[DD_TRACK], fdc_cmd[DD_SECTOR],
                          fdc_dma_addr);
      break;

    case FDC_WRITE:
      get_fdccmd(&fdc_cmd[0], fdc_cmd_addr);
      fdc_dma_addr = fdc_cmd[DD_DMAL] + (fdc_cmd[DD_DMAH] << 8);
      fdc_stat = write_sec(data & 0x0f, fdc_cmd[DD_TRACK], fdc_cmd[DD_SECTOR],
                          fdc_dma_addr);
      break;

    default:
      break;
    }
    break;

  case 1:	// LSB of disk command address
    fdc_cmd_addr = data;
    fdc_state++;
    break;

  case 2:	// MSB of disk command address
    fdc_cmd_addr += data << 8;
    fdc_state = 0;
    break;

  default:
    break;
  }
}

// I/O in interface to the 8080 CPU
const BYTE fdc_in(void)
{
  return fdc_stat;
}
