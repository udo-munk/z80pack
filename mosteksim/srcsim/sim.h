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
#define CPU_SPEED 0	/* default CPU speed 0=unlimited */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
/*#define WANT_FASTM*/	/* much faster but not accurate Z80 block moves */
#define WANT_TIM	/* count t-states */
#define HISIZE	100	/* number of entries in history */
#define SBSIZE	4	/* number of software breakpoints */
/*#define FRONTPANEL*/	/* no frontpanel emulation */
/*#define BUS_8080*/	/* no emulation of 8080 bus status */

/*#define HAS_DISKS*/	/* not using standard disk define */
/*#define HAS_CONFIG*/  /* not using standard config define */

/*
 *	Default CPU
 */
#define Z80		1
#define I8080		2
#define DEFAULT_CPU	Z80

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"\nMostek AID-80F and SYS-80FT Emulator"
#define USR_REL	"1.1"
#define USR_CPR	"by Mike Douglas"

/*
 *	The following lines of this file should not be modified by user
 */
#define COPYR	"Copyright (C) 1987-2021 by Udo Munk"
#define RELEASE	"1.37"

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
