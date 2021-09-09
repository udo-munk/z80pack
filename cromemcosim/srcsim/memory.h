/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-16 stuff moved to here and implemented as inline functions
 * 03-FEB-17 added ROM initialisation
 * 18-MAY-18 optimization
 * 18-JUL-18 use logging
 * 01-OCT-19 optimization
 * 04-NOV-19 add functions for direct memory access
 * 17-JUN-21 allow building machine without frontpanel
 * 30-AUG-21 new memory configuration sections
 * 02-SEP-21 implement banked ROM
 */

#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif

#define MAXSEG 7		/* max. number of 64KB memory banks */
#define SEGSIZ 65536	/* size of the memory segments, 64 KBytes */

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * configuration for memory map(s)
 */
#define MAXMEMMAP	6
#define MAXMEMSECT	15
#define MAXPAGES 256

struct memmap {
	int type;	/* type of memory pages */
	BYTE spage;	/* start page of segment */
	WORD size;	/* size of segment in pages */
	char *rom_file;
};

extern struct memmap memconf[MAXMEMSECT][MAXMEMMAP];
extern WORD boot_switch[MAXMEMSECT];	/* boot address */

extern void init_memory(void);
extern int wait_step(void);
extern void wait_int_step(void);

extern BYTE *memory[MAXSEG];
extern int selbnk, common, bankio;

extern BYTE fdc_banked_rom[]; /* 8K of ROM (from 64FDC) to support RDOS 3 */
extern int fdc_rom_active;

extern int p_tab[MAXPAGES];		/* 256 pages of 256 bytes */

/* return page to RAM pool */
#define MEM_RELEASE(page) 	p_tab[(page)] = _p_tab[(page)]
/* reserve page as banked ROM */
#define MEM_ROM_BANK_ON(page)	p_tab[(page)] = MEM_RO
/* reserve page as RAM */
#define MEM_RESERVE_RAM(page)	p_tab[(page)] = MEM_RW
/* reserve page as ROM */
#define MEM_RESERVE_ROM(page)	p_tab[(page)] = MEM_RO

extern void reset_fdc_rom_map(void);

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	register int i;

	cpu_bus &= ~(CPU_WO | CPU_MEMR);

#ifdef FRONTPANEL
	fp_clock++;
	fp_led_address = addr;
	fp_led_data = data;
	fp_sampleData();
	wait_step();
#endif

	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return;
	} else if(selbnk || p_tab[addr >> 8] == MEM_RW) {

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
	cpu_bus |= CPU_WO | CPU_MEMR;

#ifdef FRONTPANEL
	fp_clock++;
	fp_led_address = addr;

	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		fp_led_data = *(fdc_banked_rom + addr - 0xC000);
	} else if(selbnk || p_tab[addr >> 8] != MEM_NONE) {
		fp_led_data = *(memory[selbnk] + addr);
	} else {
		fp_led_data = 0xff;
	}
	fp_sampleData();
	wait_step();

	return(fp_led_data);
#else
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return(*(fdc_banked_rom + addr - 0xC000));
	} else if(selbnk || p_tab[addr >> 8] != MEM_NONE) {
		return(*(memory[selbnk] + addr));
	} else {
		return(0xff);
	}
#endif
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline BYTE dma_read(WORD addr)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return(*(fdc_banked_rom + addr - 0xC000));
	} else if(selbnk || p_tab[addr >> 8] != MEM_NONE) {
		return(*(memory[selbnk] + addr));
	} else {
		return(0xff);
	}
}

static inline void dma_write(WORD addr, BYTE data)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return;
	} else if(selbnk || p_tab[addr >> 8] == MEM_RW) {
		*(memory[selbnk] + addr) = data;
	}
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline BYTE getmem(WORD addr)
{
	if (fdc_rom_active && (addr >> 13) == 0x6) { /* Covers C000 to DFFF */
		return(*(fdc_banked_rom + addr - 0xC000));
	} else if(selbnk || p_tab[addr >> 8] != MEM_NONE) {
		return(*(memory[selbnk] + addr));
	} else {
		return(0xff);
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
