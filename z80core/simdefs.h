/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMDEFS_INC
#define SIMDEFS_INC

#include <stdint.h>

#define COPYR	"Copyright (C) 1987-2024 by Udo Munk and others"
#define RELEASE	"1.39"

#define MAX_LFN		4096	/* maximum long file name length */
#define LENCMD		80	/* length of command buffers etc */

				/* simulated CPUs */
#ifndef EXCLUDE_Z80
#define Z80		1
#endif
#ifndef EXCLUDE_I8080
#define I8080		2
#endif

/* check validity of sim.h options */
#if defined(EXCLUDE_I8080) && defined(EXCLUDE_Z80)
#error "Only one of EXCLUDE_I8080 or EXCLUDE_Z80 can be used"
#endif
#if defined(EXCLUDE_I8080) && DEF_CPU != Z80
#error "DEF_CPU=I8080 and no 8080 simulation included"
#endif
#if defined(EXCLUDE_Z80) && DEF_CPU != I8080
#error "DEF_CPU=Z80 and no Z80 simulation included"
#endif
#if defined(EXCLUDE_Z80) && defined(FAST_BLOCK)
#error "FAST_BLOCK makes only sense without EXCLUDE_Z80"
#endif
#if (defined(ALT_I8080) || defined(ALT_Z80)) && !defined(UNDOC_INST)
#error "UNDOC_INST required for alternate simulators"
#endif
#ifdef HISIZE
#ifndef WANT_ICE
#error "WANT_ICE required for HISIZE"
#endif
#if HISIZE < 1 || HISIZE > 1000
#error "HISIZE must be between 1 and 1000"
#endif
#endif /* HISIZE */
#ifdef SBSIZE
#ifndef WANT_ICE
#error "WANT_ICE required for SBSIZE"
#endif
#if SBSIZE < 1 || SBSIZE > 10
#error "SBSIZE must be between 1 and 10"
#endif
#endif /* SBSIZE */
#if defined(WANT_TIM) && !defined(WANT_ICE)
#error "WANT_ICE required for WANT_TIM"
#endif
#if defined(WANT_HB) && !defined(WANT_ICE)
#error "WANT_ICE required for WANT_HB"
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

				/* bit definitions for CPU bus status */
#define CPU_MEMR	128	/* memory read */
#define CPU_INP		64	/* input device address */
#define CPU_M1		32	/* machine cycle one */
#define CPU_OUT		16	/* output device address */
#define CPU_HLTA	8	/* halt acknowledge */
#define CPU_STACK	4	/* pushdown stack address */
#define CPU_WO		2	/* write or output (active low) */
#define CPU_INTA	1	/* interrupt acknowledge */

#if defined(FRONTPANEL) || defined(SIMPLEPANEL) || defined(WANT_HB)
#define BUS_8080		/* emulate 8080 bus status */
#endif

				/* operation state of simulated CPU */
#define ST_STOPPED	0	/* stopped */
#define ST_CONTIN_RUN	1	/* continual run */
#define ST_SINGLE_STEP	2	/* single step */
#define ST_RESET	4	/* reset */
#define ST_MODEL_SWITCH	8	/* model switched */

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

typedef uint16_t WORD;		/* 16 bit unsigned */
typedef int16_t  SWORD;		/* 16 bit signed */
typedef uint8_t  BYTE;		/* 8 bit unsigned */
typedef int8_t   SBYTE;		/* 8 bit signed */

typedef uint64_t Tstates_t;	/* uint64 for counting T-states */
typedef enum { BUS_DMA_NONE, BUS_DMA_BYTE,
	       BUS_DMA_BURST, BUS_DMA_CONTINUOUS } BusDMA_t;

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void) (x)

#endif /* !SIMDEFS_INC */
