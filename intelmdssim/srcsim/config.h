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
 */

#ifndef CONFIG_INC
#define CONFIG_INC

extern void config(void);

extern int  fp_size;
extern BYTE fp_port;

#endif /* !CONFIG_INC */
