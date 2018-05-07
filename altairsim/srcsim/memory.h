/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * This module implements memory management for an Altair 8800 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 02-FEB-2017 initialise ROM with 0xff
 * 13-JUN-2017 added Tarbell bootstrap ROM
 * 16-AUG-2017 overworked memrdr()
 * 07-MAY-2018 added memory configuratione needed by apple monitor
 */

extern void init_memory(void), init_rom(void);
extern void  wait_step(void), wait_int_step(void);
extern BYTE memory[], mem_wp;
extern int p_tab[];
extern BYTE tarbell_rom[];
extern int tarbell_rom_enabled, tarbell_rom_active;

#define MEM_RW		0	/* memory is readable and writeable */
#define MEM_RO		1	/* memory is read-only */
#define MEM_WPROT	2	/* memory is write protected */
#define MEM_NONE	3	/* no memory available */

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	cpu_bus &= ~(CPU_WO | CPU_MEMR);

	fp_clock++;
	fp_led_address = addr;
	fp_led_data = 0xff;
	fp_sampleData();
	wait_step();

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
		}
	} else {
		if (p_tab[addr >> 8] != MEM_NONE)
			data = memory[addr];
		else
			data = 0xff;
	}

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
	if (tarbell_rom_active && tarbell_rom_enabled) {
		if (addr <= 0x001f)
			return(tarbell_rom[addr]);
		else
			tarbell_rom_active = 0;
	}

	if (p_tab[addr >> 8] != MEM_NONE)
		return(memory[addr]);
	else
		return(0xff);
}

static inline void dma_write(WORD addr, BYTE data)
{
	if (p_tab[addr >> 8] == MEM_RW)
		memory[addr] = data;
}

/*
 * return memory base pointer for the simulation frame
 */
static inline BYTE *mem_base(void)
{
	return(&memory[0]);
}
