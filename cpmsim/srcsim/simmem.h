/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2019 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module implements banked memory management for cpmsim
 *
 *      MMU:
 *      ===
 *
 *      +--------+
 * 16KB | common |
 *      +--------+
 *      +--------+  +--------+  ..........  +--------+
 *      |        |  |        |              |        |
 * 48KB |        |  |        |  ..........  |        |
 *      | bank 0 |  | bank 1 |              | bank n |
 *      +--------+  +--------+  ..........  +--------+
 *
 * This is an example for 48KB segments as it was implemented originally.
 * The segment size now can be configured via port 22.
 * If the segment size isn't configured the default is 48 KB as it was
 * before, to maintain compatibility.
 *
 * History:
 * 22-NOV-2016 stuff moved to here for further improvements
 * 03-FEB-2017 added ROM initialization
 * 09-APR-2018 modified MMU write protect port as used by Alan Cox for FUZIX
 * 04-NOV-2019 add functions for direct memory access
 * 14-DEC-2024 added hardware breakpoint support
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

#define MAXSEG 16		/* max. number of memory banks */
#define SEGSIZ 49152		/* default size of one bank = 48 KBytes */

extern void init_memory(void);

extern BYTE *memory[MAXSEG];
extern int selbnk, maxbnk, segsize, wp_common;

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

	if ((addr >= segsize) && (wp_common != 0)) {
		wp_common |= 0x80;
#ifndef EXCLUDE_Z80
		if (wp_common & 0x40)
			int_nmi = 1;
#endif
		return;
	}

	if (selbnk == 0) {
		*(memory[0] + addr) = data;
	} else {
		if (addr >= segsize)
			*(memory[0] + addr) = data;
		else
			*(memory[selbnk] + addr) = data;
	}
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

	if (selbnk == 0) {
		data = *(memory[0] + addr);
	} else {
		if (addr >= segsize)
			data = *(memory[0] + addr);
		else
			data = *(memory[selbnk] + addr);
	}

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
	if ((addr >= segsize) && (wp_common != 0)) {
		wp_common |= 0x80;
		return;
	}

	if (selbnk == 0) {
		*(memory[0] + addr) = data;
	} else {
		if (addr >= segsize)
			*(memory[0] + addr) = data;
		else
			*(memory[selbnk] + addr) = data;
	}
}

static inline BYTE dma_read(WORD addr)
{
	if (selbnk == 0)
		return *(memory[0] + addr);

	if (addr >= segsize)
		return *(memory[0] + addr);
	else
		return *(memory[selbnk] + addr);
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	if (selbnk == 0) {
		*(memory[0] + addr) = data;
	} else {
		if (addr >= segsize)
			*(memory[0] + addr) = data;
		else
			*(memory[selbnk] + addr) = data;
	}
}

static inline BYTE getmem(WORD addr)
{
	if (selbnk == 0)
		return *(memory[0] + addr);

	if (addr >= segsize)
		return *(memory[0] + addr);
	else
		return *(memory[selbnk] + addr);
}

#endif /* !SIMMEM_INC */
