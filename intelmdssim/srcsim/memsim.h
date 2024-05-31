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
 */

#ifndef MEMSIM_INC
#define MEMSIM_INC

#define BOOT_SIZE	256	/* bootstrap ROM size */
#define MON_SIZE	2048	/* monitor ROM size */

extern void init_memory(void);
extern int wait_step(void);
extern void wait_int_step(void);

extern BYTE memory[], boot_rom[];
extern BYTE boot_switch;
extern int mon_enabled;

/*
 * memory access for the CPU cores
 */
static inline void memwrt(WORD addr, BYTE data)
{
	if (mon_enabled && addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE memrdr(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return (boot_rom[addr]);
	else
		return (memory[addr]);
}

/*
 * memory access for DMA devices which request bus from CPU
 */
static inline void dma_write(WORD addr, BYTE data)
{
	if (mon_enabled && addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE dma_read(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return (boot_rom[addr]);
	else
		return (memory[addr]);
}

/*
 * direct memory access for simulation frame, video logic, etc.
 */
static inline void putmem(WORD addr, BYTE data)
{
	if (mon_enabled && addr < 65536 - MON_SIZE)
		memory[addr] = data;
}

static inline BYTE getmem(WORD addr)
{
	if (boot_switch && addr < BOOT_SIZE)
		return (boot_rom[addr]);
	else
		return (memory[addr]);
}

#endif /* !MEMSIM_INC */
