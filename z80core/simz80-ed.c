/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	Like the function "cpu_z80()" this one emulates multi byte opcodes
 *	starting with 0xed
 */

#include <stdio.h>
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
#define UNDOC(f) trap_ed
#endif

static int trap_ed(void);
static int op_im0(void), op_im1(void), op_im2(void);
static int op_reti(void), op_retn(void);
static int op_neg(void);
static int op_inaic(void), op_inbic(void), op_incic(void);
static int op_indic(void), op_ineic(void);
static int op_inhic(void), op_inlic(void);
static int op_outca(void), op_outcb(void), op_outcc(void);
static int op_outcd(void), op_outce(void);
static int op_outch(void), op_outcl(void);
static int op_ini(void), op_inir(void), op_ind(void), op_indr(void);
static int op_outi(void), op_otir(void), op_outd(void), op_otdr(void);
static int op_ldai(void), op_ldar(void), op_ldia(void), op_ldra(void);
static int op_ldbcinn(void), op_lddeinn(void);
static int op_ldhlinn(void), op_ldspinn(void);
static int op_ldinbc(void), op_ldinde(void), op_ldinhl(void), op_ldinsp(void);
static int op_adchb(void), op_adchd(void), op_adchh(void), op_adchs(void);
static int op_sbchb(void), op_sbchd(void), op_sbchh(void), op_sbchs(void);
static int op_ldi(void), op_ldir(void), op_ldd(void), op_lddr(void);
static int op_cpi(void), op_cpir(void), op_cpdop(void), op_cpdr(void);
static int op_oprld(void), op_oprrd(void);

#ifdef UNDOC_INST
static int op_undoc_outc0(void), op_undoc_infic(void);
static int op_undoc_nop(void);
static int op_undoc_im0(void), op_undoc_im1(void), op_undoc_im2(void);
static int op_undoc_reti(void), op_undoc_retn(void);
static int op_undoc_neg(void);
#endif

