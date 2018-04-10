/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2017 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 */

/*
 *	Like the function "cpu_z80()" this one emulates 4 byte opcodes
 *	starting with 0xfd 0xcb
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

static int trap_fdcb(void);
static int op_tb0iyd(int), op_tb1iyd(int), op_tb2iyd(int), op_tb3iyd(int);
static int op_tb4iyd(int), op_tb5iyd(int), op_tb6iyd(int), op_tb7iyd(int);
static int op_rb0iyd(int), op_rb1iyd(int), op_rb2iyd(int), op_rb3iyd(int);
static int op_rb4iyd(int), op_rb5iyd(int), op_rb6iyd(int), op_rb7iyd(int);
static int op_sb0iyd(int), op_sb1iyd(int), op_sb2iyd(int), op_sb3iyd(int);
static int op_sb4iyd(int), op_sb5iyd(int), op_sb6iyd(int), op_sb7iyd(int);
static int op_rlciyd(int), op_rrciyd(int), op_rliyd(int), op_rriyd(int);
static int op_slaiyd(int), op_sraiyd(int), op_srliyd(int);

#ifdef Z80_UNDOC
#	define UNDOC_GEN_FD
#	include "undoc_ddfd_cb.h"
#	define UNDOC(f) f
#else
#	define UNDOC(f) trap_fdcb
#endif

int op_fdcb_handel(void)
{
	static int (*op_fdcb[256]) () = {
		UNDOC(op_undoc_rlciydb),	/* 0x00 */
		UNDOC(op_undoc_rlciydc),	/* 0x01 */
		UNDOC(op_undoc_rlciydd),	/* 0x02 */
		UNDOC(op_undoc_rlciyde),	/* 0x03 */
		UNDOC(op_undoc_rlciydh),	/* 0x04 */
		UNDOC(op_undoc_rlciydl),	/* 0x05 */
		op_rlciyd,			/* 0x06 */
		UNDOC(op_undoc_rlciyda),	/* 0x07 */
		UNDOC(op_undoc_rrciydb),	/* 0x08 */
		UNDOC(op_undoc_rrciydc),	/* 0x09 */
		UNDOC(op_undoc_rrciydd),	/* 0x0a */
		UNDOC(op_undoc_rrciyde),	/* 0x0b */
		UNDOC(op_undoc_rrciydh),	/* 0x0c */
		UNDOC(op_undoc_rrciydl),	/* 0x0d */
		op_rrciyd,			/* 0x0e */
		UNDOC(op_undoc_rrciyda),	/* 0x0f */
		UNDOC(op_undoc_rliydb),		/* 0x10 */
		UNDOC(op_undoc_rliydc),		/* 0x11 */
		UNDOC(op_undoc_rliydd),		/* 0x12 */
		UNDOC(op_undoc_rliyde),		/* 0x13 */
		UNDOC(op_undoc_rliydh),		/* 0x14 */
		UNDOC(op_undoc_rliydl),		/* 0x15 */
		op_rliyd,			/* 0x16 */
		UNDOC(op_undoc_rliyda),		/* 0x17 */
		UNDOC(op_undoc_rriydb),		/* 0x18 */
		UNDOC(op_undoc_rriydc),		/* 0x19 */
		UNDOC(op_undoc_rriydd),		/* 0x1a */
		UNDOC(op_undoc_rriyde),		/* 0x1b */
		UNDOC(op_undoc_rriydh),		/* 0x1c */
		UNDOC(op_undoc_rriydl),		/* 0x1d */
		op_rriyd,			/* 0x1e */
		UNDOC(op_undoc_rriyda),		/* 0x1f */
		UNDOC(op_undoc_slaiydb),	/* 0x20 */
		UNDOC(op_undoc_slaiydc),	/* 0x21 */
		UNDOC(op_undoc_slaiydd),	/* 0x22 */
		UNDOC(op_undoc_slaiyde),	/* 0x23 */
		UNDOC(op_undoc_slaiydh),	/* 0x24 */
		UNDOC(op_undoc_slaiydl),	/* 0x25 */
		op_slaiyd,			/* 0x26 */
		UNDOC(op_undoc_slaiyda),	/* 0x27 */
		UNDOC(op_undoc_sraiydb),	/* 0x28 */
		UNDOC(op_undoc_sraiydc),	/* 0x29 */
		UNDOC(op_undoc_sraiydd),	/* 0x2a */
		UNDOC(op_undoc_sraiyde),	/* 0x2b */
		UNDOC(op_undoc_sraiydh),	/* 0x2c */
		UNDOC(op_undoc_sraiydl),	/* 0x2d */
		op_sraiyd,			/* 0x2e */
		UNDOC(op_undoc_sraiyda),	/* 0x2f */
		UNDOC(op_undoc_slliydb),	/* 0x30 */
		UNDOC(op_undoc_slliydc),	/* 0x31 */
		UNDOC(op_undoc_slliydd),	/* 0x32 */
		UNDOC(op_undoc_slliyde),	/* 0x33 */
		UNDOC(op_undoc_slliydh),	/* 0x34 */
		UNDOC(op_undoc_slliydl),	/* 0x35 */
		UNDOC(op_undoc_slliyd),		/* 0x36 */
		UNDOC(op_undoc_slliyda),	/* 0x37 */
		UNDOC(op_undoc_srliydb),	/* 0x38 */
		UNDOC(op_undoc_srliydc),	/* 0x39 */
		UNDOC(op_undoc_srliydd),	/* 0x3a */
		UNDOC(op_undoc_srliyde),	/* 0x3b */
		UNDOC(op_undoc_srliydh),	/* 0x3c */
		UNDOC(op_undoc_srliydl),	/* 0x3d */
		op_srliyd,			/* 0x3e */
		UNDOC(op_undoc_srliyda),	/* 0x3f */
		UNDOC(op_undoc_tb0iyd),		/* 0x40 */
		UNDOC(op_undoc_tb0iyd),		/* 0x41 */
		UNDOC(op_undoc_tb0iyd),		/* 0x42 */
		UNDOC(op_undoc_tb0iyd),		/* 0x43 */
		UNDOC(op_undoc_tb0iyd),		/* 0x44 */
		UNDOC(op_undoc_tb0iyd),		/* 0x45 */
		op_tb0iyd,			/* 0x46 */
		UNDOC(op_undoc_tb0iyd),		/* 0x47 */
		UNDOC(op_undoc_tb1iyd),		/* 0x48 */
		UNDOC(op_undoc_tb1iyd),		/* 0x49 */
		UNDOC(op_undoc_tb1iyd),		/* 0x4a */
		UNDOC(op_undoc_tb1iyd),		/* 0x4b */
		UNDOC(op_undoc_tb1iyd),		/* 0x4c */
		UNDOC(op_undoc_tb1iyd),		/* 0x4d */
		op_tb1iyd,			/* 0x4e */
		UNDOC(op_undoc_tb1iyd),		/* 0x4f */
		UNDOC(op_undoc_tb2iyd),		/* 0x50 */
		UNDOC(op_undoc_tb2iyd),		/* 0x51 */
		UNDOC(op_undoc_tb2iyd),		/* 0x52 */
		UNDOC(op_undoc_tb2iyd),		/* 0x53 */
		UNDOC(op_undoc_tb2iyd),		/* 0x54 */
		UNDOC(op_undoc_tb2iyd),		/* 0x55 */
		op_tb2iyd,			/* 0x56 */
		UNDOC(op_undoc_tb2iyd),		/* 0x57 */
		UNDOC(op_undoc_tb3iyd),		/* 0x58 */
		UNDOC(op_undoc_tb3iyd),		/* 0x59 */
		UNDOC(op_undoc_tb3iyd),		/* 0x5a */
		UNDOC(op_undoc_tb3iyd),		/* 0x5b */
		UNDOC(op_undoc_tb3iyd),		/* 0x5c */
		UNDOC(op_undoc_tb3iyd),		/* 0x5d */
		op_tb3iyd,			/* 0x5e */
		UNDOC(op_undoc_tb3iyd),		/* 0x5f */
		UNDOC(op_undoc_tb4iyd),		/* 0x60 */
		UNDOC(op_undoc_tb4iyd),		/* 0x61 */
		UNDOC(op_undoc_tb4iyd),		/* 0x62 */
		UNDOC(op_undoc_tb4iyd),		/* 0x63 */
		UNDOC(op_undoc_tb4iyd),		/* 0x64 */
		UNDOC(op_undoc_tb4iyd),		/* 0x65 */
		op_tb4iyd,			/* 0x66 */
		UNDOC(op_undoc_tb4iyd),		/* 0x67 */
		UNDOC(op_undoc_tb5iyd),		/* 0x68 */
		UNDOC(op_undoc_tb5iyd),		/* 0x69 */
		UNDOC(op_undoc_tb5iyd),		/* 0x6a */
		UNDOC(op_undoc_tb5iyd),		/* 0x6b */
		UNDOC(op_undoc_tb5iyd),		/* 0x6c */
		UNDOC(op_undoc_tb5iyd),		/* 0x6d */
		op_tb5iyd,			/* 0x6e */
		UNDOC(op_undoc_tb5iyd),		/* 0x6f */
		UNDOC(op_undoc_tb6iyd),		/* 0x70 */
		UNDOC(op_undoc_tb6iyd),		/* 0x71 */
		UNDOC(op_undoc_tb6iyd),		/* 0x72 */
		UNDOC(op_undoc_tb6iyd),		/* 0x73 */
		UNDOC(op_undoc_tb6iyd),		/* 0x74 */
		UNDOC(op_undoc_tb6iyd),		/* 0x75 */
		op_tb6iyd,			/* 0x76 */
		UNDOC(op_undoc_tb6iyd),		/* 0x77 */
		UNDOC(op_undoc_tb7iyd),		/* 0x78 */
		UNDOC(op_undoc_tb7iyd),		/* 0x79 */
		UNDOC(op_undoc_tb7iyd),		/* 0x7a */
		UNDOC(op_undoc_tb7iyd),		/* 0x7b */
		UNDOC(op_undoc_tb7iyd),		/* 0x7c */
		UNDOC(op_undoc_tb7iyd),		/* 0x7d */
		op_tb7iyd,			/* 0x7e */
		UNDOC(op_undoc_tb7iyd),		/* 0x7f */
		UNDOC(op_undoc_rb0iydb),	/* 0x80 */
		UNDOC(op_undoc_rb0iydc),	/* 0x81 */
		UNDOC(op_undoc_rb0iydd),	/* 0x82 */
		UNDOC(op_undoc_rb0iyde),	/* 0x83 */
		UNDOC(op_undoc_rb0iydh),	/* 0x84 */
		UNDOC(op_undoc_rb0iydl),	/* 0x85 */
		op_rb0iyd,			/* 0x86 */
		UNDOC(op_undoc_rb0iyda),	/* 0x87 */
		UNDOC(op_undoc_rb1iydb),	/* 0x88 */
		UNDOC(op_undoc_rb1iydc),	/* 0x89 */
		UNDOC(op_undoc_rb1iydd),	/* 0x8a */
		UNDOC(op_undoc_rb1iyde),	/* 0x8b */
		UNDOC(op_undoc_rb1iydh),	/* 0x8c */
		UNDOC(op_undoc_rb1iydl),	/* 0x8d */
		op_rb1iyd,			/* 0x8e */
		UNDOC(op_undoc_rb1iyda),	/* 0x8f */
		UNDOC(op_undoc_rb2iydb),	/* 0x90 */
		UNDOC(op_undoc_rb2iydc),	/* 0x91 */
		UNDOC(op_undoc_rb2iydd),	/* 0x92 */
		UNDOC(op_undoc_rb2iyde),	/* 0x93 */
		UNDOC(op_undoc_rb2iydh),	/* 0x94 */
		UNDOC(op_undoc_rb2iydl),	/* 0x95 */
		op_rb2iyd,			/* 0x96 */
		UNDOC(op_undoc_rb2iyda),	/* 0x97 */
		UNDOC(op_undoc_rb3iydb),	/* 0x98 */
		UNDOC(op_undoc_rb3iydc),	/* 0x99 */
		UNDOC(op_undoc_rb3iydd),	/* 0x9a */
		UNDOC(op_undoc_rb3iyde),	/* 0x9b */
		UNDOC(op_undoc_rb3iydh),	/* 0x9c */
		UNDOC(op_undoc_rb3iydl),	/* 0x9d */
		op_rb3iyd,			/* 0x9e */
		UNDOC(op_undoc_rb3iyda),	/* 0x9f */
		UNDOC(op_undoc_rb4iydb),	/* 0xa0 */
		UNDOC(op_undoc_rb4iydc),	/* 0xa1 */
		UNDOC(op_undoc_rb4iydd),	/* 0xa2 */
		UNDOC(op_undoc_rb4iyde),	/* 0xa3 */
		UNDOC(op_undoc_rb4iydh),	/* 0xa4 */
		UNDOC(op_undoc_rb4iydl),	/* 0xa5 */
		op_rb4iyd,			/* 0xa6 */
		UNDOC(op_undoc_rb4iyda),	/* 0xa7 */
		UNDOC(op_undoc_rb5iydb),	/* 0xa8 */
		UNDOC(op_undoc_rb5iydc),	/* 0xa9 */
		UNDOC(op_undoc_rb5iydd),	/* 0xaa */
		UNDOC(op_undoc_rb5iyde),	/* 0xab */
		UNDOC(op_undoc_rb5iydh),	/* 0xac */
		UNDOC(op_undoc_rb5iydl),	/* 0xad */
		op_rb5iyd,			/* 0xae */
		UNDOC(op_undoc_rb5iyda),	/* 0xaf */
		UNDOC(op_undoc_rb6iydb),	/* 0xb0 */
		UNDOC(op_undoc_rb6iydc),	/* 0xb1 */
		UNDOC(op_undoc_rb6iydd),	/* 0xb2 */
		UNDOC(op_undoc_rb6iyde),	/* 0xb3 */
		UNDOC(op_undoc_rb6iydh),	/* 0xb4 */
		UNDOC(op_undoc_rb6iydl),	/* 0xb5 */
		op_rb6iyd,			/* 0xb6 */
		UNDOC(op_undoc_rb6iyda),	/* 0xb7 */
		UNDOC(op_undoc_rb7iydb),	/* 0xb8 */
		UNDOC(op_undoc_rb7iydc),	/* 0xb9 */
		UNDOC(op_undoc_rb7iydd),	/* 0xba */
		UNDOC(op_undoc_rb7iyde),	/* 0xbb */
		UNDOC(op_undoc_rb7iydh),	/* 0xbc */
		UNDOC(op_undoc_rb7iydl),	/* 0xbd */
		op_rb7iyd,			/* 0xbe */
		UNDOC(op_undoc_rb7iyda),	/* 0xbf */
		UNDOC(op_undoc_sb0iydb),	/* 0xc0 */
		UNDOC(op_undoc_sb0iydc),	/* 0xc1 */
		UNDOC(op_undoc_sb0iydd),	/* 0xc2 */
		UNDOC(op_undoc_sb0iyde),	/* 0xc3 */
		UNDOC(op_undoc_sb0iydh),	/* 0xc4 */
		UNDOC(op_undoc_sb0iydl),	/* 0xc5 */
		op_sb0iyd,			/* 0xc6 */
		UNDOC(op_undoc_sb0iyda),	/* 0xc7 */
		UNDOC(op_undoc_sb1iydb),	/* 0xc8 */
		UNDOC(op_undoc_sb1iydc),	/* 0xc9 */
		UNDOC(op_undoc_sb1iydd),	/* 0xca */
		UNDOC(op_undoc_sb1iyde),	/* 0xcb */
		UNDOC(op_undoc_sb1iydh),	/* 0xcc */
		UNDOC(op_undoc_sb1iydl),	/* 0xcd */
		op_sb1iyd,			/* 0xce */
		UNDOC(op_undoc_sb1iyda),	/* 0xcf */
		UNDOC(op_undoc_sb2iydb),	/* 0xd0 */
		UNDOC(op_undoc_sb2iydc),	/* 0xd1 */
		UNDOC(op_undoc_sb2iydd),	/* 0xd2 */
		UNDOC(op_undoc_sb2iyde),	/* 0xd3 */
		UNDOC(op_undoc_sb2iydh),	/* 0xd4 */
		UNDOC(op_undoc_sb2iydl),	/* 0xd5 */
		op_sb2iyd,			/* 0xd6 */
		UNDOC(op_undoc_sb2iyda),	/* 0xd7 */
		UNDOC(op_undoc_sb3iydb),	/* 0xd8 */
		UNDOC(op_undoc_sb3iydc),	/* 0xd9 */
		UNDOC(op_undoc_sb3iydd),	/* 0xda */
		UNDOC(op_undoc_sb3iyde),	/* 0xdb */
		UNDOC(op_undoc_sb3iydh),	/* 0xdc */
		UNDOC(op_undoc_sb3iydl),	/* 0xdd */
		op_sb3iyd,			/* 0xde */
		UNDOC(op_undoc_sb3iyda),	/* 0xdf */
		UNDOC(op_undoc_sb4iydb),	/* 0xe0 */
		UNDOC(op_undoc_sb4iydc),	/* 0xe1 */
		UNDOC(op_undoc_sb4iydd),	/* 0xe2 */
		UNDOC(op_undoc_sb4iyde),	/* 0xe3 */
		UNDOC(op_undoc_sb4iydh),	/* 0xe4 */
		UNDOC(op_undoc_sb4iydl),	/* 0xe5 */
		op_sb4iyd,			/* 0xe6 */
		UNDOC(op_undoc_sb4iyda),	/* 0xe7 */
		UNDOC(op_undoc_sb5iydb),	/* 0xe8 */
		UNDOC(op_undoc_sb5iydc),	/* 0xe9 */
		UNDOC(op_undoc_sb5iydd),	/* 0xea */
		UNDOC(op_undoc_sb5iyde),	/* 0xeb */
		UNDOC(op_undoc_sb5iydh),	/* 0xec */
		UNDOC(op_undoc_sb5iydl),	/* 0xed */
		op_sb5iyd,			/* 0xee */
		UNDOC(op_undoc_sb5iyda),	/* 0xef */
		UNDOC(op_undoc_sb6iydb),	/* 0xf0 */
		UNDOC(op_undoc_sb6iydc),	/* 0xf1 */
		UNDOC(op_undoc_sb6iydd),	/* 0xf2 */
		UNDOC(op_undoc_sb6iyde),	/* 0xf3 */
		UNDOC(op_undoc_sb6iydh),	/* 0xf4 */
		UNDOC(op_undoc_sb6iydl),	/* 0xf5 */
		op_sb6iyd,			/* 0xf6 */
		UNDOC(op_undoc_sb6iyda),	/* 0xf7 */
		UNDOC(op_undoc_sb7iydb),	/* 0xf8 */
		UNDOC(op_undoc_sb7iydc),	/* 0xf9 */
		UNDOC(op_undoc_sb7iydd),	/* 0xfa */
		UNDOC(op_undoc_sb7iyde),	/* 0xfb */
		UNDOC(op_undoc_sb7iydh),	/* 0xfc */
		UNDOC(op_undoc_sb7iydl),	/* 0xfd */
		op_sb7iyd,			/* 0xfe */
		UNDOC(op_undoc_sb7iyda),	/* 0xff */
	};

	register int d;
	register int t;

	d = (signed char) memrdr(PC++);
	t = (*op_fdcb[memrdr(PC++)]) (d); /* execute next opcode */

	return(t);
}

