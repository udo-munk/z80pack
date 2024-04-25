/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2022 by Udo Munk
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU Z80	/* default CPU (Z80 or I8080) */
#define CPU_SPEED 0	/* default CPU speed 0=unlimited */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
#define WANT_FASTB	/* much faster but not accurate Z80 block instr. */
#define CORE_LOG	/* use LOG() logging in core simulator */

/*#define WANT_ICE*/	/* attach ICE to machine */
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */

#define HAS_DISKS	/* uses disk images */
/*#define HAS_CONFIG*/	/* has no configuration file */

#define PIPES		/* use named pipes for auxiliary device */
#define NETWORKING	/* TCP/IP networked serial ports */
#define NUMSOC	4	/* number of server sockets */
#define TCPASYNC	/* tcp/ip server can use async I/O */
/*#define CNETDEBUG*/	/* client network protocol debugger */
/*#define SNETDEBUG*/	/* server network protocol debugger */

/*
 * forking and pipes are working now with Cygwin in 2014,
 * but SIGIO on BSD sockets is not
 */
#ifdef __CYGWIN__
/*#undef PIPES*/	/* forking and pipes was not working correct */
#undef TCPASYNC		/* SIGIO on BSD sockets not working */
#endif

extern void do_sleep_ms(int);
#define SLEEP_MS(t)	do_sleep_ms(t)

/*
 *	Structure for the disk images
 */
struct dskdef {
	const char *fn;			/* filename */
	int *fd;			/* file descriptor */
	unsigned int tracks;		/* number of tracks */
	unsigned int sectors;		/* number of sectors */
};

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */
/*
#define USR_COM	"XYZ-System Simulation"
#define USR_REL	"x.y"
#define USR_CPR	"Copyright (C) 20xx by XYZ"
*/

#include "simcore.h"
