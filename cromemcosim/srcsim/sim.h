/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2014-2022 Udo Munk
 * Copyright (C) 2021-2022 David McNaughton
 *
 * Configuration for a Cromemco Z-1 system
 *
 * History:
 * 15-DEC-14 first version
 * 20-DEC-14 added 4FDC emulation and machine boots CP/M 2.2
 * 28-DEC-14 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-15 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-15 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-15 fdc and tu-art improvements, implemented banked memory
 * 10-MAR-15 TU-ART lpt's implemented for CP/M, CDOS and Cromix
 * 26-MAR-15 TU-ART tty's implemented for CDOS and Cromix
 * 11-AUG-16 implemented memwrt as function to support ROM
 * 01-DEC-16 implemented memrdr to separate memory from CPU
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 12-JAN-17 improved configuration and front panel LED timing
 * 10-APR-18 trap CPU on unsupported bus data during interrupt
 * 22-APR-18 implemented TCP socket polling
 * 29-AUG-21 new memory configuration sections
 * 02-SEP-21 implement banked ROM
 * 14-JUL-22 added generic AT modem and HAL
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define CPU_SPEED 4	/* default CPU speed */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
/*#define WANT_FASTB*/	/* much faster but not accurate Z80 block instr. */
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */
#define FRONTPANEL	/* emulate a machines frontpanel */
#define BUS_8080	/* emulate 8080 bus status for front panel */

#define HAS_DAZZLER	/* has simulated I/O for Cromemco Dazzler */
#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */
#define HAS_BANKED_ROM	/* has banked RDOS ROM */

/*#define HAS_DISKMANAGER*/	/* uses file based disk map for disks[] */
/*#define HAS_NETSERVER*/	/* uses civet webserver to present a web based frontend */
#define HAS_MODEM		/* has simulated 'AT' style modem over TCP/IP (telnet) */
#define HAS_HAL			/* implements a hardware abstraction layer (HAL) for TU-ART devices */

#define CROMEMCOSIM
#define MACHINE "cromemco"
#define DOCUMENT_ROOT "../webfrontend/www/" MACHINE

#define NUMNSOC 2	/* number of TCP/IP sockets, 2 per TU-ART */
#define TCPASYNC	/* use async I/O if possible */
#define SERVERPORT 4010	/* first TCP/IP server port used */
#define NUMUSOC 0	/* number of UNIX sockets */

/*
 * SIGIO on BSD sockets not working with Cygwin
 */
#ifdef __CYGWIN__
#undef TCPASYNC
#endif

/*
 *	Default CPU
 */
#define Z80		1
#define I8080		2
#define DEFAULT_CPU	Z80

/*
 *	The following lines of this file should not be modified by user
 */
#define COPYR	"Copyright (C) 1987-2022 by Udo Munk"
#define RELEASE	"1.38-dev"

#define USR_COM	"Cromemco Z-1 Simulation"
#define USR_REL	"1.19"
#define USR_CPR	"\nCopyright (C) 2014-2022 by Udo Munk & " \
		"2021-2022 by David McNaughton"

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
	WORD	h_addr;			/* address of execution */
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
	WORD	sb_addr;		/* address of breakpoint */
	BYTE	sb_oldopc;		/* op-code at address of breakpoint */
	int	sb_passcount;		/* pass counter of breakpoint */
	int	sb_pass;		/* no. of pass to break */
};
#endif

extern void sleep_ms(int);
#define SLEEP_MS(t)	sleep_ms(t)

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void)(x)