/*
 *	This function traps all illegal opcodes following the
 *	initial 0xfd 0xcb of a 4 byte opcode.
 */
static int trap_fdcb(void)
{
	cpu_error = OPTRAP4;
	cpu_state = STOPPED;
	return(0L);
}

static int op_tb0iyd(int data)		/* BIT 0,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb1iyd(int data)		/* BIT 1,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb2iyd(int data)		/* BIT 2,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb3iyd(int data)		/* BIT 3,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb4iyd(int data)		/* BIT 4,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb5iyd(int data)		/* BIT 5,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr( IY + data) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				  : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb6iyd(int data)		/* BIT 6,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb7iyd(int data)		/* BIT 7,(IY+d) */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (memrdr(IY + data) & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(20);
}

static int op_rb0iyd(int data)		/* RES 0,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~1);
	return(23);
}

static int op_rb1iyd(int data)		/* RES 1,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~2);
	return(23);
}

static int op_rb2iyd(int data)		/* RES 2,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~4);
	return(23);
}

static int op_rb3iyd(int data)		/* RES 3,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~8);
	return(23);
}

static int op_rb4iyd(int data)		/* RES 4,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~16);
	return(23);
}

static int op_rb5iyd(int data)		/* RES 5,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~32);
	return(23);
}

static int op_rb6iyd(int data)		/* RES 6,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~64);
	return(23);
}

static int op_rb7iyd(int data)		/* RES 7,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~128);
	return(23);
}

static int op_sb0iyd(int data)		/* SET 0,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 1);
	return(23);
}

static int op_sb1iyd(int data)		/* SET 1,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 2);
	return(23);
}

static int op_sb2iyd(int data)		/* SET 2,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 4);
	return(23);
}

static int op_sb3iyd(int data)		/* SET 3,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 8);
	return(23);
}

static int op_sb4iyd(int data)		/* SET 4,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 16);
	return(23);
}

static int op_sb5iyd(int data)		/* SET 5,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 32);
	return(23);
}

static int op_sb6iyd(int data)		/* SET 6,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 64);
	return(23);
}

static int op_sb7iyd(int data)		/* SET 7,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 128);
	return(23);
}

static int op_rlciyd(int data)		/* RLC (IY+d) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = IY + data;
	P = memrdr(addr);
	i = P & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (i) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rrciyd(int data)		/* RRC (IY+d) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = IY + data;
	P = memrdr(addr);
	i = P & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (i) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rliyd(int data)		/* RL (IY+d) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = IY + data;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (old_c_flag) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rriyd(int data)		/* RR (IY+d) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = IY + data;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (old_c_flag) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_slaiyd(int data)		/* SLA (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_sraiyd(int data)		/* SRA (IY+d) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = IY + data;
	P = memrdr(addr);
	i = P & 128;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P >> 1) | i;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_srliyd(int data)		/* SRL (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + data;
	P = memrdr(addr);
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}
