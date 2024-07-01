/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module implements memory management for the Z80/8080 CPU
 *
 * History:
 * 23-APR-2024 derived from z80sim
 */

#ifndef MEMSIM_INC
#define MEMSIM_INC

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

extern void init_memory(void);
extern BYTE memory[];

/* Last page in memory is ROM and write protected. Some software */
/* expects a ROM in upper memory, if not it will wrap arround to */
/* address 0, and destroys itself with testing RAM access. */

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if (addr < 0xff00)
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
	if (addr < 0xff00)
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
	if (addr < 0xff00)
		memory[addr] = data;
}

static inline BYTE getmem(WORD addr)
{
	return memory[addr];
}

#endif
