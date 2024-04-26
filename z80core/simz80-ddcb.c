/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022 by Thomas Eberhardt
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

#ifndef EXCLUDE_Z80

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_ddcb
#endif

static int trap_ddcb(int);
static int op_tb0ixd(int), op_tb1ixd(int), op_tb2ixd(int), op_tb3ixd(int);
static int op_tb4ixd(int), op_tb5ixd(int), op_tb6ixd(int), op_tb7ixd(int);
static int op_rb0ixd(int), op_rb1ixd(int), op_rb2ixd(int), op_rb3ixd(int);
static int op_rb4ixd(int), op_rb5ixd(int), op_rb6ixd(int), op_rb7ixd(int);
static int op_sb0ixd(int), op_sb1ixd(int), op_sb2ixd(int), op_sb3ixd(int);
static int op_sb4ixd(int), op_sb5ixd(int), op_sb6ixd(int), op_sb7ixd(int);
static int op_rlcixd(int), op_rrcixd(int), op_rlixd(int), op_rrixd(int);
static int op_slaixd(int), op_sraixd(int), op_srlixd(int);

#ifdef UNDOC_INST
static int op_undoc_tb0ixd(int), op_undoc_tb1ixd(int), op_undoc_tb2ixd(int);
static int op_undoc_tb3ixd(int), op_undoc_tb4ixd(int), op_undoc_tb5ixd(int);
static int op_undoc_tb6ixd(int), op_undoc_tb7ixd(int);
static int op_undoc_rb0ixda(int), op_undoc_rb1ixda(int), op_undoc_rb2ixda(int);
static int op_undoc_rb3ixda(int), op_undoc_rb4ixda(int), op_undoc_rb5ixda(int);
static int op_undoc_rb6ixda(int), op_undoc_rb7ixda(int);
static int op_undoc_rb0ixdb(int), op_undoc_rb1ixdb(int), op_undoc_rb2ixdb(int);
static int op_undoc_rb3ixdb(int), op_undoc_rb4ixdb(int), op_undoc_rb5ixdb(int);
static int op_undoc_rb6ixdb(int), op_undoc_rb7ixdb(int);
static int op_undoc_rb0ixdc(int), op_undoc_rb1ixdc(int), op_undoc_rb2ixdc(int);
static int op_undoc_rb3ixdc(int), op_undoc_rb4ixdc(int), op_undoc_rb5ixdc(int);
static int op_undoc_rb6ixdc(int), op_undoc_rb7ixdc(int);
static int op_undoc_rb0ixdd(int), op_undoc_rb1ixdd(int), op_undoc_rb2ixdd(int);
static int op_undoc_rb3ixdd(int), op_undoc_rb4ixdd(int), op_undoc_rb5ixdd(int);
static int op_undoc_rb6ixdd(int), op_undoc_rb7ixdd(int);
static int op_undoc_rb0ixde(int), op_undoc_rb1ixde(int), op_undoc_rb2ixde(int);
static int op_undoc_rb3ixde(int), op_undoc_rb4ixde(int), op_undoc_rb5ixde(int);
static int op_undoc_rb6ixde(int), op_undoc_rb7ixde(int);
static int op_undoc_rb0ixdh(int), op_undoc_rb1ixdh(int), op_undoc_rb2ixdh(int);
static int op_undoc_rb3ixdh(int), op_undoc_rb4ixdh(int), op_undoc_rb5ixdh(int);
static int op_undoc_rb6ixdh(int), op_undoc_rb7ixdh(int);
static int op_undoc_rb0ixdl(int), op_undoc_rb1ixdl(int), op_undoc_rb2ixdl(int);
static int op_undoc_rb3ixdl(int), op_undoc_rb4ixdl(int), op_undoc_rb5ixdl(int);
static int op_undoc_rb6ixdl(int), op_undoc_rb7ixdl(int);
static int op_undoc_sb0ixda(int), op_undoc_sb1ixda(int), op_undoc_sb2ixda(int);
static int op_undoc_sb3ixda(int), op_undoc_sb4ixda(int), op_undoc_sb5ixda(int);
static int op_undoc_sb6ixda(int), op_undoc_sb7ixda(int);
static int op_undoc_sb0ixdb(int), op_undoc_sb1ixdb(int), op_undoc_sb2ixdb(int);
static int op_undoc_sb3ixdb(int), op_undoc_sb4ixdb(int), op_undoc_sb5ixdb(int);
static int op_undoc_sb6ixdb(int), op_undoc_sb7ixdb(int);
static int op_undoc_sb0ixdc(int), op_undoc_sb1ixdc(int), op_undoc_sb2ixdc(int);
static int op_undoc_sb3ixdc(int), op_undoc_sb4ixdc(int), op_undoc_sb5ixdc(int);
static int op_undoc_sb6ixdc(int), op_undoc_sb7ixdc(int);
static int op_undoc_sb0ixdd(int), op_undoc_sb1ixdd(int), op_undoc_sb2ixdd(int);
static int op_undoc_sb3ixdd(int), op_undoc_sb4ixdd(int), op_undoc_sb5ixdd(int);
static int op_undoc_sb6ixdd(int), op_undoc_sb7ixdd(int);
static int op_undoc_sb0ixde(int), op_undoc_sb1ixde(int), op_undoc_sb2ixde(int);
static int op_undoc_sb3ixde(int), op_undoc_sb4ixde(int), op_undoc_sb5ixde(int);
static int op_undoc_sb6ixde(int), op_undoc_sb7ixde(int);
static int op_undoc_sb0ixdh(int), op_undoc_sb1ixdh(int), op_undoc_sb2ixdh(int);
static int op_undoc_sb3ixdh(int), op_undoc_sb4ixdh(int), op_undoc_sb5ixdh(int);
static int op_undoc_sb6ixdh(int), op_undoc_sb7ixdh(int);
static int op_undoc_sb0ixdl(int), op_undoc_sb1ixdl(int), op_undoc_sb2ixdl(int);
static int op_undoc_sb3ixdl(int), op_undoc_sb4ixdl(int), op_undoc_sb5ixdl(int);
static int op_undoc_sb6ixdl(int), op_undoc_sb7ixdl(int);
static int op_undoc_rlcixda(int), op_undoc_rlcixdb(int), op_undoc_rlcixdc(int);
static int op_undoc_rlcixdd(int), op_undoc_rlcixde(int), op_undoc_rlcixdh(int);
static int op_undoc_rlcixdl(int);
static int op_undoc_rrcixda(int), op_undoc_rrcixdb(int), op_undoc_rrcixdc(int);
static int op_undoc_rrcixdd(int), op_undoc_rrcixde(int), op_undoc_rrcixdh(int);
static int op_undoc_rrcixdl(int);
static int op_undoc_rlixda(int), op_undoc_rlixdb(int), op_undoc_rlixdc(int);
static int op_undoc_rlixdd(int), op_undoc_rlixde(int), op_undoc_rlixdh(int);
static int op_undoc_rlixdl(int);
static int op_undoc_rrixda(int), op_undoc_rrixdb(int), op_undoc_rrixdc(int);
static int op_undoc_rrixdd(int), op_undoc_rrixde(int), op_undoc_rrixdh(int);
static int op_undoc_rrixdl(int);
static int op_undoc_slaixda(int), op_undoc_slaixdb(int), op_undoc_slaixdc(int);
static int op_undoc_slaixdd(int), op_undoc_slaixde(int), op_undoc_slaixdh(int);
static int op_undoc_slaixdl(int);
static int op_undoc_sraixda(int), op_undoc_sraixdb(int), op_undoc_sraixdc(int);
static int op_undoc_sraixdd(int), op_undoc_sraixde(int), op_undoc_sraixdh(int);
static int op_undoc_sraixdl(int);
static int op_undoc_sllixda(int), op_undoc_sllixdb(int), op_undoc_sllixdc(int);
static int op_undoc_sllixdd(int), op_undoc_sllixde(int), op_undoc_sllixdh(int);
static int op_undoc_sllixdl(int), op_undoc_sllixd(int);
static int op_undoc_srlixda(int), op_undoc_srlixdb(int), op_undoc_srlixdc(int);
static int op_undoc_srlixdd(int), op_undoc_srlixde(int), op_undoc_srlixdh(int);
static int op_undoc_srlixdl(int);
#endif

