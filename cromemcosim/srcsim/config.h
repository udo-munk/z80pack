/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
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
 * 29-JUL-21 add boot config for machine without frontpanel
 * 30-AUG-21 new memory configuration sections
 */

extern void config(void);

extern int  fp_size;
extern BYTE fp_port;
