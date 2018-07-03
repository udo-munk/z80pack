/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-16 stuff moved to here and implemented as inline functions
 * 03-FEB-17 added ROM initialisation
 * 18-MAY-18 optimization
 */

#define MAXSEG 7		/* max. number of 64KB memory banks */

extern void init_memory(void), init_rom(void);
extern void wait_step(void), wait_int_step(void);

extern BYTE *memory[MAXSEG];
extern int selbnk, common, bankio;

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	register int i;

	cpu_bus &= ~(CPU_WO | CPU_MEMR);

	fp_clock++;
	fp_led_address = addr;
	fp_led_data = data;
	fp_sampleData();
	wait_step();

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
	register BYTE data = *(memory[selbnk] + addr);

	cpu_bus |= CPU_WO | CPU_MEMR;

	fp_clock++;
	fp_led_address = addr;
	fp_led_data = data;
	fp_sampleData();
	wait_step();

	return(data);
}

/*
 * memory access for DMA devices
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
 * return memory base for the simulation frame
 */
static inline BYTE *mem_base(void)
{
	return(memory[0]);
}