int op_ed_handle(void)
{
	register int t;

	static int (*op_ed[256])(void) = {
		UNDOC(op_undoc_nop),		/* 0x00 */
		UNDOC(op_undoc_nop),		/* 0x01 */
		UNDOC(op_undoc_nop),		/* 0x02 */
		UNDOC(op_undoc_nop),		/* 0x03 */
		UNDOC(op_undoc_nop),		/* 0x04 */
		UNDOC(op_undoc_nop),		/* 0x05 */
		UNDOC(op_undoc_nop),		/* 0x06 */
		UNDOC(op_undoc_nop),		/* 0x07 */
		UNDOC(op_undoc_nop),		/* 0x08 */
		UNDOC(op_undoc_nop),		/* 0x09 */
		UNDOC(op_undoc_nop),		/* 0x0a */
		UNDOC(op_undoc_nop),		/* 0x0b */
		UNDOC(op_undoc_nop),		/* 0x0c */
		UNDOC(op_undoc_nop),		/* 0x0d */
		UNDOC(op_undoc_nop),		/* 0x0e */
		UNDOC(op_undoc_nop),		/* 0x0f */
		UNDOC(op_undoc_nop),		/* 0x10 */
		UNDOC(op_undoc_nop),		/* 0x11 */
		UNDOC(op_undoc_nop),		/* 0x12 */
		UNDOC(op_undoc_nop),		/* 0x13 */
		UNDOC(op_undoc_nop),		/* 0x14 */
		UNDOC(op_undoc_nop),		/* 0x15 */
		UNDOC(op_undoc_nop),		/* 0x16 */
		UNDOC(op_undoc_nop),		/* 0x17 */
		UNDOC(op_undoc_nop),		/* 0x18 */
		UNDOC(op_undoc_nop),		/* 0x19 */
		UNDOC(op_undoc_nop),		/* 0x1a */
		UNDOC(op_undoc_nop),		/* 0x1b */
		UNDOC(op_undoc_nop),		/* 0x1c */
		UNDOC(op_undoc_nop),		/* 0x1d */
		UNDOC(op_undoc_nop),		/* 0x1e */
		UNDOC(op_undoc_nop),		/* 0x1f */
		UNDOC(op_undoc_nop),		/* 0x20 */
		UNDOC(op_undoc_nop),		/* 0x21 */
		UNDOC(op_undoc_nop),		/* 0x22 */
		UNDOC(op_undoc_nop),		/* 0x23 */
		UNDOC(op_undoc_nop),		/* 0x24 */
		UNDOC(op_undoc_nop),		/* 0x25 */
		UNDOC(op_undoc_nop),		/* 0x26 */
		UNDOC(op_undoc_nop),		/* 0x27 */
		UNDOC(op_undoc_nop),		/* 0x28 */
		UNDOC(op_undoc_nop),		/* 0x29 */
		UNDOC(op_undoc_nop),		/* 0x2a */
		UNDOC(op_undoc_nop),		/* 0x2b */
		UNDOC(op_undoc_nop),		/* 0x2c */
		UNDOC(op_undoc_nop),		/* 0x2d */
		UNDOC(op_undoc_nop),		/* 0x2e */
		UNDOC(op_undoc_nop),		/* 0x2f */
		UNDOC(op_undoc_nop),		/* 0x30 */
		UNDOC(op_undoc_nop),		/* 0x31 */
		UNDOC(op_undoc_nop),		/* 0x32 */
		UNDOC(op_undoc_nop),		/* 0x33 */
		UNDOC(op_undoc_nop),		/* 0x34 */
		UNDOC(op_undoc_nop),		/* 0x35 */
		UNDOC(op_undoc_nop),		/* 0x36 */
		UNDOC(op_undoc_nop),		/* 0x37 */
		UNDOC(op_undoc_nop),		/* 0x38 */
		UNDOC(op_undoc_nop),		/* 0x39 */
		UNDOC(op_undoc_nop),		/* 0x3a */
		UNDOC(op_undoc_nop),		/* 0x3b */
		UNDOC(op_undoc_nop),		/* 0x3c */
		UNDOC(op_undoc_nop),		/* 0x3d */
		UNDOC(op_undoc_nop),		/* 0x3e */
		UNDOC(op_undoc_nop),		/* 0x3f */
		op_inbic,			/* 0x40 */
		op_outcb,			/* 0x41 */
		op_sbchb,			/* 0x42 */
		op_ldinbc,			/* 0x43 */
		op_neg,				/* 0x44 */
		op_retn,			/* 0x45 */
		op_im0,				/* 0x46 */
		op_ldia,			/* 0x47 */
		op_incic,			/* 0x48 */
		op_outcc,			/* 0x49 */
		op_adchb,			/* 0x4a */
		op_ldbcinn,			/* 0x4b */
		UNDOC(op_undoc_neg),		/* 0x4c */
		op_reti,			/* 0x4d */
		UNDOC(op_undoc_im0),		/* 0x4e */
		op_ldra,			/* 0x4f */
		op_indic,			/* 0x50 */
		op_outcd,			/* 0x51 */
		op_sbchd,			/* 0x52 */
		op_ldinde,			/* 0x53 */
		UNDOC(op_undoc_neg),		/* 0x54 */
		UNDOC(op_undoc_retn),		/* 0x55 */
		op_im1,				/* 0x56 */
		op_ldai,			/* 0x57 */
		op_ineic,			/* 0x58 */
		op_outce,			/* 0x59 */
		op_adchd,			/* 0x5a */
		op_lddeinn,			/* 0x5b */
		UNDOC(op_undoc_neg),		/* 0x5c */
		UNDOC(op_undoc_reti),		/* 0x5d */
		op_im2,				/* 0x5e */
		op_ldar,			/* 0x5f */
		op_inhic,			/* 0x60 */
		op_outch,			/* 0x61 */
		op_sbchh,			/* 0x62 */
		op_ldinhl,			/* 0x63 */
		UNDOC(op_undoc_neg),		/* 0x64 */
		UNDOC(op_undoc_retn),		/* 0x65 */
		UNDOC(op_undoc_im0),		/* 0x66 */
		op_oprrd,			/* 0x67 */
		op_inlic,			/* 0x68 */
		op_outcl,			/* 0x69 */
		op_adchh,			/* 0x6a */
		op_ldhlinn,			/* 0x6b */
		UNDOC(op_undoc_neg),		/* 0x6c */
		UNDOC(op_undoc_reti),		/* 0x6d */
		UNDOC(op_undoc_im0),		/* 0x6e */
		op_oprld,			/* 0x6f */
		UNDOC(op_undoc_infic),		/* 0x70 */
		UNDOC(op_undoc_outc0),		/* 0x71 */
		op_sbchs,			/* 0x72 */
		op_ldinsp,			/* 0x73 */
		UNDOC(op_undoc_neg),		/* 0x74 */
		UNDOC(op_undoc_retn),		/* 0x75 */
		UNDOC(op_undoc_im1),		/* 0x76 */
		UNDOC(op_undoc_nop),		/* 0x77 */
		op_inaic,			/* 0x78 */
		op_outca,			/* 0x79 */
		op_adchs,			/* 0x7a */
		op_ldspinn,			/* 0x7b */
		UNDOC(op_undoc_neg),		/* 0x7c */
		UNDOC(op_undoc_reti),		/* 0x7d */
		UNDOC(op_undoc_im2),		/* 0x7e */
		UNDOC(op_undoc_nop),		/* 0x7f */
		UNDOC(op_undoc_nop),		/* 0x80 */
		UNDOC(op_undoc_nop),		/* 0x81 */
		UNDOC(op_undoc_nop),		/* 0x82 */
		UNDOC(op_undoc_nop),		/* 0x83 */
		UNDOC(op_undoc_nop),		/* 0x84 */
		UNDOC(op_undoc_nop),		/* 0x85 */
		UNDOC(op_undoc_nop),		/* 0x86 */
		UNDOC(op_undoc_nop),		/* 0x87 */
		UNDOC(op_undoc_nop),		/* 0x88 */
		UNDOC(op_undoc_nop),		/* 0x89 */
		UNDOC(op_undoc_nop),		/* 0x8a */
		UNDOC(op_undoc_nop),		/* 0x8b */
		UNDOC(op_undoc_nop),		/* 0x8c */
		UNDOC(op_undoc_nop),		/* 0x8d */
		UNDOC(op_undoc_nop),		/* 0x8e */
		UNDOC(op_undoc_nop),		/* 0x8f */
		UNDOC(op_undoc_nop),		/* 0x90 */
		UNDOC(op_undoc_nop),		/* 0x91 */
		UNDOC(op_undoc_nop),		/* 0x92 */
		UNDOC(op_undoc_nop),		/* 0x93 */
		UNDOC(op_undoc_nop),		/* 0x94 */
		UNDOC(op_undoc_nop),		/* 0x95 */
		UNDOC(op_undoc_nop),		/* 0x96 */
		UNDOC(op_undoc_nop),		/* 0x97 */
		UNDOC(op_undoc_nop),		/* 0x98 */
		UNDOC(op_undoc_nop),		/* 0x99 */
		UNDOC(op_undoc_nop),		/* 0x9a */
		UNDOC(op_undoc_nop),		/* 0x9b */
		UNDOC(op_undoc_nop),		/* 0x9c */
		UNDOC(op_undoc_nop),		/* 0x9d */
		UNDOC(op_undoc_nop),		/* 0x9e */
		UNDOC(op_undoc_nop),		/* 0x9f */
		op_ldi,				/* 0xa0 */
		op_cpi,				/* 0xa1 */
		op_ini,				/* 0xa2 */
		op_outi,			/* 0xa3 */
		UNDOC(op_undoc_nop),		/* 0xa4 */
		UNDOC(op_undoc_nop),		/* 0xa5 */
		UNDOC(op_undoc_nop),		/* 0xa6 */
		UNDOC(op_undoc_nop),		/* 0xa7 */
		op_ldd,				/* 0xa8 */
		op_cpdop,			/* 0xa9 */
		op_ind,				/* 0xaa */
		op_outd,			/* 0xab */
		UNDOC(op_undoc_nop),		/* 0xac */
		UNDOC(op_undoc_nop),		/* 0xad */
		UNDOC(op_undoc_nop),		/* 0xae */
		UNDOC(op_undoc_nop),		/* 0xaf */
		op_ldir,			/* 0xb0 */
		op_cpir,			/* 0xb1 */
		op_inir,			/* 0xb2 */
		op_otir,			/* 0xb3 */
		UNDOC(op_undoc_nop),		/* 0xb4 */
		UNDOC(op_undoc_nop),		/* 0xb5 */
		UNDOC(op_undoc_nop),		/* 0xb6 */
		UNDOC(op_undoc_nop),		/* 0xb7 */
		op_lddr,			/* 0xb8 */
		op_cpdr,			/* 0xb9 */
		op_indr,			/* 0xba */
		op_otdr,			/* 0xbb */
		UNDOC(op_undoc_nop),		/* 0xbc */
		UNDOC(op_undoc_nop),		/* 0xbd */
		UNDOC(op_undoc_nop),		/* 0xbe */
		UNDOC(op_undoc_nop),		/* 0xbf */
		UNDOC(op_undoc_nop),		/* 0xc0 */
		UNDOC(op_undoc_nop),		/* 0xc1 */
		UNDOC(op_undoc_nop),		/* 0xc2 */
		UNDOC(op_undoc_nop),		/* 0xc3 */
		UNDOC(op_undoc_nop),		/* 0xc4 */
		UNDOC(op_undoc_nop),		/* 0xc5 */
		UNDOC(op_undoc_nop),		/* 0xc6 */
		UNDOC(op_undoc_nop),		/* 0xc7 */
		UNDOC(op_undoc_nop),		/* 0xc8 */
		UNDOC(op_undoc_nop),		/* 0xc9 */
		UNDOC(op_undoc_nop),		/* 0xca */
		UNDOC(op_undoc_nop),		/* 0xcb */
		UNDOC(op_undoc_nop),		/* 0xcc */
		UNDOC(op_undoc_nop),		/* 0xcd */
		UNDOC(op_undoc_nop),		/* 0xce */
		UNDOC(op_undoc_nop),		/* 0xcf */
		UNDOC(op_undoc_nop),		/* 0xd0 */
		UNDOC(op_undoc_nop),		/* 0xd1 */
		UNDOC(op_undoc_nop),		/* 0xd2 */
		UNDOC(op_undoc_nop),		/* 0xd3 */
		UNDOC(op_undoc_nop),		/* 0xd4 */
		UNDOC(op_undoc_nop),		/* 0xd5 */
		UNDOC(op_undoc_nop),		/* 0xd6 */
		UNDOC(op_undoc_nop),		/* 0xd7 */
		UNDOC(op_undoc_nop),		/* 0xd8 */
		UNDOC(op_undoc_nop),		/* 0xd9 */
		UNDOC(op_undoc_nop),		/* 0xda */
		UNDOC(op_undoc_nop),		/* 0xdb */
		UNDOC(op_undoc_nop),		/* 0xdc */
		UNDOC(op_undoc_nop),		/* 0xdd */
		UNDOC(op_undoc_nop),		/* 0xde */
		UNDOC(op_undoc_nop),		/* 0xdf */
		UNDOC(op_undoc_nop),		/* 0xe0 */
		UNDOC(op_undoc_nop),		/* 0xe1 */
		UNDOC(op_undoc_nop),		/* 0xe2 */
		UNDOC(op_undoc_nop),		/* 0xe3 */
		UNDOC(op_undoc_nop),		/* 0xe4 */
		UNDOC(op_undoc_nop),		/* 0xe5 */
		UNDOC(op_undoc_nop),		/* 0xe6 */
		UNDOC(op_undoc_nop),		/* 0xe7 */
		UNDOC(op_undoc_nop),		/* 0xe8 */
		UNDOC(op_undoc_nop),		/* 0xe9 */
		UNDOC(op_undoc_nop),		/* 0xea */
		UNDOC(op_undoc_nop),		/* 0xeb */
		UNDOC(op_undoc_nop),		/* 0xec */
		UNDOC(op_undoc_nop),		/* 0xed */
		UNDOC(op_undoc_nop),		/* 0xee */
		UNDOC(op_undoc_nop),		/* 0xef */
		UNDOC(op_undoc_nop),		/* 0xf0 */
		UNDOC(op_undoc_nop),		/* 0xf1 */
		UNDOC(op_undoc_nop),		/* 0xf2 */
		UNDOC(op_undoc_nop),		/* 0xf3 */
		UNDOC(op_undoc_nop),		/* 0xf4 */
		UNDOC(op_undoc_nop),		/* 0xf5 */
		UNDOC(op_undoc_nop),		/* 0xf6 */
		UNDOC(op_undoc_nop),		/* 0xf7 */
		UNDOC(op_undoc_nop),		/* 0xf8 */
		UNDOC(op_undoc_nop),		/* 0xf9 */
		UNDOC(op_undoc_nop),		/* 0xfa */
		UNDOC(op_undoc_nop),		/* 0xfb */
		UNDOC(op_undoc_nop),		/* 0xfc */
		UNDOC(op_undoc_nop),		/* 0xfd */
		UNDOC(op_undoc_nop),		/* 0xfe */
		UNDOC(op_undoc_nop)		/* 0xff */
	};

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	/* update frontpanel */
	fp_clock++;
	fp_sampleLightGroup(0, 0);
#endif

	R++;				/* increment refresh register */

	t = (*op_ed[memrdr(PC++)])();	/* execute next opcode */

	return (t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xed of a multi byte opcode.
 */
static int trap_ed(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

static int op_im0(void)			/* IM 0 */
{
	int_mode = 0;
	return (8);
}

static int op_im1(void)			/* IM 1 */
{
	int_mode = 1;
	return (8);
}

static int op_im2(void)			/* IM 2 */
{
	int_mode = 2;
	return (8);
}

static int op_reti(void)		/* RETI */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	return (14);
}

static int op_retn(void)		/* RETN */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	if (IFF & 2)
		IFF |= 1;
	return (14);
}

static int op_neg(void)			/* NEG */
{
	(A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	(A == 0x80) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(0 - ((signed char) A & 0xf) < 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A = 0 - A;
	F |= N_FLAG;
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (8);
}

static int op_inaic(void)		/* IN A,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	A = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_inbic(void)		/* IN B,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	B = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_incic(void)		/* IN C,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	C = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_indic(void)		/* IN D,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	D = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_ineic(void)		/* IN E,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	E = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_inhic(void)		/* IN H,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	H = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_inlic(void)		/* IN L,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

	L = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_outca(void)		/* OUT (C),A */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, A);
	return (12);
}

static int op_outcb(void)		/* OUT (C),B */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, B);
	return (12);
}

