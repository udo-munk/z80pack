/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2019 by Udo Munk
 *
 * This module implements memory management for z80sim
 *
 * History:
 * 22-NOV-16 stuff moved to here for further improvements
 * 03-FEB-17 added ROM initialization
 * 15-AUG-17 don't use macros, use inline functions that coerce appropriate
 * 04-NOV-19 add functions for direct memory access
 */

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
