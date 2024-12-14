/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2022 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2024 Thomas Eberhardt
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 03-FEB-2017 added ROM initialization
 * 18-MAY-2018 optimization
 * 18-JUL-2018 use logging
 * 01-OCT-2019 optimization
 * 04-NOV-2019 add functions for direct memory access
 * 17-JUN-2021 allow building machine without frontpanel
 * 30-AUG-2021 new memory configuration sections
 * 02-SEP-2021 implement banked ROM
 * 14-DEC-2024 added hardware breakpoint support
 */

#ifndef SIMMEM_INC
#define SIMMEM_INC

#include "sim.h"
#include "simdefs.h"
#ifdef WANT_ICE
#include "simice.h"
#endif

#include "cromemco-fdc.h"

#if defined(FRONTPANEL) || defined(BUS_8080)
#include "simglb.h"
#endif
#ifdef FRONTPANEL
#include "simctl.h"
#include "frontpanel.h"
#endif

#define MAXPAGES	256
#define MAXSEG		7	/* max. number of 64KB memory banks */
#define SEGSIZ		65536	/* size of the memory segments, 64 KBytes */

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * configuration for memory map(s)
 */
#define MAXMEMMAP	6
#define MAXMEMSECT	15

#define BANKED_ROM_MSG "FDC Banked ROM enabled"

struct memmap {
	int type;	/* type of memory pages */
	BYTE spage;	/* start page of segment */
	WORD size;	/* size of segment in pages */
	char *rom_file;
};

extern struct memmap memconf[MAXMEMSECT][MAXMEMMAP];
extern WORD boot_switch[MAXMEMSECT];	/* boot address */

extern BYTE *memory[MAXSEG];
extern int selbnk, common, bankio, num_banks;

extern int p_tab[MAXPAGES];		/* 256 pages of 256 bytes */

/* return page to RAM pool */
#define MEM_RELEASE(page) 	p_tab[(page)] = _p_tab[(page)]
/* reserve page as banked ROM */
#define MEM_ROM_BANK_ON(page)	p_tab[(page)] = MEM_RO
/* reserve page as RAM */
#define MEM_RESERVE_RAM(page)	p_tab[(page)] = MEM_RW
/* reserve page as ROM */
#define MEM_RESERVE_ROM(page)	p_tab[(page)] = MEM_RO

extern void init_memory(void);
extern void reset_fdc_rom_map(void);

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	register int i;

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
		fp_led_data = data;
		fp_sampleData();
		wait_step();
	} else
		cpu_bus &= ~CPU_M1;
#endif

#ifdef WANT_HB
	if (hb_flag && hb_addr == addr && (hb_mode & HB_WRITE))
		hb_trig = HB_WRITE;
#endif

	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return;
	} else if (selbnk || p_tab[addr >> 8] == MEM_RW) {

		if (!common) {
			*(memory[selbnk] + addr) = data;
		} else {
			if (addr < 32768)
				*(memory[selbnk] + addr) = data;
			else {
				for (i = 0; i < MAXSEG; i++)
					*(memory[i] + addr) = data;
			}
		}
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

	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		data = *(fdc_banked_rom + addr - 0xC000);
	} else if (selbnk || p_tab[addr >> 8] != MEM_NONE) {
		data = *(memory[selbnk] + addr);
	} else {
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
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return *(fdc_banked_rom + addr - 0xC000);
	} else if (selbnk || p_tab[addr >> 8] != MEM_NONE) {
		return *(memory[selbnk] + addr);
	} else {
		return 0xff;
	}
}

static inline void dma_write(WORD addr, BYTE data)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return;
	} else if (selbnk || p_tab[addr >> 8] == MEM_RW) {
		*(memory[selbnk] + addr) = data;
	}
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline BYTE getmem(WORD addr)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return *(fdc_banked_rom + addr - 0xC000);
	} else if (selbnk || p_tab[addr >> 8] != MEM_NONE) {
		return *(memory[selbnk] + addr);
	} else {
		return 0xff;
	}
}

static inline void putmem(WORD addr, BYTE data)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		*(fdc_banked_rom + addr - 0xC000) = data;
	} else {
		*(memory[selbnk] + addr) = data;
	}
}

#endif /* !SIMMEM_INC */
