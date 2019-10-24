/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019 by Udo Munk
 *
 * Emulation of a RTC to provide date and time informations.
 * This doesn't emulate a specific RTC card or chip, in 1980
 * there was no standard yet for such things.
 *
 * History:
 * 24-OCT-19 moved out of the cpmsim machine
 */

extern BYTE clkc_in(void), clkd_in(void);
extern void clkc_out(BYTE), clkd_out(BYTE data);
