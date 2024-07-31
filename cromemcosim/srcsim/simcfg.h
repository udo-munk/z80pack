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
 * 20-DEC-2016 dummy, no configuration implemented yet
 * 02-JAN-2017 front panel framerate configurable
 * 27-JAN-2017 initial window size of the front panel configurable
 * 18-JUL-2018 use logging
 * 22-JAN-2021 added option for config file
 * 17-JUN-2021 allow building machine without frontpanel
 * 29-JUL-2021 add boot config for machine without frontpanel
 * 30-AUG-2021 new memory configuration sections
 */

#ifndef SIMCFG_INC
#define SIMCFG_INC

#include "sim.h"
#include "simdefs.h"

extern int  fp_size;
extern BYTE fp_port;
extern int  ns_port;

extern void config(void);

#endif /* !SIMCFG_INC */