int op_ddcb_handle(void)
{
	static int (*op_ddcb[256])(int) = {
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
		UNDOC(op_undoc_sb7ixda)		/* 0xff */
	};

	register int d;
	register int t;

	d = (signed char) memrdr(PC++);
	t = (*op_ddcb[memrdr(PC++)])(d); /* execute next opcode */

	return (t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xdd 0xcb of a 4 byte opcode.
 */
static int trap_ddcb(int data)
{
	UNUSED(data);

	cpu_error = OPTRAP4;
	cpu_state = STOPPED;
	return (0);
}

static int op_tb0ixd(int data)		/* BIT 0,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb1ixd(int data)		/* BIT 1,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb2ixd(int data)		/* BIT 2,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb3ixd(int data)		/* BIT 3,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb4ixd(int data)		/* BIT 4,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb5ixd(int data)		/* BIT 5,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return (20);
}

static int op_tb6ixd(int data)		/* BIT 6,(IX+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IX + data) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return (20);
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
	return (20);
}

static int op_rb0ixd(int data)		/* RES 0,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~1);
	return (23);
}

static int op_rb1ixd(int data)		/* RES 1,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~2);
	return (23);
}

static int op_rb2ixd(int data)		/* RES 2,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~4);
	return (23);
}

static int op_rb3ixd(int data)		/* RES 3,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~8);
	return (23);
}

static int op_rb4ixd(int data)		/* RES 4,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~16);
	return (23);
}

static int op_rb5ixd(int data)		/* RES 5,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~32);
	return (23);
}

static int op_rb6ixd(int data)		/* RES 6,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~64);
	return (23);
}

static int op_rb7ixd(int data)		/* RES 7,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) & ~128);
	return (23);
}

static int op_sb0ixd(int data)		/* SET 0,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 1);
	return (23);
}

static int op_sb1ixd(int data)		/* SET 1,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 2);
	return (23);
}

static int op_sb2ixd(int data)		/* SET 2,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 4);
	return (23);
}

static int op_sb3ixd(int data)		/* SET 3,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 8);
	return (23);
}

static int op_sb4ixd(int data)		/* SET 4,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 16);
	return (23);
}

static int op_sb5ixd(int data)		/* SET 5,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 32);
	return (23);
}

static int op_sb6ixd(int data)		/* SET 6,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 64);
	return (23);
}

static int op_sb7ixd(int data)		/* SET 7,(IX+d) */
{
	memwrt(IX + data, memrdr(IX + data) | 128);
	return (23);
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
	return (23);
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
	return (23);
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
	return (23);
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
	return (23);
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
	return (23);
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
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
	return (23);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

static int op_undoc_tb0ixd(int data)	/* BIT 0,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb0ixd(data));
}

static int op_undoc_tb1ixd(int data)	/* BIT 1,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb1ixd(data));
}

static int op_undoc_tb2ixd(int data)	/* BIT 2,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb2ixd(data));
}

static int op_undoc_tb3ixd(int data)	/* BIT 3,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb3ixd(data));
}

static int op_undoc_tb4ixd(int data)	/* BIT 4,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb4ixd(data));
}

static int op_undoc_tb5ixd(int data)	/* BIT 5,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb5ixd(data));
}

static int op_undoc_tb6ixd(int data)	/* BIT 6,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb6ixd(data));
}

static int op_undoc_tb7ixd(int data)	/* BIT 7,(IX+d) */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	return (op_tb7ixd(data));
}

static int op_undoc_rb0ixda(int data)	/* RES 0,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~1;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb1ixda(int data)	/* RES 1,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~2;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb2ixda(int data)	/* RES 2,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~4;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb3ixda(int data)	/* RES 3,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~8;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb4ixda(int data)	/* RES 4,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~16;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb5ixda(int data)	/* RES 5,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~32;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb6ixda(int data)	/* RES 6,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~64;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb7ixda(int data)	/* RES 7,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) & ~128;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_rb0ixdb(int data)	/* RES 0,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~1;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb1ixdb(int data)	/* RES 1,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~2;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb2ixdb(int data)	/* RES 2,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~4;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb3ixdb(int data)	/* RES 3,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~8;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb4ixdb(int data)	/* RES 4,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~16;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb5ixdb(int data)	/* RES 5,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~32;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb6ixdb(int data)	/* RES 6,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~64;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb7ixdb(int data)	/* RES 7,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) & ~128;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_rb0ixdc(int data)	/* RES 0,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~1;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb1ixdc(int data)	/* RES 1,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~2;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb2ixdc(int data)	/* RES 2,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~4;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb3ixdc(int data)	/* RES 3,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~8;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb4ixdc(int data)	/* RES 4,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~16;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb5ixdc(int data)	/* RES 5,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~32;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb6ixdc(int data)	/* RES 6,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~64;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb7ixdc(int data)	/* RES 7,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) & ~128;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_rb0ixdd(int data)	/* RES 0,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~1;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb1ixdd(int data)	/* RES 1,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~2;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb2ixdd(int data)	/* RES 2,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~4;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb3ixdd(int data)	/* RES 3,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~8;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb4ixdd(int data)	/* RES 4,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~16;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb5ixdd(int data)	/* RES 5,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~32;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb6ixdd(int data)	/* RES 6,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~64;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb7ixdd(int data)	/* RES 7,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) & ~128;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_rb0ixde(int data)	/* RES 0,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~1;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb1ixde(int data)	/* RES 1,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~2;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb2ixde(int data)	/* RES 2,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~4;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb3ixde(int data)	/* RES 3,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~8;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb4ixde(int data)	/* RES 4,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~16;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb5ixde(int data)	/* RES 5,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~32;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb6ixde(int data)	/* RES 6,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~64;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb7ixde(int data)	/* RES 7,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) & ~128;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_rb0ixdh(int data)	/* RES 0,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~1;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb1ixdh(int data)	/* RES 1,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~2;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb2ixdh(int data)	/* RES 2,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~4;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb3ixdh(int data)	/* RES 3,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~8;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb4ixdh(int data)	/* RES 4,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~16;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb5ixdh(int data)	/* RES 5,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~32;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb6ixdh(int data)	/* RES 6,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~64;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb7ixdh(int data)	/* RES 7,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) & ~128;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_rb0ixdl(int data)	/* RES 0,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~1;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb1ixdl(int data)	/* RES 1,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~2;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb2ixdl(int data)	/* RES 2,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~4;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb3ixdl(int data)	/* RES 3,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~8;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb4ixdl(int data)	/* RES 4,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~16;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb5ixdl(int data)	/* RES 5,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~32;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb6ixdl(int data)	/* RES 6,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~64;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rb7ixdl(int data)	/* RES 7,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) & ~128;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb0ixda(int data)	/* SET 0,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 1;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb1ixda(int data)	/* SET 1,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 2;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb2ixda(int data)	/* SET 2,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 4;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb3ixda(int data)	/* SET 3,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 8;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb4ixda(int data)	/* SET 4,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 16;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb5ixda(int data)	/* SET 5,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 32;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb6ixda(int data)	/* SET 6,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 64;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb7ixda(int data)	/* SET 7,(IX+d),A */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	A = memrdr(IX + data) | 128;
	memwrt(IX + data, A);
	return (23);
}

static int op_undoc_sb0ixdb(int data)	/* SET 0,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 1;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb1ixdb(int data)	/* SET 1,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 2;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb2ixdb(int data)	/* SET 2,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 4;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb3ixdb(int data)	/* SET 3,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 8;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb4ixdb(int data)	/* SET 4,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 16;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb5ixdb(int data)	/* SET 5,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 32;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb6ixdb(int data)	/* SET 6,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 64;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb7ixdb(int data)	/* SET 7,(IX+d),B */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	B = memrdr(IX + data) | 128;
	memwrt(IX + data, B);
	return (23);
}