static int op_outcc(void)		/* OUT (C),C */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, C);
	return (12);
}

static int op_outcd(void)		/* OUT (C),D */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, D);
	return (12);
}

static int op_outce(void)		/* OUT (C),E */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, E);
	return (12);
}

static int op_outch(void)		/* OUT (C),H */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, H);
	return (12);
}

static int op_outcl(void)		/* OUT (C),L */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, L);
	return (12);
}

static int op_ini(void)			/* INI */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif

	data = io_in(C, B);
	memwrt((H << 8) + L, data);
	L++;
	if (!L)
		H++;
	B--;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) ((C + 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_inir(void)		/* INIR */
{
	extern BYTE io_in(BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif
	register int t = -21;

	addr = (H << 8) + L;
	R -= 2;
	do {
		data = io_in(C, B);
		memwrt(addr++, data);
		B--;
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) ((C + 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
	return (t + 16);
}
#else
static int op_inir(void)		/* INIR */
{
	register int t;

	op_ini();
	if (!(F & Z_FLAG)) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_ind(void)			/* IND */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif

	data = io_in(C, B);
	memwrt((H << 8) + L, data);
	L--;
	if (L == 0xff)
		H--;
	B--;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) ((C - 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_indr(void)		/* INDR */
{
	extern BYTE io_in(BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif
	register int t = -21;

	addr = (H << 8) + L;
	R -= 2;
	do {
		data = io_in(C, B);
		memwrt(addr--, data);
		B--;
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) ((C - 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
	return (t + 16);
}
#else
static int op_indr(void)		/* INDR */
{
	register int t;

	op_ind();
	if (!(F & Z_FLAG)) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_outi(void)		/* OUTI */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif

	B--;
	data = memrdr((H << 8) + L);
	io_out(C, B, data);
	L++;
	if (!L)
		H++;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_otir(void)		/* OTIR */
{
	extern void io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif
	register int t = -21;

	addr = (H << 8) + L;
	R -= 2;
	do {
		B--;
		data = memrdr(addr++);
		io_out(C, B, data);
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
	return (t + 16);
}
#else
static int op_otir(void)		/* OTIR */
{
	register int t;

	op_outi();
	if (!(F & Z_FLAG)) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_outd(void)		/* OUTD */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif

	B--;
	data = memrdr((H << 8) + L);
	io_out(C, B, data);
	L--;
	if (L == 0xff)
		H--;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_otdr(void)		/* OTDR */
{
	extern void io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef Z80BLKIOF_DOC
	WORD k;
#endif
	register int t = -21;

	addr = (H << 8) + L;
	R -= 2;
	do {
		B--;
		data = memrdr(addr--);
		io_out(C, B, data);
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#ifdef Z80BLKIOF_DOC
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
	return (t + 16);
}
#else
static int op_otdr(void)		/* OTDR */
{
	register int t;

	op_outd();
	if (!(F & Z_FLAG)) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_ldai(void)		/* LD A,I */
{
	A = I;
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (9);
}

static int op_ldar(void)		/* LD A,R */
{
	A = (R_ & 0x80) | (R & 0x7f);
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (9);
}

static int op_ldia(void)		/* LD I,A */
{
	I = A;
	return (9);
}

static int op_ldra(void)		/* LD R,A */
{
	R_ = R = A;
	return (9);
}

static int op_ldbcinn(void)		/* LD BC,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	C = memrdr(i++);
	B = memrdr(i);
	return (20);
}

static int op_lddeinn(void)		/* LD DE,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	E = memrdr(i++);
	D = memrdr(i);
	return (20);
}

static int op_ldhlinn(void)		/* LD HL,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i++);
	H = memrdr(i);
	return (20);
}

static int op_ldspinn(void)		/* LD SP,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	SP = memrdr(i++);
	SP += memrdr(i) << 8;
	return (20);
}

static int op_ldinbc(void)		/* LD (nn),BC */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, C);
	memwrt(i, B);
	return (20);
}

static int op_ldinde(void)		/* LD (nn),DE */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, E);
	memwrt(i, D);
	return (20);
}

static int op_ldinhl(void)		/* LD (nn),HL */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, L);
	memwrt(i, H);
	return (20);
}

static int op_ldinsp(void)		/* LD (nn),SP */
{
	register WORD addr;
	WORD i;

	addr = memrdr(PC++);
	addr += memrdr(PC++) << 8;
	i = SP;
	memwrt(addr++, i);
	memwrt(addr, i >> 8);
	return (20);
}

static int op_adchb(void)		/* ADC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H << 8) + L;
	bc = (B << 8) + C;
	shl = hl;
	sbc = bc;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (bc & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + sbc + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + bc + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return (15);
}

static int op_adchd(void)		/* ADC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H << 8) + L;
	de = (D << 8) + E;
	shl = hl;
	sde = de;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (de & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + sde + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + de + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return (15);
}

static int op_adchh(void)		/* ADC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H << 8) + L;
	shl = hl;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (hl & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + shl + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + hl + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return (15);
}

static int op_adchs(void)		/* ADC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H << 8) + L;
	sp = SP;
	shl = hl;
	ssp = sp;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (sp & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + ssp + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + sp + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return (15);
}

static int op_sbchb(void)		/* SBC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H << 8) + L;
	bc = (B << 8) + C;
	shl = hl;
	sbc = bc;
	carry = (F & C_FLAG) ? 1 : 0;
	(((bc & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - sbc - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(bc + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return (15);
}

static int op_sbchd(void)		/* SBC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H << 8) + L;
	de = (D << 8) + E;
	shl = hl;
	sde = de;
	carry = (F & C_FLAG) ? 1 : 0;
	(((de & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - sde - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(de + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return (15);
}

static int op_sbchh(void)		/* SBC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H << 8) + L;
	shl = hl;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - shl - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return (15);
}

static int op_sbchs(void)		/* SBC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H << 8) + L;
	sp = SP;
	shl = hl;
	ssp = sp;
	carry = (F & C_FLAG) ? 1 : 0;
	(((sp & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - ssp - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(sp + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return (15);
}

static int op_ldi(void)			/* LDI */
{
	memwrt((D << 8) + E, memrdr((H << 8) + L));
	E++;
	if (!E)
		D++;
	L++;
	if (!L)
		H++;
	C--;
	if (C == 0xff)
		B--;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	F &= ~(N_FLAG | H_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_ldir(void)		/* LDIR */
{
	register int t = -21;
	register WORD i;
	register WORD s, d;

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	R -= 2;
	do {
		memwrt(d++, memrdr(s++));
		t += 21;
		R += 2;
	} while (--i);
	B = C = 0;
	D = d >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
	return (t + 16);
}
#else
static int op_ldir(void)		/* LDIR */
{
	register int t;

	op_ldi();
	if (F & P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_ldd(void)			/* LDD */
{
	memwrt((D << 8) + E, memrdr((H << 8) + L));
	E--;
	if (E == 0xff)
		D--;
	L--;
	if (L == 0xff)
		H--;
	C--;
	if (C == 0xff)
		B--;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	F &= ~(N_FLAG | H_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_lddr(void)		/* LDDR */
{
	register int t = -21;
	register WORD i;
	register WORD s, d;

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	R -= 2;
	do {
		memwrt(d--, memrdr(s--));
		t += 21;
		R += 2;
	} while (--i);
	B = C = 0;
	D = d >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
	return (t + 16);
}
#else
static int op_lddr(void)		/* LDDR */
{
	register int t;

	op_ldd();
	if (F & P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_cpi(void)			/* CPI */
{
	register BYTE i;

	i = memrdr(((H << 8) + L));
	((i & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	i = A - i;
	L++;
	if (!L)
		H++;
	C--;
	if (C == 0xff)
		B--;
	F |= N_FLAG;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_cpir(void)		/* CPIR */
{
	register int t = -21;
	register WORD s;
	register BYTE d;
	register WORD i;
	register BYTE tmp;

	i = (B << 8) + C;
	s = (H << 8) + L;
	R -= 2;
	do {
		tmp = memrdr(s++);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
		R += 2;
	} while (--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (t + 16);
}
#else
static int op_cpir(void)		/* CPIR */
{
	register int t;

	op_cpi();
	if ((F & (P_FLAG | Z_FLAG)) == P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_cpdop(void)		/* CPD */
{
	register BYTE i;

	i = memrdr(((H << 8) + L));
	((i & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	i = A - i;
	L--;
	if (L == 0xff)
		H--;
	C--;
	if (C == 0xff)
		B--;
	F |= N_FLAG;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (16);
}

#ifdef WANT_FASTB
static int op_cpdr(void)		/* CPDR */
{
	register int t = -21;
	register WORD s;
	register BYTE d;
	register WORD i;
	register BYTE tmp;

	i = (B << 8) + C;
	s = (H << 8) + L;
	R -= 2;
	do {
		tmp = memrdr(s--);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
		R += 2;
	} while (--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return (t + 16);
}
#else
static int op_cpdr(void)		/* CPDR */
{
	register int t;

	op_cpdop();
	if ((F & (P_FLAG | Z_FLAG)) == P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return (t);
}
#endif

static int op_oprld(void)		/* RLD (HL) */
{
	register BYTE i, j;

	i = memrdr((H << 8) + L);
	j = A & 0x0f;
	A = (A & 0xf0) | (i >> 4);
	i = (i << 4) | j;
	memwrt((H << 8) + L, i);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (18);
}

static int op_oprrd(void)		/* RRD (HL) */
{
	register BYTE i, j;

	i = memrdr((H << 8) + L);
	j = A & 0x0f;
	A = (A & 0xf0) | (i & 0x0f);
	i = (i >> 4) | (j << 4);
	memwrt((H << 8) + L, i);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (18);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

static int op_undoc_outc0(void)		/* OUT (C),0 */
{
	extern void io_out(BYTE, BYTE, BYTE);

	if (u_flag)
		return (trap_ed());

	io_out(C, B, 0); /* NMOS, CMOS outputs 0xff */
	return (12);
}

static int op_undoc_infic(void)		/* IN F,(C) */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE tmp;

	if (u_flag)
		return (trap_ed());

	tmp = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(tmp) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(tmp & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[tmp]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return (12);
}

static int op_undoc_nop(void)		/* NOP */
{
	if (u_flag)
		return (trap_ed());

	return (8);
}

static int op_undoc_im0(void)		/* IM 0 */
{
	if (u_flag)
		return (trap_ed());

	return (op_im0());
}

static int op_undoc_im1(void)		/* IM 1 */
{
	if (u_flag)
		return (trap_ed());

	return (op_im1());
}

static int op_undoc_im2(void)		/* IM 2 */
{
	if (u_flag)
		return (trap_ed());

	return (op_im2());
}

static int op_undoc_reti(void)		/* RETI */
{
	if (u_flag)
		return (trap_ed());

	return (op_reti());
}

static int op_undoc_retn(void)		/* RETN */
{
	if (u_flag)
		return (trap_ed());

	return (op_retn());
}

static int op_undoc_neg(void)		/* NEG */
{
	if (u_flag)
		return (trap_ed());

	return (op_neg());
}

#endif

#endif
