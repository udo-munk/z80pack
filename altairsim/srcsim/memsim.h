/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
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
 */

#ifndef MEMSIM_INC
#define MEMSIM_INC

#ifdef FRONTPANEL
#include "frontpanel.h"
#endif

extern void init_memory(void);
extern int wait_step(void);
extern void wait_int_step(void);
extern BYTE memory[], mem_wp;
extern int p_tab[];
extern BYTE tarbell_rom[];
extern int tarbell_rom_enabled, tarbell_rom_active;

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * configuration for memory map(s)
 */
#define MAXMEMMAP	6
#define MAXMEMSECT	15

struct memmap {
	int type;	/* type of memory pages */
	BYTE spage;	/* start page of segment */
	WORD size;	/* size of segment in pages */
	char *rom_file;
};

extern struct memmap memconf[MAXMEMSECT][MAXMEMMAP];
extern WORD _boot_switch[MAXMEMSECT];	/* boot address */

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
#ifdef BUS_8080
	cpu_bus &= ~(CPU_WO | CPU_MEMR);
#endif

#ifdef FRONTPANEL
	if (fp_enabled) {
		fp_clock++;
		fp_led_address = addr;
		fp_led_data = 0xff;
		fp_sampleData();
		wait_step();
	}
#endif

	if (p_tab[addr >> 8] == MEM_RW) {
		memory[addr] = data;
		mem_wp = 0;
	}
}

static inline BYTE memrdr(WORD addr)
{
	register BYTE data;

	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f) {
			data = tarbell_rom[addr];
		} else {
			if (p_tab[addr >> 8] != MEM_NONE)
				data = memory[addr];
			else
				data = 0xff;
			tarbell_rom_active = 0;
		}
	} else {
		if (p_tab[addr >> 8] != MEM_NONE)
			data = memory[addr];
		else
			data = 0xff;
	}

#ifdef BUS_8080
	cpu_bus |= CPU_WO | CPU_MEMR;
#endif

#ifdef FRONTPANEL
	if (fp_enabled) {
		fp_clock++;
		fp_led_address = addr;
		fp_led_data = data;
		fp_sampleData();
		wait_step();
	}
#endif

	return (data);
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline BYTE dma_read(WORD addr)
{
	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f)
			return (tarbell_rom[addr]);
		else
			tarbell_rom_active = 0;
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return (memory[addr]);
	else
		return (0xff);
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
			return (tarbell_rom[addr]);
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return (memory[addr]);
	else
		return (0xff);
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
			return (tarbell_rom[addr]);
		else
			tarbell_rom_active = 0;
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return (memory[addr]);
	else
		return (0xff);
}

#endif
