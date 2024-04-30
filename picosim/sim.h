/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the configuration for a Raspberry Pico (W) board
 */

#define PICO 1		/* board we use, 0 for Pico 1 for Pico W */

#define DEF_CPU Z80	/* CPU (Z80 or I8080) */
#define CPU_SPEED 4	/* CPU speed 0=unlimited */
#define UNDOC_INST	/* compile undocumented instructions */
/*#define CORE_LOG*/	/* don't use LOG() logging in core simulator */
#define EXCLUDE_I8080	/* don't include 8080 emulation support */

extern void sleep_ms();
#define SLEEP_MS(t)	sleep_ms(t)

#define USR_COM "Raspberry Pi Pico Z80 Simulation"
#define USR_REL "1.0"
#define USR_CPR "Copyright (C) 2024 by Udo Munk"
 
#include "simcore.h"
