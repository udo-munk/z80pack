/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Configuration for an Intel Intellec MDS-800 system
 *
 * History:
 * 03-JUN-2024 first version
 */

#ifndef SIM_INC
#define SIM_INC

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU I8080	/* default CPU (Z80 or I8080) */
/*#define AMD8080*/	/* AMD 8080 instead of Intel 8080 */
#define CPU_SPEED 2	/* default CPU speed 0=unlimited */
#define EXCLUDE_Z80	/* Intel Intellec MDS-800 was an 8080 machine */
/*#define ALT_I8080*/	/* use alt. 8080 sim. primarily optimized for size */
/*#define ALT_Z80*/	/* use alt. Z80 sim. primarily optimized for size */
#define UNDOC_INST	/* compile undocumented instrs. (required by ALT_*) */
#ifndef EXCLUDE_Z80
/*#define FAST_BLOCK*/	/* much faster but not accurate Z80 block instr. */
#endif

/*#define WANT_ICE*/	/* attach ICE to headless machine */
#ifdef WANT_ICE
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */
#endif

#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */

#define HAS_ISBC201	/* has iSBC 201 single density diskette controller */
#define HAS_ISBC202	/* has iSBC 202 double density diskette controller */
#define HAS_ISBC206	/* has iSBC 206 hard disk controller */

#define NUMNSOC 1	/* one TCP/IP socket for TTY */
#define TCPASYNC	/* use async I/O if possible */
#define SERVERPORT 4010	/* first TCP/IP server port used */
#define NUMUSOC 1	/* one UNIX socket for PTR/PTP */

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"Intel Intellec MDS-800 Simulation"
#define USR_REL	"1.0"
#define USR_CPR	"Copyright (C) 2008-2024 by Udo Munk & " \
		"2024 by Thomas Eberhardt"

#endif /* !SIM_INC */
