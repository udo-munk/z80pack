/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2019 by Udo Munk
 *
 * This module implements memory management for z80sim
 *
 * History:
 * 22-NOV-2016 stuff moved to here for further improvements
 * 03-FEB-2017 added ROM initialization
 * 15-AUG-2017 don't use macros, use inline functions that coerce appropriate
 * 04-NOV-2019 add functions for direct memory access
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
