/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module implements memory management for mosteksim
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) Created from memory.h in the z80sim
 *	       directory. Emulate memory of the Mostek AID-80F and SYS-80FT
 *	       computers by treating 0xe000-0xefff as ROM.
 * 04-NOV-2019 (Udo Munk) add functions for direct memory access
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#define MAXMEMSECT	0

extern BYTE memory[65536];

extern void init_memory(void);

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if ((addr & 0xf000) != 0xe000)
		memory[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	return memory[addr];
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	if ((addr & 0xf000) != 0xe000)
		memory[addr] = data;
}

static inline BYTE dma_read(WORD addr)
{
	return memory[addr];
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	memory[addr] = data;
}

static inline BYTE getmem(WORD addr)
{
	return memory[addr];
}

#endif /* !SIMMEM_INC */
