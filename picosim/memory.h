/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module implements memory management for picosim
 *
 * History:
 * 23-APR-24 derived from z80sim
 */

extern void init_memory(void);
extern BYTE code[];

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	code[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	return (code[addr]);
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	code[addr] = data;
}

static inline BYTE dma_read(WORD addr)
{
	return (code[addr]);
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	code[addr] = data;
}

static inline BYTE getmem(WORD addr)
{
	return (code[addr]);
}
