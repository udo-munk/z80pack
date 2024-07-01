/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 03-JUN-2024 first version
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 */

#ifndef SIMCFG_INC
#define SIMCFG_INC

extern int fp_size;

extern void config(void);

#endif /* !SIMCFG_INC */