static int op_undoc_sb0ixdc(int data)	/* SET 0,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 1;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb1ixdc(int data)	/* SET 1,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 2;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb2ixdc(int data)	/* SET 2,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 4;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb3ixdc(int data)	/* SET 3,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 8;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb4ixdc(int data)	/* SET 4,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 16;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb5ixdc(int data)	/* SET 5,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 32;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb6ixdc(int data)	/* SET 6,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 64;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb7ixdc(int data)	/* SET 7,(IX+d),C */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	C = memrdr(IX + data) | 128;
	memwrt(IX + data, C);
	return (23);
}

static int op_undoc_sb0ixdd(int data)	/* SET 0,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 1;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb1ixdd(int data)	/* SET 1,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 2;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb2ixdd(int data)	/* SET 2,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 4;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb3ixdd(int data)	/* SET 3,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 8;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb4ixdd(int data)	/* SET 4,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 16;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb5ixdd(int data)	/* SET 5,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 32;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb6ixdd(int data)	/* SET 6,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 64;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb7ixdd(int data)	/* SET 7,(IX+d),D */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	D = memrdr(IX + data) | 128;
	memwrt(IX + data, D);
	return (23);
}

static int op_undoc_sb0ixde(int data)	/* SET 0,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 1;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb1ixde(int data)	/* SET 1,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 2;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb2ixde(int data)	/* SET 2,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 4;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb3ixde(int data)	/* SET 3,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 8;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb4ixde(int data)	/* SET 4,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 16;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb5ixde(int data)	/* SET 5,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 32;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb6ixde(int data)	/* SET 6,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 64;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb7ixde(int data)	/* SET 7,(IX+d),E */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	E = memrdr(IX + data) | 128;
	memwrt(IX + data, E);
	return (23);
}

