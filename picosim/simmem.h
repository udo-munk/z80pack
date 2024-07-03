/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module implements memory management for the Z80/8080 CPU.
 *
 * History:
 * 23-APR-2024 derived from z80sim
 * 29-JUN-2024 implemented banked memory
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simdefs.h"

extern BYTE bnk0[65536], bnk1[49152];
extern BYTE selbnk;

extern void init_memory(void);

/* Last page in memory is ROM and write protected. Some software */
/* expects a ROM in upper memory, if not it will wrap arround to */
/* address 0, and destroys itself with testing RAM access. */

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= 0xc000)) {
		if (addr < 0xff00)
			bnk0[addr] = data;
	} else {
		bnk1[addr] = data;
	}
}

static inline BYTE memrdr(WORD addr)
{
	if ((selbnk == 0) || (addr >= 0xc000))
		return bnk0[addr];
	else
		return bnk1[addr];
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= 0xc000)) {
		if (addr < 0xff00)
			bnk0[addr] = data;
	} else {
		bnk1[addr] = data;
	}
}

static inline BYTE dma_read(WORD addr)
{
	if ((selbnk == 0) || (addr >= 0xc000))
		return bnk0[addr];
	else
		return bnk1[addr];
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= 0xc000)) {
		if (addr < 0xff00)
			bnk0[addr] = data;
	} else {
		bnk1[addr] = data;
	}
}

static inline BYTE getmem(WORD addr)
{
	if ((selbnk == 0) || (addr >= 0xc000))
		return bnk0[addr];
	else
		return bnk1[addr];
}

#endif /* !SIMMEM_INC */
