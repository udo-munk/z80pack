/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * This module implements memory management for an IMSAI 8080 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 30-DEC-2016 implemented 1 KB page table and setup for that
 * 26-JAN-2017 initialise ROM with 0xff
 * 04-JUL-2018 optimization
 * 07-JUL-2018 implemended banked ROM/RAM
 * 12-JUL-2018 use logging
 * 18-JUL-2019 bug fix so that fp shows mapped memory contents
 * 18-OCT-2019 add MMU and memory banks
 * 04-NOV-2019 add functions for direct memory access
 * 06-NOV-2019 add function for frontpanel memory write
 * 14-AUG-2020 allow building machine without frontpanel
 * 20-JUL-2021 log banked memory
 * 29-AUG-2021 new memory configuration sections
 */

#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif

extern void init_memory(void), reset_memory(void);
extern void groupswap(void);
extern int wait_step(void);
extern void wait_int_step(void);
extern BYTE memory[], *banks[];
extern int p_tab[];
extern int _p_tab[];
extern int selbnk;

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * configuration for memory map(s)
 */
#define MAXMEMMAP	6
#define MAXMEMSECT	7

#define BANKED_ROM_MSG "MPU-B Banked ROM/RAM enabled"

struct memmap {
	int type;	/* type of memory pages */
	BYTE spage;	/* start page of segment */
	WORD size;	/* size of segment in pages */
	char *rom_file;
};

extern struct memmap memconf[MAXMEMSECT][MAXMEMMAP];
extern WORD boot_switch[MAXMEMSECT];	/* boot address */

#define MAXSEG		8	/* max number of memory segments */
#define SEGSIZ		49152	/* size of the memory segments, 48 KBytes */

extern void ctrl_port_out(BYTE);
extern BYTE ctrl_port_in(void);

extern BYTE *rdrvec[];
extern BYTE *wrtvec[];

extern int cyclecount;

#ifdef HAS_BANKED_ROM
#define _MEMWRTTHRU(addr) 	*(wrtvec[(addr) >> 8] + ((addr) & 0x0ff))
#define _MEMMAPPED(addr) 	*(rdrvec[(addr) >> 8] + ((addr) & 0x0ff))
#else
#define _MEMWRTTHRU(addr) 	_MEMDIRECT(addr)
#define _MEMMAPPED(addr) 	_MEMDIRECT(addr)
#endif
#define _MEMDIRECT(addr) 	memory[(addr)]

#define _GROUPINIT	0x00	/* Power-on default */
#define _GROUP0 	0x40	/* 2K ROM @ 0000-07FF */
#define _GROUP1 	0x80	/* 2K ROM @ D800-DFFF, 256 byte RAM @ DOOO-DOFF
				   (actually 1K RAM @ DOOO-D3FF) */

/* return page to RAM pool */
#define MEM_RELEASE(page) 	p_tab[(page)] = _p_tab[(page)]
/* reserve page as banked ROM */
#define MEM_ROM_BANK_ON(page)	p_tab[(page)] = MEM_RO
/* reserve page as RAM */
#define MEM_RESERVE_RAM(page)	p_tab[(page)] = MEM_RW
/* reserve page as ROM */
#define MEM_RESERVE_ROM(page)	p_tab[(page)] = MEM_RO

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] == MEM_RW)
			_MEMWRTTHRU(addr) = data;
	} else {
		*(banks[selbnk] + addr) = data;
	}

#ifdef BUS_8080
	cpu_bus &= ~(CPU_WO | CPU_MEMR);
#endif

#ifdef FRONTPANEL
	fp_clock++;
	fp_led_address = addr;
	fp_led_data = data;
	fp_sampleData();
	wait_step();
#endif
}

static inline BYTE memrdr(WORD addr)
{
	BYTE data;

#ifdef BUS_8080
	cpu_bus |= CPU_WO | CPU_MEMR;
#endif

#ifdef FRONTPANEL
	fp_clock++;
	fp_led_address = addr;
#endif

	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] != MEM_NONE) {
			data = _MEMMAPPED(addr);
#ifdef FRONTPANEL
			fp_led_data = data;
#endif
		} else {
			data = 0xff;
#ifdef FRONTPANEL
			fp_led_data = 0xff;
#endif
		}
	} else {
		data = *(banks[selbnk] + addr);
#ifdef FRONTPANEL
		fp_led_data = data;
#endif
	}

#ifdef FRONTPANEL
	fp_sampleData();
	wait_step();
#endif

	if(cyclecount && --cyclecount == 0) 
		groupswap();

	return(data);
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline BYTE dma_read(WORD addr)
{
	bus_request = 1;
#if 0
	/* updating the LED's slows down too much */
	fp_clock++;
	fp_sampleData();
#endif
	bus_request = 0;

	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] != MEM_NONE)
			return(_MEMMAPPED(addr));
		else
			return(0xff);
	} else {
		return(*(banks[selbnk] + addr));
	}
}

static inline void dma_write(WORD addr, BYTE data)
{
	bus_request = 1;
#if 0
	/* updating the LED's slows down too much */
	fp_clock++;
	fp_sampleData();
#endif
	bus_request = 0;

	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] == MEM_RW)
			_MEMDIRECT(addr) = data;
	} else {
		 *(banks[selbnk] + addr) = data;
	}
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline BYTE getmem(WORD addr)
{
	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] != MEM_NONE)
			return(_MEMMAPPED(addr));
		else
			return(0xff);
	} else {
		return(*(banks[selbnk] + addr));
	}
}

static inline void putmem(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		_MEMMAPPED(addr) = data;
	} else {
		*(banks[selbnk] + addr) = data;
	}
}

/*
 * memory write for frontpanel logic
 */
static inline void fp_write(WORD addr, BYTE data)
{
	if ((selbnk == 0) || (addr >= SEGSIZ)) {
		if (p_tab[addr >> 8] == MEM_RW)
			_MEMDIRECT(addr) = data;
	} else {
		 *(banks[selbnk] + addr) = data;
	}
}
