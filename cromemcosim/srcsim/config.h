/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-DEC-16 dummy, no configuration implemented yet
 * 02-JAN-17 front panel framerate configurable
 * 27-JAN-17 initial window size of the front panel configurable
 */

extern void config(void);

extern int fp_size;
