/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2022 by Udo Munk
 */

#define COPYR	"Copyright (C) 1987-2022 by Udo Munk"
#define RELEASE	"1.38-dev"

#define MAX_LFN		4096		/* maximum long file name length */
#define LENCMD		80		/* length of command buffers etc */

#define Z80		1		/* simulated CPUs */
#define I8080		2

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

#ifdef FRONTPANEL
#define BUS_8080			/* emulate 8080 bus status */
#endif

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
