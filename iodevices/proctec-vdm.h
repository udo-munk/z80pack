/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2018 by Udo Munk
 *
 * Emulation of a Processor Technology VDM-1 S100 board
 *
 * History:
 * 28-FEB-17 first version, all software tested with working
 * 21-JUN-17 don't use dma_read(), switches Tarbell ROM off
 * 20-APR-18 avoid thread deadlock on Windows/Cygwin
 * 15-JUL-18 use logging
 */

extern void proctec_vdm_out(BYTE);
extern void proctec_vdm_off(void);
