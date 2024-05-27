/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the configuration for a Raspberry Pico (W) board
 */

#ifndef SIM_INC
#define SIM_INC

#define PICO 1		/* board we use, 0 for Pico 1 for Pico W */

#define DEF_CPU Z80	/* default CPU (Z80 or I8080) */
//#define EXCLUDE_I8080	/* don't include 8080, for now we want both */
#define CPU_SPEED 4	/* CPU speed 0=unlimited */
/*#define ALT_I8080*/	/* use alt. 8080 sim. primarily optimized for size */
/*#define ALT_Z80*/	/* use alt. Z80 sim. primarily optimized for size */
#define UNDOC_INST	/* compile undocumented instructions */
/*#define FAST_BLOCK*/	/* much faster but not accurate Z80 block instr. */
#define BAREMETAL	/* set up the simulator core for bare metal use */

/*#define WANT_ICE*/	/* attach ICE to headless machine */
#define WANT_TIM	/* count t-states */
#define HISIZE	100	/* number of entries in history */
#define SBSIZE	4	/* number of software breakpoints */

extern void sleep_us(unsigned long long);
#define SLEEP_US(t)	sleep_us(t)
extern void sleep_ms(unsigned long);
#define SLEEP_MS(t)	sleep_ms(t)

#define USR_COM "Raspberry Pi Pico Z80/8080 emulator"
#define USR_REL "1.1"
#define USR_CPR "Copyright (C) 2024 by Udo Munk & Thomas Eberhardt"
 
#include "simcore.h"

#endif
