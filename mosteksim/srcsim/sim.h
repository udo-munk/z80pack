/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This is the configuration I'm using for software testing and debugging
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) Created from sim.h from the z80sim source
 *	       directory. Set start-up message for Mostek AID-80F and SYS-80FT
 *	       computers.
 * 27-SEP-2019 (Udo Munk) modified for integration into 1.37
 * 25-APR-2024 (Udo Munk) this was a Z80 machine and we can exclude 8080 now
 */

#ifndef SIM_INC
#define SIM_INC

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU Z80	/* default CPU (Z80 or I8080) */
#define CPU_SPEED 0	/* default CPU speed 0=unlimited */
#define EXCLUDE_I8080	/* this was a Z80 machine */
/*#define ALT_I8080*/	/* use alt. 8080 sim. primarily optimized for size */
/*#define ALT_Z80*/	/* use alt. Z80 sim. primarily optimized for size */
#define UNDOC_INST	/* compile undocumented instrs. (required by ALT_*) */
#ifndef EXCLUDE_Z80
/*#define FAST_BLOCK*/	/* much faster but not accurate Z80 block instr. */
#endif

#define WANT_ICE	/* attach ICE to headless machine */
#ifdef WANT_ICE
#define WANT_TIM	/* count t-states */
#define HISIZE	100	/* number of entries in history */
#define SBSIZE	4	/* number of software breakpoints */
#endif

#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"Mostek AID-80F and SYS-80FT Emulator"
#define USR_REL	"1.1"
#define USR_CPR	"by Mike Douglas"

#endif
