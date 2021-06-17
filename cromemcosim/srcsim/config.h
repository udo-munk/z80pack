/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 by Udo Munk
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-DEC-16 dummy, no configuration implemented yet
 * 02-JAN-17 front panel framerate configurable
 * 27-JAN-17 initial window size of the front panel configurable
 * 18-JUL-18 use logging
 * 22-JAN-21 added option for config file
 * 17-JUN-21 allow building machine without frontpanel
 */

extern void config(void);

extern int fp_size;
