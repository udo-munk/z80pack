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
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simctl.h"
#include "simfun.h"

#define BOOT_SIZE	256	/* bootstrap ROM size */
#define MON_SIZE	2048	/* monitor ROM size */

extern void init_memory(void);

extern BYTE memory[65536], boot_rom[BOOT_SIZE];
extern char *boot_rom_file, *mon_rom_file;
extern int mon_enabled;

/*
 *	Memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if (!mon_enabled || addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return boot_rom[addr];
	else
		return memory[addr];
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
