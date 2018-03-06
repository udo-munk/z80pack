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
 *	starting with 0xdd 0xcb
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

static int trap_ddcb(void);
static int op_tb0ixd(int), op_tb1ixd(int), op_tb2ixd(int), op_tb3ixd(int);
static int op_tb4ixd(int), op_tb5ixd(int), op_tb6ixd(int), op_tb7ixd(int);
static int op_rb0ixd(int), op_rb1ixd(int), op_rb2ixd(int), op_rb3ixd(int);
static int op_rb4ixd(int), op_rb5ixd(int), op_rb6ixd(int), op_rb7ixd(int);
static int op_sb0ixd(int), op_sb1ixd(int), op_sb2ixd(int), op_sb3ixd(int);
static int op_sb4ixd(int), op_sb5ixd(int), op_sb6ixd(int), op_sb7ixd(int);
static int op_rlcixd(int), op_rrcixd(int), op_rlixd(int), op_rrixd(int);
static int op_slaixd(int), op_sraixd(int), op_srlixd(int);

#ifdef Z80_UNDOC
#	define UNDOC_GEN_DD
#	include "undoc_ddfd_cb.h"
#	define UNDOC(f) f
#else
#	define UNDOC(f) trap_ddcb
#endif

int op_ddcb_handel(void)
{
	static int (*op_ddcb[256]) () = {
		UNDOC(op_undoc_rlcixdb),	/* 0x00 */
		UNDOC(op_undoc_rlcixdc),	/* 0x01 */
		UNDOC(op_undoc_rlcixdd),	/* 0x02 */
		UNDOC(op_undoc_rlcixde),	/* 0x03 */
		UNDOC(op_undoc_rlcixdh),	/* 0x04 */
		UNDOC(op_undoc_rlcixdl),	/* 0x05 */
		op_rlcixd,			/* 0x06 */
		UNDOC(op_undoc_rlcixda),	/* 0x07 */
		UNDOC(op_undoc_rrcixdb),	/* 0x08 */
		UNDOC(op_undoc_rrcixdc),	/* 0x09 */
		UNDOC(op_undoc_rrcixdd),	/* 0x0a */
		UNDOC(op_undoc_rrcixde),	/* 0x0b */
		UNDOC(op_undoc_rrcixdh),	/* 0x0c */
		UNDOC(op_undoc_rrcixdl),	/* 0x0d */
		op_rrcixd,			/* 0x0e */
		UNDOC(op_undoc_rrcixda),	/* 0x0f */
		UNDOC(op_undoc_rlixdb),		/* 0x10 */
		UNDOC(op_undoc_rlixdc),		/* 0x11 */
		UNDOC(op_undoc_rlixdd),		/* 0x12 */
		UNDOC(op_undoc_rlixde),		/* 0x13 */
		UNDOC(op_undoc_rlixdh),		/* 0x14 */
		UNDOC(op_undoc_rlixdl),		/* 0x15 */
		op_rlixd,			/* 0x16 */
		UNDOC(op_undoc_rlixda),		/* 0x17 */
		UNDOC(op_undoc_rrixdb),		/* 0x18 */
		UNDOC(op_undoc_rrixdc),		/* 0x19 */
		UNDOC(op_undoc_rrixdd),		/* 0x1a */
		UNDOC(op_undoc_rrixde),		/* 0x1b */
		UNDOC(op_undoc_rrixdh),		/* 0x1c */
		UNDOC(op_undoc_rrixdl),		/* 0x1d */
		op_rrixd,			/* 0x1e */
		UNDOC(op_undoc_rrixda),		/* 0x1f */
		UNDOC(op_undoc_slaixdb),	/* 0x20 */
		UNDOC(op_undoc_slaixdc),	/* 0x21 */
		UNDOC(op_undoc_slaixdd),	/* 0x22 */
		UNDOC(op_undoc_slaixde),	/* 0x23 */
		UNDOC(op_undoc_slaixdh),	/* 0x24 */
		UNDOC(op_undoc_slaixdl),	/* 0x25 */
		op_slaixd,			/* 0x26 */
		UNDOC(op_undoc_slaixda),	/* 0x27 */
		UNDOC(op_undoc_sraixdb),	/* 0x28 */
		UNDOC(op_undoc_sraixdc),	/* 0x29 */
		UNDOC(op_undoc_sraixdd),	/* 0x2a */
		UNDOC(op_undoc_sraixde),	/* 0x2b */
		UNDOC(op_undoc_sraixdh),	/* 0x2c */
		UNDOC(op_undoc_sraixdl),	/* 0x2d */
		op_sraixd,			/* 0x2e */
		UNDOC(op_undoc_sraixda),	/* 0x2f */
		UNDOC(op_undoc_sllixdb),	/* 0x30 */
		UNDOC(op_undoc_sllixdc),	/* 0x31 */
		UNDOC(op_undoc_sllixdd),	/* 0x32 */
		UNDOC(op_undoc_sllixde),	/* 0x33 */
		UNDOC(op_undoc_sllixdh),	/* 0x34 */
		UNDOC(op_undoc_sllixdl),	/* 0x35 */
		UNDOC(op_undoc_sllixd),		/* 0x36 */
		UNDOC(op_undoc_sllixda),	/* 0x37 */
		UNDOC(op_undoc_srlixdb),	/* 0x38 */
		UNDOC(op_undoc_srlixdc),	/* 0x39 */
		UNDOC(op_undoc_srlixdd),	/* 0x3a */
		UNDOC(op_undoc_srlixde),	/* 0x3b */
		UNDOC(op_undoc_srlixdh),	/* 0x3c */
		UNDOC(op_undoc_srlixdl),	/* 0x3d */
		op_srlixd,			/* 0x3e */
		UNDOC(op_undoc_srlixda),	/* 0x3f */
		UNDOC(op_undoc_tb0ixd),		/* 0x40 */
		UNDOC(op_undoc_tb0ixd),		/* 0x41 */
		UNDOC(op_undoc_tb0ixd),		/* 0x42 */
		UNDOC(op_undoc_tb0ixd),		/* 0x43 */
		UNDOC(op_undoc_tb0ixd),		/* 0x44 */
		UNDOC(op_undoc_tb0ixd),		/* 0x45 */
		op_tb0ixd,			/* 0x46 */
		UNDOC(op_undoc_tb0ixd),		/* 0x47 */
		UNDOC(op_undoc_tb1ixd),		/* 0x48 */
		UNDOC(op_undoc_tb1ixd),		/* 0x49 */
		UNDOC(op_undoc_tb1ixd),		/* 0x4a */
		UNDOC(op_undoc_tb1ixd),		/* 0x4b */
		UNDOC(op_undoc_tb1ixd),		/* 0x4c */
		UNDOC(op_undoc_tb1ixd),		/* 0x4d */
		op_tb1ixd,			/* 0x4e */
		UNDOC(op_undoc_tb1ixd),		/* 0x4f */
		UNDOC(op_undoc_tb2ixd),		/* 0x50 */
		UNDOC(op_undoc_tb2ixd),		/* 0x51 */
		UNDOC(op_undoc_tb2ixd),		/* 0x52 */
		UNDOC(op_undoc_tb2ixd),		/* 0x53 */
		UNDOC(op_undoc_tb2ixd),		/* 0x54 */
		UNDOC(op_undoc_tb2ixd),		/* 0x55 */
		op_tb2ixd,			/* 0x56 */
		UNDOC(op_undoc_tb2ixd),		/* 0x57 */
		UNDOC(op_undoc_tb3ixd),		/* 0x58 */
		UNDOC(op_undoc_tb3ixd),		/* 0x59 */
		UNDOC(op_undoc_tb3ixd),		/* 0x5a */
		UNDOC(op_undoc_tb3ixd),		/* 0x5b */
		UNDOC(op_undoc_tb3ixd),		/* 0x5c */
		UNDOC(op_undoc_tb3ixd),		/* 0x5d */
		op_tb3ixd,			/* 0x5e */
		UNDOC(op_undoc_tb3ixd),		/* 0x5f */
		UNDOC(op_undoc_tb4ixd),		/* 0x60 */
		UNDOC(op_undoc_tb4ixd),		/* 0x61 */
		UNDOC(op_undoc_tb4ixd),		/* 0x62 */
		UNDOC(op_undoc_tb4ixd),		/* 0x63 */
		UNDOC(op_undoc_tb4ixd),		/* 0x64 */
		UNDOC(op_undoc_tb4ixd),		/* 0x65 */
		op_tb4ixd,			/* 0x66 */
		UNDOC(op_undoc_tb4ixd),		/* 0x67 */
		UNDOC(op_undoc_tb5ixd),		/* 0x68 */
		UNDOC(op_undoc_tb5ixd),		/* 0x69 */
		UNDOC(op_undoc_tb5ixd),		/* 0x6a */
		UNDOC(op_undoc_tb5ixd),		/* 0x6b */
		UNDOC(op_undoc_tb5ixd),		/* 0x6c */
		UNDOC(op_undoc_tb5ixd),		/* 0x6d */
		op_tb5ixd,			/* 0x6e */
		UNDOC(op_undoc_tb5ixd),		/* 0x6f */
		UNDOC(op_undoc_tb6ixd),		/* 0x70 */
		UNDOC(op_undoc_tb6ixd),		/* 0x71 */
		UNDOC(op_undoc_tb6ixd),		/* 0x72 */
		UNDOC(op_undoc_tb6ixd),		/* 0x73 */
		UNDOC(op_undoc_tb6ixd),		/* 0x74 */
		UNDOC(op_undoc_tb6ixd),		/* 0x75 */
		op_tb6ixd,			/* 0x76 */
		UNDOC(op_undoc_tb6ixd),		/* 0x77 */
		UNDOC(op_undoc_tb7ixd),		/* 0x78 */
		UNDOC(op_undoc_tb7ixd),		/* 0x79 */
		UNDOC(op_undoc_tb7ixd),		/* 0x7a */
		UNDOC(op_undoc_tb7ixd),		/* 0x7b */
		UNDOC(op_undoc_tb7ixd),		/* 0x7c */
		UNDOC(op_undoc_tb7ixd),		/* 0x7d */
		op_tb7ixd,			/* 0x7e */
		UNDOC(op_undoc_tb7ixd),		/* 0x7f */
		UNDOC(op_undoc_rb0ixdb),	/* 0x80 */
		UNDOC(op_undoc_rb0ixdc),	/* 0x81 */
		UNDOC(op_undoc_rb0ixdd),	/* 0x82 */
		UNDOC(op_undoc_rb0ixde),	/* 0x83 */
		UNDOC(op_undoc_rb0ixdh),	/* 0x84 */
		UNDOC(op_undoc_rb0ixdl),	/* 0x85 */
		op_rb0ixd,			/* 0x86 */
		UNDOC(op_undoc_rb0ixda),	/* 0x87 */
		UNDOC(op_undoc_rb1ixdb),	/* 0x88 */
		UNDOC(op_undoc_rb1ixdc),	/* 0x89 */
		UNDOC(op_undoc_rb1ixdd),	/* 0x8a */
		UNDOC(op_undoc_rb1ixde),	/* 0x8b */
		UNDOC(op_undoc_rb1ixdh),	/* 0x8c */
		UNDOC(op_undoc_rb1ixdl),	/* 0x8d */
		op_rb1ixd,			/* 0x8e */
		UNDOC(op_undoc_rb1ixda),	/* 0x8f */
		UNDOC(op_undoc_rb2ixdb),	/* 0x90 */
		UNDOC(op_undoc_rb2ixdc),	/* 0x91 */
		UNDOC(op_undoc_rb2ixdd),	/* 0x92 */
		UNDOC(op_undoc_rb2ixde),	/* 0x93 */
		UNDOC(op_undoc_rb2ixdh),	/* 0x94 */
		UNDOC(op_undoc_rb2ixdl),	/* 0x95 */
		op_rb2ixd,			/* 0x96 */
		UNDOC(op_undoc_rb2ixda),	/* 0x97 */
		UNDOC(op_undoc_rb3ixdb),	/* 0x98 */
		UNDOC(op_undoc_rb3ixdc),	/* 0x99 */
		UNDOC(op_undoc_rb3ixdd),	/* 0x9a */
		UNDOC(op_undoc_rb3ixde),	/* 0x9b */
		UNDOC(op_undoc_rb3ixdh),	/* 0x9c */
		UNDOC(op_undoc_rb3ixdl),	/* 0x9d */
		op_rb3ixd,			/* 0x9e */
		UNDOC(op_undoc_rb3ixda),	/* 0x9f */
		UNDOC(op_undoc_rb4ixdb),	/* 0xa0 */
		UNDOC(op_undoc_rb4ixdc),	/* 0xa1 */
		UNDOC(op_undoc_rb4ixdd),	/* 0xa2 */
		UNDOC(op_undoc_rb4ixde),	/* 0xa3 */
		UNDOC(op_undoc_rb4ixdh),	/* 0xa4 */
		UNDOC(op_undoc_rb4ixdl),	/* 0xa5 */
		op_rb4ixd,			/* 0xa6 */
		UNDOC(op_undoc_rb4ixda),	/* 0xa7 */
		UNDOC(op_undoc_rb5ixdb),	/* 0xa8 */
		UNDOC(op_undoc_rb5ixdc),	/* 0xa9 */
		UNDOC(op_undoc_rb5ixdd),	/* 0xaa */
		UNDOC(op_undoc_rb5ixde),	/* 0xab */
		UNDOC(op_undoc_rb5ixdh),	/* 0xac */
		UNDOC(op_undoc_rb5ixdl),	/* 0xad */
		op_rb5ixd,			/* 0xae */
		UNDOC(op_undoc_rb5ixda),	/* 0xaf */
		UNDOC(op_undoc_rb6ixdb),	/* 0xb0 */
		UNDOC(op_undoc_rb6ixdc),	/* 0xb1 */
		UNDOC(op_undoc_rb6ixdd),	/* 0xb2 */
		UNDOC(op_undoc_rb6ixde),	/* 0xb3 */
		UNDOC(op_undoc_rb6ixdh),	/* 0xb4 */
		UNDOC(op_undoc_rb6ixdl),	/* 0xb5 */
		op_rb6ixd,			/* 0xb6 */
		UNDOC(op_undoc_rb6ixda),	/* 0xb7 */
		UNDOC(op_undoc_rb7ixdb),	/* 0xb8 */
		UNDOC(op_undoc_rb7ixdc),	/* 0xb9 */
		UNDOC(op_undoc_rb7ixdd),	/* 0xba */
		UNDOC(op_undoc_rb7ixde),	/* 0xbb */
		UNDOC(op_undoc_rb7ixdh),	/* 0xbc */
		UNDOC(op_undoc_rb7ixdl),	/* 0xbd */
		op_rb7ixd,			/* 0xbe */
		UNDOC(op_undoc_rb7ixda),	/* 0xbf */
		UNDOC(op_undoc_sb0ixdb),	/* 0xc0 */
		UNDOC(op_undoc_sb0ixdc),	/* 0xc1 */
		UNDOC(op_undoc_sb0ixdd),	/* 0xc2 */
		UNDOC(op_undoc_sb0ixde),	/* 0xc3 */
		UNDOC(op_undoc_sb0ixdh),	/* 0xc4 */
		UNDOC(op_undoc_sb0ixdl),	/* 0xc5 */
		op_sb0ixd,			/* 0xc6 */
		UNDOC(op_undoc_sb0ixda),	/* 0xc7 */
		UNDOC(op_undoc_sb1ixdb),	/* 0xc8 */
		UNDOC(op_undoc_sb1ixdc),	/* 0xc9 */
		UNDOC(op_undoc_sb1ixdd),	/* 0xca */
		UNDOC(op_undoc_sb1ixde),	/* 0xcb */
		UNDOC(op_undoc_sb1ixdh),	/* 0xcc */
		UNDOC(op_undoc_sb1ixdl),	/* 0xcd */
		op_sb1ixd,			/* 0xce */
		UNDOC(op_undoc_sb1ixda),	/* 0xcf */
		UNDOC(op_undoc_sb2ixdb),	/* 0xd0 */
		UNDOC(op_undoc_sb2ixdc),	/* 0xd1 */
		UNDOC(op_undoc_sb2ixdd),	/* 0xd2 */
		UNDOC(op_undoc_sb2ixde),	/* 0xd3 */
		UNDOC(op_undoc_sb2ixdh),	/* 0xd4 */
		UNDOC(op_undoc_sb2ixdl),	/* 0xd5 */
		op_sb2ixd,			/* 0xd6 */
		UNDOC(op_undoc_sb2ixda),	/* 0xd7 */
		UNDOC(op_undoc_sb3ixdb),	/* 0xd8 */
		UNDOC(op_undoc_sb3ixdc),	/* 0xd9 */
		UNDOC(op_undoc_sb3ixdd),	/* 0xda */
		UNDOC(op_undoc_sb3ixde),	/* 0xdb */
		UNDOC(op_undoc_sb3ixdh),	/* 0xdc */
		UNDOC(op_undoc_sb3ixdl),	/* 0xdd */
		op_sb3ixd,			/* 0xde */
		UNDOC(op_undoc_sb3ixda),	/* 0xdf */
		UNDOC(op_undoc_sb4ixdb),	/* 0xe0 */
		UNDOC(op_undoc_sb4ixdc),	/* 0xe1 */
		UNDOC(op_undoc_sb4ixdd),	/* 0xe2 */
		UNDOC(op_undoc_sb4ixde),	/* 0xe3 */
		UNDOC(op_undoc_sb4ixdh),	/* 0xe4 */
		UNDOC(op_undoc_sb4ixdl),	/* 0xe5 */
		op_sb4ixd,			/* 0xe6 */
		UNDOC(op_undoc_sb4ixda),	/* 0xe7 */
		UNDOC(op_undoc_sb5ixdb),	/* 0xe8 */
		UNDOC(op_undoc_sb5ixdc),	/* 0xe9 */
		UNDOC(op_undoc_sb5ixdd),	/* 0xea */
		UNDOC(op_undoc_sb5ixde),	/* 0xeb */
		UNDOC(op_undoc_sb5ixdh),	/* 0xec */
		UNDOC(op_undoc_sb5ixdl),	/* 0xed */
		op_sb5ixd,			/* 0xee */
		UNDOC(op_undoc_sb5ixda),	/* 0xef */
		UNDOC(op_undoc_sb6ixdb),	/* 0xf0 */
		UNDOC(op_undoc_sb6ixdc),	/* 0xf1 */
		UNDOC(op_undoc_sb6ixdd),	/* 0xf2 */
		UNDOC(op_undoc_sb6ixde),	/* 0xf3 */
		UNDOC(op_undoc_sb6ixdh),	/* 0xf4 */
		UNDOC(op_undoc_sb6ixdl),	/* 0xf5 */
		op_sb6ixd,			/* 0xf6 */
		UNDOC(op_undoc_sb6ixda),	/* 0xf7 */
		UNDOC(op_undoc_sb7ixdb),	/* 0xf8 */
		UNDOC(op_undoc_sb7ixdc),	/* 0xf9 */
		UNDOC(op_undoc_sb7ixdd),	/* 0xfa */
		UNDOC(op_undoc_sb7ixde),	/* 0xfb */
		UNDOC(op_undoc_sb7ixdh),	/* 0xfc */
		UNDOC(op_undoc_sb7ixdl),	/* 0xfd */
		op_sb7ixd,			/* 0xfe */
		UNDOC(op_undoc_sb7ixda),	/* 0xff */
	};

	register int d;
	register int t;

	d = (signed char) memrdr(PC++);
	t = (*op_ddcb[memrdr(PC++)]) (d); /* execute next opcode */


	return(t);
}

