/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 by Udo Munk
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
 */

#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif

#define MAXSEG 7		/* max. number of 64KB memory banks */

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

extern void init_memory(void), init_rom(void);
extern int wait_step(void);
extern void wait_int_step(void);

extern BYTE *memory[MAXSEG];
extern int selbnk, common, bankio;

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

static inline BYTE memrdr(WORD addr)
{
	cpu_bus |= CPU_WO | CPU_MEMR;

#ifdef FRONTPANEL
	fp_clock++;
	fp_led_address = addr;
	fp_led_data = *(memory[selbnk] + addr);
	fp_sampleData();
	wait_step();

	return(fp_led_data);
#else
	return(*(memory[selbnk] + addr));
#endif
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline BYTE dma_read(WORD addr)
{
	return(*(memory[selbnk] + addr));
}

static inline void dma_write(WORD addr, BYTE data)
{
	*(memory[selbnk] + addr) = data;
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline BYTE getmem(WORD addr)
{
	return(*(memory[selbnk] + addr));
}

static inline void putmem(WORD addr, BYTE data)
{
	*(memory[selbnk] + addr) = data;
}

/*
 * return memory base for the simulation frame
 */
static inline BYTE *mem_base(void)
{
	return(memory[0]);
}
