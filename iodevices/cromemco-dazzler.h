/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2015-2018 by Udo Munk
 *
 * Emulation of a Cromemco DAZZLER S100 board
 *
 * History:
 * 24-APR-15 first version
 * 25-APR-15 fixed a few things, good enough for a BETA release now
 * 27-APR-15 fixed logic bugs with on/off state and thread handling
 * 08-MAY-15 fixed Xlib multithreading problems
 * 26-AUG-15 implemented double buffering to prevent flicker
 * 27-AUG-15 more bug fixes
 * 15-NOV-16 fixed logic bug, display wasn't always clear after
 *           the device is switched off
 * 06-DEC-16 added bus request for the DMA
 * 16-DEC-16 use DMA function for memory access
 * 26-JAN-17 optimization
 * 15-JUL-18 use logging
 */

void cromemco_dazzler_ctl_out(BYTE);
BYTE cromemco_dazzler_flags_in(void);
void cromemco_dazzler_format_out(BYTE);

void cromemco_dazzler_off(void);
