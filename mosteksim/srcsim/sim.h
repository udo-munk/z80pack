/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This is the configuration I'm using for software testing and debugging
 *
 * History:
 * 15-SEP-19 (Mike Douglas) Created from sim.h from the z80sim source
 *		directory. Set start-up message for Mostek AID-80F and SYS-80FT
 *		computers.
 * 27_SEP-19 (Udo Munk) modified for integration into 1.37
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU Z80	/* default CPU (Z80 or I8080) */
#define CPU_SPEED 0	/* default CPU speed 0=unlimited */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
/*#define WANT_FASTB*/	/* much faster but not accurate Z80 block instr. */
#define CORE_LOG	/* use LOG() logging in core simulator */

#define WANT_TIM	/* count t-states */
#define HISIZE	100	/* number of entries in history */
#define SBSIZE	4	/* number of software breakpoints */

/*#define HAS_DISKS*/	/* not using standard disk define */
/*#define HAS_CONFIG*/  /* not using standard config define */

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"\nMostek AID-80F and SYS-80FT Emulator"
#define USR_REL	"1.1"
#define USR_CPR	"by Mike Douglas"

#include "simcore.h"
