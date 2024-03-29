/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
 *
 * Configuration for an IMSAI 8080 system
 *
 * History:
 * 20-OCT-08 first version finished
 * 02-MAR-14 source cleanup and improvements
 * 23-MAR-14 enabled interrupts, 10ms timer added to iosim
 * xx-JUN-14 added default CPU define
 * 19-JUL-14 added typedef for signed 16bit
 * 09-MAY-15 added Cromemco DAZZLER to the machine
 * 30-AUG-16 implemented memwrt as function to support ROM
 * 01-DEC-16 implemented memrdr to separate memory from CPU
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 12-JAN-17 improved configuration and front panel LED timing, VIO emulation
 * 10-APR-18 trap CPU on unsupported bus data during interrupt
 * 14-JUL-18 integrate webfrontend
 * 12-JUL-19 implemented second SIO
 * 14-AUG-20 allow building machine without frontpanel
 * 07-AUG-21 add APU emulation
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define CPU_SPEED 2	/* default CPU speed */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
/*#define WANT_FASTM*/	/* much faster but not accurate Z80 block moves */
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */
#define FRONTPANEL	/* emulate a machines frontpanel */
#define BUS_8080	/* emulate 8080 bus status for front panel */

#define UNIX_TERMINAL	/* uses a UNIX terminal emulation */
#define HAS_DAZZLER	/* has simulated I/O for Cromemeco Dazzler */
/*#define HAS_CYCLOPS*/	/* has simulated I/O for Cromemeco 88 CCC/ACC Cyclops Camera */

#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */
#define HAS_BANKED_ROM	/* emulate IMSAI MPU-B banked ROM & RAM */

/*#define HAS_DISKMANAGER*/	/* uses file based disk map for disks[] */
/*#define HAS_NETSERVER*/	/* uses civet webserver to present a web based frontend */
#define HAS_MODEM		/* has simulated 'AT' style modem over TCP/IP (telnet) */
#define HAS_APU			/* has simulated AM9511 floating point maths coprocessor */

#define MAX_RAM	64	/* Maximum RAM size */

#define NUMNSOC 0	/* number of TCP/IP sockets for SIO connections */
#define NUMUSOC 1	/* number of UNIX sockets for SIO connections */

/*
 *	Default CPU
 */
#define Z80		1
#define I8080		2
#define DEFAULT_CPU	I8080

/*
 *	The following lines of this file should not be modified by user
 */
#define COPYR	"Copyright (C) 1987-2021 by Udo Munk"
#define RELEASE	"1.37"

#define USR_COM	"IMSAI 8080 Simulation"
#define USR_REL	"1.18"
#define USR_CPR	"\nCopyright (C) 2008-2021 by Udo Munk & " \
		"2018-2021 by David McNaughton"

#define MAX_LFN		4096		/* maximum long file name length */
#define LENCMD		80		/* length of command buffers etc */

#define S_FLAG		128		/* bit definitions of CPU flags */
#define Z_FLAG		64
#define N2_FLAG		32
#define H_FLAG		16
#define N1_FLAG		8
#define P_FLAG		4
#define N_FLAG		2
#define C_FLAG		1

#define CPU_MEMR	128		/* bit definitions for CPU bus status */
#define CPU_INP		64
#define CPU_M1		32
#define CPU_OUT		16
#define CPU_HLTA	8
#define CPU_STACK	4
#define CPU_WO		2
#define CPU_INTA	1

					/* operation of simulated CPU */
#define STOPPED		0		/* stopped */
#define CONTIN_RUN	1		/* continual run */
#define SINGLE_STEP	2		/* single step */
#define RESET		4		/* reset */

					/* error codes */
#define NONE		0		/* no error */
#define OPHALT		1		/* HALT op-code trap */
#define IOTRAPIN	2		/* I/O trap input */
#define IOTRAPOUT	3		/* I/O trap output */
#define IOHALT		4		/* halt system via I/O register */
#define IOERROR		5		/* fatal I/O error */
#define OPTRAP1		6		/* illegal 1 byte op-code trap */
#define OPTRAP2		7		/* illegal 2 byte op-code trap */
#define OPTRAP4		8		/* illegal 4 byte op-code trap */
#define USERINT		9		/* user interrupt */
#define INTERROR	10		/* unsupported bus data on interrupt */
#define POWEROFF	255		/* CPU off, no error */

typedef unsigned short WORD;		/* 16 bit unsigned */
typedef signed short   SWORD;		/* 16 bit signed */
typedef unsigned char  BYTE;		/* 8 bit unsigned */

#ifdef HISIZE
struct history {			/* structure of a history entry */
	WORD	h_adr;			/* address of execution */
	WORD	h_af;			/* register AF */
	WORD	h_bc;			/* register BC */
	WORD	h_de;			/* register DE */
	WORD	h_hl;			/* register HL */
	WORD	h_ix;			/* register IX */
	WORD	h_iy;			/* register IY */
	WORD	h_sp;			/* register SP */
};
#endif

#ifdef SBSIZE
struct softbreak {			/* structure of a breakpoint */
	WORD	sb_adr;			/* address of breakpoint */
	BYTE	sb_oldopc;		/* op-code at address of breakpoint */
	int	sb_passcount;		/* pass counter of breakpoint */
	int	sb_pass;		/* no. of pass to break */
};
#endif

#ifndef isxdigit
#define isxdigit(c) ((c<='f'&&c>='a')||(c<='F'&&c>='A')||(c<='9'&&c>='0'))
#endif

extern void sleep_ms(int);
#define SLEEP_MS(t)	sleep_ms(t)
