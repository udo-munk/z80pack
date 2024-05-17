/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMCORE_INC
#define SIMCORE_INC

#define COPYR	"Copyright (C) 1987-2024 by Udo Munk and others"
#define RELEASE	"1.38"

#define MAX_LFN		4096	/* maximum long file name length */
#define LENCMD		80	/* length of command buffers etc */

				/* simulated CPUs */
#ifndef EXCLUDE_Z80
#define Z80		1
#endif
#ifndef EXCLUDE_I8080
#define I8080		2
#endif

#if defined(EXCLUDE_I8080) && defined(EXCLUDE_Z80)
#error "Only one of EXCLUDE_I8080 or EXCLUDE_Z80 can be used"
#endif
#if defined(EXCLUDE_I8080) && DEF_CPU != Z80
#error "DEF_CPU=I8080 and no 8080 simulation included"
#endif
#if defined(EXCLUDE_Z80) && DEF_CPU != I8080
#error "DEF_CPU=Z80 and no Z80 simulation included"
#endif
#if defined(EXCLUDE_Z80) && defined(UNDOC_IALL)
#error "UNDOC_IALL makes only sense without EXCLUDE_Z80"
#endif
#if defined(EXCLUDE_Z80) && defined(UNDOC_FLAGS)
#error "UNDOC_FLAGS makes only sense without EXCLUDE_Z80"
#endif
#if defined(EXCLUDE_Z80) && defined(FAST_BLOCK)
#error "FAST_BLOCK makes only sense without EXCLUDE_Z80"
#endif
#if defined(UNDOC_IALL) && !defined(UNDOC_INST)
#error "UNDOC_IALL makes only sense together with UNDOC_INST"
#endif

				/* bit definitions of CPU flags */
#define S_FLAG		128	/* sign flag */
#define Z_FLAG		64	/* zero flag */
#define Y_FLAG		32	/* undocumented */
#define H_FLAG		16	/* half carry flag */
#define X_FLAG		8	/* undocumented */
#define P_FLAG		4	/* parity/overflow flag */
#define N_FLAG		2	/* add/subtract flag */
#define C_FLAG		1	/* carry flag */

#ifdef FLAG_TABLES
#ifndef EXCLUDE_Z80
#define SZ_FLAGS	(S_FLAG | Z_FLAG)
#endif
#define SZP_FLAGS	(S_FLAG | Z_FLAG | P_FLAG)
#ifdef UNDOC_FLAGS
#define SZYX_FLAGS	(S_FLAG | Z_FLAG | Y_FLAG | X_FLAG)
#define SZYXP_FLAGS	(S_FLAG | Z_FLAG | Y_FLAG | X_FLAG | P_FLAG)
#endif
#endif

				/* bit definitions for CPU bus status */
#define CPU_MEMR	128	/* memory read */
#define CPU_INP		64	/* input device address */
#define CPU_M1		32	/* machine cycle one */
#define CPU_OUT		16	/* output device address */
#define CPU_HLTA	8	/* halt acknowledge */
#define CPU_STACK	4	/* pushdown stack address */
#define CPU_WO		2	/* write or output (active low) */
#define CPU_INTA	1	/* interrupt acknowledge */

#ifdef FRONTPANEL
#define BUS_8080		/* emulate 8080 bus status */
#endif

				/* operation state of simulated CPU */
#define STOPPED		0	/* stopped */
#define CONTIN_RUN	1	/* continual run */
#define SINGLE_STEP	2	/* single step */
#define RESET		4	/* reset */
#define MODEL_SWITCH	8	/* model switched */

				/* error codes */
#define NONE		0	/* no error */
#define OPHALT		1	/* HALT op-code trap */
#define IOTRAPIN	2	/* I/O trap input */
#define IOTRAPOUT	3	/* I/O trap output */
#define IOHALT		4	/* halt system via I/O register */
#define IOERROR		5	/* fatal I/O error */
#define OPTRAP1		6	/* illegal 1 byte op-code trap */
#define OPTRAP2		7	/* illegal 2 byte op-code trap */
#define OPTRAP4		8	/* illegal 4 byte op-code trap */
#define USERINT		9	/* user interrupt */
#define INTERROR	10	/* unsupported bus data on interrupt */
#define POWEROFF	255	/* CPU off, no error */

typedef unsigned short WORD;	/* 16 bit unsigned */
typedef signed short   SWORD;	/* 16 bit signed */
typedef unsigned char  BYTE;	/* 8 bit unsigned */

#ifdef HISIZE
struct history {		/* structure of a history entry */
	int	h_cpu;		/* CPU type */
	WORD	h_addr;		/* address of execution */
	WORD	h_af;		/* register AF */
	WORD	h_bc;		/* register BC */
	WORD	h_de;		/* register DE */
	WORD	h_hl;		/* register HL */
#ifndef EXCLUDE_Z80
	WORD	h_ix;		/* register IX */
	WORD	h_iy;		/* register IY */
#endif
	WORD	h_sp;		/* register SP */
};
#endif

#ifdef SBSIZE
struct softbreak {		/* structure of a breakpoint */
	WORD	sb_addr;	/* address of breakpoint */
	BYTE	sb_oldopc;	/* op-code at address of breakpoint */
	int	sb_passcount;	/* pass counter of breakpoint */
	int	sb_pass;	/* no. of pass to break */
};
#endif

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void) (x)

#endif
