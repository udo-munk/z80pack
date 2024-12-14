/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 * Copyright (C) 2024 Thomas Eberhardt
 *
 * This module implements memory management for mosteksim
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) Created from memory.h in the z80sim
 *	       directory. Emulate memory of the Mostek AID-80F and SYS-80FT
 *	       computers by treating 0xe000-0xefff as ROM.
 * 04-NOV-2019 (Udo Munk) add functions for direct memory access
 * 14-DEC-2024 (Thomas Eberhardt) added hardware breakpoint support
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simdefs.h"
#ifdef WANT_ICE
#include "simice.h"
#endif

#ifdef BUS_8080
#include "simglb.h"
#endif

#define MAXMEMSECT	0

extern BYTE memory[65536];

extern void init_memory(void);

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
#ifdef BUS_8080
	cpu_bus &= ~(CPU_M1 | CPU_WO | CPU_MEMR);
#endif

#ifdef WANT_HB
	if (hb_flag && hb_addr == addr && (hb_mode & HB_WRITE))
		hb_trig = HB_WRITE;
#endif

	if ((addr & 0xf000) != 0xe000)
		memory[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	register BYTE data;

#ifdef WANT_HB
	if (hb_flag && hb_addr == addr) {
		if (cpu_bus & CPU_M1) {
			if (hb_mode & HB_EXEC)
				hb_trig = HB_EXEC;
		} else {
			if (hb_mode & HB_READ)
				hb_trig = HB_READ;
		}
	}
#endif

	data = memory[addr];

#ifdef BUS_8080
	cpu_bus &= ~CPU_M1;
	cpu_bus |= CPU_WO | CPU_MEMR;
#endif

	return data;
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