/*
 *	This function traps all illegal opcodes following the
 *	initial 0xdd 0xcb of a 4 byte opcode.
 */
static int trap_ddcb(void)
{
	cpu_error = OPTRAP4;
	cpu_state = STOPPED;
	return(0);
}

static int op_tb0ixd(int data)		/* BIT 0,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb1ixd(int data)		/* BIT 1,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb2ixd(int data)		/* BIT 2,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb3ixd(int data)		/* BIT 3,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb4ixd(int data)		/* BIT 4,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb5ixd(int data)		/* BIT 5,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb6ixd(int data)		/* BIT 6,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return(20);
}

static int op_tb7ixd(int data)		/* BIT 7,(IX+d) */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (memrdr(IX + data) & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(20);
}

static int op_rb0ixd(int data)		/* RES 0,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~1);
	return(23);
}

static int op_rb1ixd(int data)		/* RES 1,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~2);
	return(23);
}

static int op_rb2ixd(int data)		/* RES 2,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~4);
	return(23);
}

static int op_rb3ixd(int data)		/* RES 3,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~8);
	return(23);
}

static int op_rb4ixd(int data)		/* RES 4,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~16);
	return(23);
}

static int op_rb5ixd(int data)		/* RES 5,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~32);
	return(23);
}

static int op_rb6ixd(int data)		/* RES 6,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~64);
	return(23);
}

static int op_rb7ixd(int data)		/* RES 7,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~128);
	return(23);
}

static int op_sb0ixd(int data)		/* SET 0,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 1);
	return(23);
}

static int op_sb1ixd(int data)		/* SET 1,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 2);
	return(23);
}

static int op_sb2ixd(int data)		/* SET 2,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 4);
	return(23);
}

static int op_sb3ixd(int data)		/* SET 3,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 8);
	return(23);
}

static int op_sb4ixd(int data)		/* SET 4,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 16);
	return(23);
}

static int op_sb5ixd(int data)		/* SET 5,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 32);
	return(23);
}

static int op_sb6ixd(int data)		/* SET 6,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 64);
	return(23);
}

static int op_sb7ixd(int data)		/* SET 7,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 128);
	return(23);
}

static int op_rlcixd(int data)		/* RLC (IX+d) */
{
	register BYTE P;
        WORD addr;
	int i;

	addr = IX + data;
	P = memrdr(addr);
	i = P & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (i) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rrcixd(int data)		/* RRC (IX+d) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = IX + data;
	P = memrdr(addr);
	i = P & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (i) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rlixd(int data)		/* RL (IX+d) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = IX + data;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (old_c_flag) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_rrixd(int data)		/* RR (IX+d) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = IX + data;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (old_c_flag) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_slaixd(int data)		/* SLA (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_sraixd(int data)		/* SRA (IX+d) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = IX + data;
	P = memrdr(addr);
	i = P & 128;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P >> 1) | i;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?(F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}

static int op_srlixd(int data)		/* SRL (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + data;
	P = memrdr(addr);
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(23);
}
