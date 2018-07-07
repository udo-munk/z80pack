/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * This module implements memory management for an IMSAI 8080 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 30-DEC-2016 implemented 1 KB page table and setup for that
 * 26-JAN-2017 initialise ROM with 0xff
 * 04-JUL-2018 optimization
 */

extern void init_memory(void), reset_memory(void), init_rom(void);
extern void wait_step(void), wait_int_step(void);
extern BYTE memory[];
extern int p_tab[];

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

extern void ctrl_port_out(BYTE);
extern BYTE ctrl_port_in(void);

extern BYTE *rdrvec[];
extern BYTE *wrtvec[];

extern int cyclecount;
extern void groupswap();

#ifdef HAS_BANKED_ROM
#undef MEMORY_WRITE
#define MEMORY_WRITE(addr)	_MEMMAPPED(addr)
#define _MEMWRTTHRU(addr) 	*(wrtvec[(addr)>>10] + ((addr)&0x03ff))
#define _MEMMAPPED(addr) 	*(rdrvec[(addr)>>10] + ((addr)&0x03ff))
#else
#define _MEMWRTTHRU(addr) 	_MEMDIRECT(addr)
#define _MEMMAPPED(addr) 	_MEMDIRECT(addr)
#endif //HAS_BANKED_ROM
#define _MEMDIRECT(addr) 	memory[(addr)]

#define _GROUPINIT	0x00	// Power-on default
#define _GROUP0 	0x40	// 2K ROM @ 0000-07FF
#define _GROUP1 	0x80	// 2K ROM @ D800-DFFF, 256 byte RAM @ DOOO-DOFF (actually 1K RAM @ DOOO-D3FF)

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if (p_tab[addr >> 10] == MEM_RW)
		_MEMWRTTHRU(addr) = data;

	cpu_bus &= ~(CPU_WO | CPU_MEMR);

	fp_clock++;
	fp_led_address = addr;
	fp_led_data = data;
	fp_sampleData();
	wait_step();
}

static inline BYTE memrdr(WORD addr)
{
	register BYTE data = _MEMMAPPED(addr);
	if(cyclecount && --cyclecount == 0) 
		groupswap();

	cpu_bus |= CPU_WO | CPU_MEMR;

	fp_clock++;
	fp_led_address = addr;
	if (p_tab[addr >> 10] != MEM_NONE)
		fp_led_data = data;
	else
		fp_led_data = 0xff;
	fp_sampleData();
	wait_step();

	return(fp_led_data);
}

/*
 * memory access for DMA devices
 */
static inline BYTE dma_read(WORD addr)
{
	if (p_tab[addr >> 10] != MEM_NONE)
		return(_MEMDIRECT(addr));
	else
		return(0xff);
}

static inline void dma_write(WORD addr, BYTE data)
{
	if (p_tab[addr >> 10] == MEM_RW)
		_MEMDIRECT(addr) = data;
}

/*
 * return memory base pointer for the simulation frame
 */
static inline BYTE *mem_base(void)
{
	return(&memory[0]);
}
