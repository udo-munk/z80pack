/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module implements memory management for an Intel Intellec MDS-800
 * system
 *
 * History:
 * 03-JUN-2024 first version
 * 14-DEC-2024 added hardware breakpoint support
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simdefs.h"
#ifdef WANT_ICE
#include "simice.h"
#endif
#include "simctl.h"

#ifdef BUS_8080
#include "simglb.h"
#endif

#define BOOT_SIZE	256	/* bootstrap ROM size */
#define MON_SIZE	2048	/* monitor ROM size */

extern BYTE memory[65536], boot_rom[BOOT_SIZE];
extern char *boot_rom_file, *mon_rom_file;
extern bool mon_enabled;

extern void init_memory(void);

/*
 *	Memory access for the CPU cores
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

	if (!mon_enabled || addr < 65536 - MON_SIZE)
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

	if (boot_switch && addr < BOOT_SIZE)
		data = boot_rom[addr];
	else
		data = memory[addr];

#ifdef BUS_8080
	cpu_bus &= ~CPU_M1;
	cpu_bus |= CPU_WO | CPU_MEMR;
#endif

	return data;
}

/*
 *	Memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	if (!mon_enabled || addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE dma_read(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return boot_rom[addr];
	else
		return memory[addr];
}

/*
 *	Direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	if (!mon_enabled || addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE getmem(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return boot_rom[addr];
	else
		return memory[addr];
}

#endif /* !SIMMEM_INC */
