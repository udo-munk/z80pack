/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2024 Thomas Eberhardt
 *
 * This module implements memory management for an Altair 8800 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 02-FEB-2017 initialize ROM with 0xff
 * 13-JUN-2017 added Tarbell bootstrap ROM
 * 16-AUG-2017 overworked memrdr()
 * 07-MAY-2018 added memory configuration needed by apple monitor
 * 11-JUN-2018 fixed bug in Tarbell ROM mapping
 * 21-AUG-2018 improved memory configuration
 * 04-NOV-2019 add functions for direct memory access
 * 31-JUL-2021 allow building machine without frontpanel
 * 29-AUG-2021 new memory configuration sections
 * 14-DEC-2024 added hardware breakpoint support
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simdefs.h"
#ifdef WANT_ICE
#include "simice.h"
#endif

#include "tarbell_fdc.h"

#if defined(FRONTPANEL) || defined(BUS_8080)
#include "simglb.h"
#endif
#ifdef FRONTPANEL
#include "simctl.h"
#include "frontpanel.h"
#endif

#define MAXPAGES	256

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * configuration for memory map(s)
 */
#define MAXMEMMAP	6
#define MAXMEMSECT	15

typedef struct memmap {
	int type;	/* type of memory pages */
	BYTE spage;	/* start page of segment */
	WORD size;	/* size of segment in pages */
	char *rom_file;
} memmap_t;

extern memmap_t memconf[MAXMEMSECT][MAXMEMMAP];
extern WORD _boot_switch[MAXMEMSECT];	/* boot address */

extern BYTE memory[65536], mem_wp;
extern int p_tab[MAXPAGES];

extern void init_memory(void);

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
#ifdef BUS_8080
#ifndef FRONTPANEL
	cpu_bus &= ~CPU_M1;
#endif
	cpu_bus &= ~(CPU_WO | CPU_MEMR);
#endif

#ifdef FRONTPANEL
	if (F_flag) {
		fp_clock++;
		fp_led_address = addr;
		fp_led_data = 0xff;
		fp_sampleData();
		wait_step();
	} else
		cpu_bus &= ~CPU_M1;
#endif

#ifdef WANT_HB
	if (hb_flag && hb_addr == addr && (hb_mode & HB_WRITE))
		hb_trig = HB_WRITE;
#endif

	if (p_tab[addr >> 8] == MEM_RW) {
		memory[addr] = data;
		mem_wp = 0;
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

	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f) {
			data = tarbell_rom[addr];
		} else {
			if (p_tab[addr >> 8] != MEM_NONE)
				data = memory[addr];
			else
				data = 0xff;
			tarbell_rom_active = false;
		}
	} else {
		if (p_tab[addr >> 8] != MEM_NONE)
			data = memory[addr];
		else
			data = 0xff;
	}

#ifdef BUS_8080
#ifndef FRONTPANEL
	cpu_bus &= ~CPU_M1;
#endif
	cpu_bus |= CPU_WO | CPU_MEMR;
#endif

#ifdef FRONTPANEL
	if (F_flag) {
		fp_clock++;
		fp_led_address = addr;
		fp_led_data = data;
		fp_sampleData();
		wait_step();
	} else
		cpu_bus &= ~CPU_M1;
#endif

	return data;
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline BYTE dma_read(WORD addr)
{
	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f)
			return tarbell_rom[addr];
		else
			tarbell_rom_active = false;
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return memory[addr];
	else
		return 0xff;
}

static inline void dma_write(WORD addr, BYTE data)
{
	if (p_tab[addr >> 8] == MEM_RW)
		memory[addr] = data;
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline BYTE getmem(WORD addr)
{
	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f)
			return tarbell_rom[addr];
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return memory[addr];
	else
		return 0xff;
}

static inline void putmem(WORD addr, BYTE data)
{
	memory[addr] = data;
}

/*
 * memory read for frontpanel logic
 */
static inline BYTE fp_read(WORD addr)
{
	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f)
			return tarbell_rom[addr];
		else
			tarbell_rom_active = false;
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return memory[addr];
	else
		return 0xff;
}

#endif /* !SIMMEM_INC */