static int op_undoc_sb0ixdh(int data)	/* SET 0,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 1;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb1ixdh(int data)	/* SET 1,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 2;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb2ixdh(int data)	/* SET 2,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 4;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb3ixdh(int data)	/* SET 3,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 8;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb4ixdh(int data)	/* SET 4,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 16;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb5ixdh(int data)	/* SET 5,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 32;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb6ixdh(int data)	/* SET 6,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 64;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb7ixdh(int data)	/* SET 7,(IX+d),H */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	H = memrdr(IX + data) | 128;
	memwrt(IX + data, H);
	return (23);
}

static int op_undoc_sb0ixdl(int data)	/* SET 0,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 1;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb1ixdl(int data)	/* SET 1,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 2;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb2ixdl(int data)	/* SET 2,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 4;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb3ixdl(int data)	/* SET 3,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 8;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb4ixdl(int data)	/* SET 4,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 16;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb5ixdl(int data)	/* SET 5,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 32;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb6ixdl(int data)	/* SET 6,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 64;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_sb7ixdl(int data)	/* SET 7,(IX+d),L */
{
	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	L = memrdr(IX + data) | 128;
	memwrt(IX + data, L);
	return (23);
}

static int op_undoc_rlcixda(int data)	/* RLC (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	i = A & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (i) A |= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixdb(int data)	/* RLC (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	i = B & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (i) B |= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixdc(int data)	/* RLC (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	i = C & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (i) C |= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixdd(int data)	/* RLC (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	i = D & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (i) D |= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixde(int data)	/* RLC (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	i = E & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (i) E |= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixdh(int data)	/* RLC (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	i = H & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (i) H |= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlcixdl(int data)	/* RLC (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	i = L & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (i) L |= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixda(int data)	/* RRC (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (i) A |= 128;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixdb(int data)	/* RRC (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	i = B & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (i) B |= 128;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixdc(int data)	/* RRC (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	i = C & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (i) C |= 128;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixdd(int data)	/* RRC (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	i = D & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (i) D |= 128;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixde(int data)	/* RRC (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	i = E & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (i) E |= 128;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixdh(int data)	/* RRC (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	i = H & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (i) H |= 128;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrcixdl(int data)	/* RRC (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	i = L & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (i) L |= 128;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixda(int data)	/* RL (IX+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixdb(int data)	/* RL (IX+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (old_c_flag) B |= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixdc(int data)	/* RL (IX+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (old_c_flag) C |= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixdd(int data)	/* RL (IX+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (old_c_flag) D |= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixde(int data)	/* RL (IX+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (old_c_flag) E |= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixdh(int data)	/* RL (IX+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (old_c_flag) H |= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rlixdl(int data)	/* RL (IX+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (old_c_flag) L |= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixda(int data)	/* RR (IX+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixdb(int data)	/* RR (IX+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (old_c_flag) B |= 128;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixdc(int data)	/* RR (IX+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (old_c_flag) C |= 128;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixdd(int data)	/* RR (IX+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (old_c_flag) D |= 128;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixde(int data)	/* RR (IX+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (old_c_flag) E |= 128;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixdh(int data)	/* RR (IX+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (old_c_flag) H |= 128;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_rrixdl(int data)	/* RR (IX+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (old_c_flag) L |= 128;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixda(int data)	/* SLA (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixdb(int data)	/* SLA (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixdc(int data)	/* SLA (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixdd(int data)	/* SLA (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixde(int data)	/* SLA (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixdh(int data)	/* SLA (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_slaixdl(int data)	/* SLA (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixda(int data)	/* SRA (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	i = A & 128;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A >> 1) | i;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixdb(int data)	/* SRA (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	i = B & 128;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = (B >> 1) | i;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixdc(int data)	/* SRA (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	i = C & 128;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = (C >> 1) | i;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixdd(int data)	/* SRA (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	i = D & 128;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = (D >> 1) | i;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixde(int data)	/* SRA (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	i = E & 128;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = (E >> 1) | i;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixdh(int data)	/* SRA (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	i = H & 128;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = (H >> 1) | i;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sraixdl(int data)	/* SRA (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	i = L & 128;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = (L >> 1) | i;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixda(int data)	/* SLL (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A << 1) | 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixdb(int data)	/* SLL (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = (B << 1) | 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixdc(int data)	/* SLL (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = (C << 1) | 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixdd(int data)	/* SLL (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = (D << 1) | 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixde(int data)	/* SLL (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = (E << 1) | 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixdh(int data)	/* SLL (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = (H << 1) | 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixdl(int data)	/* SLL (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = (L << 1) | 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_sllixd(int data)	/* SLL (IX+d) */
{
	register BYTE P;
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P << 1) | 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixda(int data)	/* SRL (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	A = memrdr(addr);
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixdb(int data)	/* SRL (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	B = memrdr(addr);
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixdc(int data)	/* SRL (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	C = memrdr(addr);
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixdd(int data)	/* SRL (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	D = memrdr(addr);
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixde(int data)	/* SRL (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	E = memrdr(addr);
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixdh(int data)	/* SRL (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	H = memrdr(addr);
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

static int op_undoc_srlixdl(int data)	/* SRL (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		trap_ddcb(0);
		return (0);
	}

	addr = IX + data;
	L = memrdr(addr);
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (23);
}

#endif

#endif
