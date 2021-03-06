/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019-2021 by Udo Munk
 *
 * Emulation of a RTC to provide date and time informations.
 * This doesn't emulate a specific RTC card or chip, in 1980
 * there was no standard yet for such things.
 *
 * History:
 * 24-OCT-19 moved out of the cpmsim machine
 * 30-JUN-21 clock read now returns time format instead of last command
 */

extern BYTE clkc_in(void), clkd_in(void);
extern void clkc_out(BYTE), clkd_out(BYTE data);
