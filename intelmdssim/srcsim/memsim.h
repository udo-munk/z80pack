/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module implements memory management for an Intel MDS-800 system
 *
 * History:
 */

#ifndef MEMSIM_INC
#define MEMSIM_INC

extern void init_memory(void);
extern BYTE memory[];

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	memory[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	return (memory[addr]);
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	memory[addr] = data;
}

static inline BYTE dma_read(WORD addr)
{
	return (memory[addr]);
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
	return (memory[addr]);
}

#endif
