/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	Like the function "cpu_z80()" this one emulates 4 byte opcodes
 *	starting with 0xfd 0xcb
 */

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simz80-fdcb.h"

#if !defined(EXCLUDE_Z80) && !defined(ALT_Z80)

static int trap_fdcb(int data);
static int op_tb0iyd(int data), op_tb1iyd(int data), op_tb2iyd(int data);
static int op_tb3iyd(int data), op_tb4iyd(int data), op_tb5iyd(int data);
static int op_tb6iyd(int data), op_tb7iyd(int data);
static int op_rb0iyd(int data), op_rb1iyd(int data), op_rb2iyd(int data);
static int op_rb3iyd(int data), op_rb4iyd(int data), op_rb5iyd(int data);
static int op_rb6iyd(int data), op_rb7iyd(int data);
static int op_sb0iyd(int data), op_sb1iyd(int data), op_sb2iyd(int data);
static int op_sb3iyd(int data), op_sb4iyd(int data), op_sb5iyd(int data);
static int op_sb6iyd(int data), op_sb7iyd(int data);
static int op_rlciyd(int data), op_rrciyd(int data), op_rliyd(int data);
static int op_rriyd(int data), op_slaiyd(int data), op_sraiyd(int data);
static int op_srliyd(int data);

#ifdef UNDOC_INST
static int op_undoc_slliyd(int data);
#endif /* UNDOC_INST */

int op_fdcb_handle(void)
{

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_fdcb
#endif

	static int (*op_fdcb[256])(int) = {
		trap_fdcb,			/* 0x00 */
		trap_fdcb,			/* 0x01 */
		trap_fdcb,			/* 0x02 */
		trap_fdcb,			/* 0x03 */
		trap_fdcb,			/* 0x04 */
		trap_fdcb,			/* 0x05 */
		op_rlciyd,			/* 0x06 */
		trap_fdcb,			/* 0x07 */
		trap_fdcb,			/* 0x08 */
		trap_fdcb,			/* 0x09 */
		trap_fdcb,			/* 0x0a */
		trap_fdcb,			/* 0x0b */
		trap_fdcb,			/* 0x0c */
		trap_fdcb,			/* 0x0d */
		op_rrciyd,			/* 0x0e */
		trap_fdcb,			/* 0x0f */
		trap_fdcb,			/* 0x10 */
		trap_fdcb,			/* 0x11 */
		trap_fdcb,			/* 0x12 */
		trap_fdcb,			/* 0x13 */
		trap_fdcb,			/* 0x14 */
		trap_fdcb,			/* 0x15 */
		op_rliyd,			/* 0x16 */
		trap_fdcb,			/* 0x17 */
		trap_fdcb,			/* 0x18 */
		trap_fdcb,			/* 0x19 */
		trap_fdcb,			/* 0x1a */
		trap_fdcb,			/* 0x1b */
		trap_fdcb,			/* 0x1c */
		trap_fdcb,			/* 0x1d */
		op_rriyd,			/* 0x1e */
		trap_fdcb,			/* 0x1f */
		trap_fdcb,			/* 0x20 */
		trap_fdcb,			/* 0x21 */
		trap_fdcb,			/* 0x22 */
		trap_fdcb,			/* 0x23 */
		trap_fdcb,			/* 0x24 */
		trap_fdcb,			/* 0x25 */
		op_slaiyd,			/* 0x26 */
		trap_fdcb,			/* 0x27 */
		trap_fdcb,			/* 0x28 */
		trap_fdcb,			/* 0x29 */
		trap_fdcb,			/* 0x2a */
		trap_fdcb,			/* 0x2b */
		trap_fdcb,			/* 0x2c */
		trap_fdcb,			/* 0x2d */
		op_sraiyd,			/* 0x2e */
		trap_fdcb,			/* 0x2f */
		trap_fdcb,			/* 0x30 */
		trap_fdcb,			/* 0x31 */
		trap_fdcb,			/* 0x32 */
		trap_fdcb,			/* 0x33 */
		trap_fdcb,			/* 0x34 */
		trap_fdcb,			/* 0x35 */
		UNDOC(op_undoc_slliyd),		/* 0x36 */
		trap_fdcb,			/* 0x37 */
		trap_fdcb,			/* 0x38 */
		trap_fdcb,			/* 0x39 */
		trap_fdcb,			/* 0x3a */
		trap_fdcb,			/* 0x3b */
		trap_fdcb,			/* 0x3c */
		trap_fdcb,			/* 0x3d */
		op_srliyd,			/* 0x3e */
		trap_fdcb,			/* 0x3f */
		trap_fdcb,			/* 0x40 */
		trap_fdcb,			/* 0x41 */
		trap_fdcb,			/* 0x42 */
		trap_fdcb,			/* 0x43 */
		trap_fdcb,			/* 0x44 */
		trap_fdcb,			/* 0x45 */
		op_tb0iyd,			/* 0x46 */
		trap_fdcb,			/* 0x47 */
		trap_fdcb,			/* 0x48 */
		trap_fdcb,			/* 0x49 */
		trap_fdcb,			/* 0x4a */
		trap_fdcb,			/* 0x4b */
		trap_fdcb,			/* 0x4c */
		trap_fdcb,			/* 0x4d */
		op_tb1iyd,			/* 0x4e */
		trap_fdcb,			/* 0x4f */
		trap_fdcb,			/* 0x50 */
		trap_fdcb,			/* 0x51 */
		trap_fdcb,			/* 0x52 */
		trap_fdcb,			/* 0x53 */
		trap_fdcb,			/* 0x54 */
		trap_fdcb,			/* 0x55 */
		op_tb2iyd,			/* 0x56 */
		trap_fdcb,			/* 0x57 */
		trap_fdcb,			/* 0x58 */
		trap_fdcb,			/* 0x59 */
		trap_fdcb,			/* 0x5a */
		trap_fdcb,			/* 0x5b */
		trap_fdcb,			/* 0x5c */
		trap_fdcb,			/* 0x5d */
		op_tb3iyd,			/* 0x5e */
		trap_fdcb,			/* 0x5f */
		trap_fdcb,			/* 0x60 */
		trap_fdcb,			/* 0x61 */
		trap_fdcb,			/* 0x62 */
		trap_fdcb,			/* 0x63 */
		trap_fdcb,			/* 0x64 */
		trap_fdcb,			/* 0x65 */
		op_tb4iyd,			/* 0x66 */
		trap_fdcb,			/* 0x67 */
		trap_fdcb,			/* 0x68 */
		trap_fdcb,			/* 0x69 */
		trap_fdcb,			/* 0x6a */
		trap_fdcb,			/* 0x6b */
		trap_fdcb,			/* 0x6c */
		trap_fdcb,			/* 0x6d */
		op_tb5iyd,			/* 0x6e */
		trap_fdcb,			/* 0x6f */
		trap_fdcb,			/* 0x70 */
		trap_fdcb,			/* 0x71 */
		trap_fdcb,			/* 0x72 */
		trap_fdcb,			/* 0x73 */
		trap_fdcb,			/* 0x74 */
		trap_fdcb,			/* 0x75 */
		op_tb6iyd,			/* 0x76 */
		trap_fdcb,			/* 0x77 */
		trap_fdcb,			/* 0x78 */
		trap_fdcb,			/* 0x79 */
		trap_fdcb,			/* 0x7a */
		trap_fdcb,			/* 0x7b */
		trap_fdcb,			/* 0x7c */
		trap_fdcb,			/* 0x7d */
		op_tb7iyd,			/* 0x7e */
		trap_fdcb,			/* 0x7f */
		trap_fdcb,			/* 0x80 */
		trap_fdcb,			/* 0x81 */
		trap_fdcb,			/* 0x82 */
		trap_fdcb,			/* 0x83 */
		trap_fdcb,			/* 0x84 */
		trap_fdcb,			/* 0x85 */
		op_rb0iyd,			/* 0x86 */
		trap_fdcb,			/* 0x87 */
		trap_fdcb,			/* 0x88 */
		trap_fdcb,			/* 0x89 */
		trap_fdcb,			/* 0x8a */
		trap_fdcb,			/* 0x8b */
		trap_fdcb,			/* 0x8c */
		trap_fdcb,			/* 0x8d */
		op_rb1iyd,			/* 0x8e */
		trap_fdcb,			/* 0x8f */
		trap_fdcb,			/* 0x90 */
		trap_fdcb,			/* 0x91 */
		trap_fdcb,			/* 0x92 */
		trap_fdcb,			/* 0x93 */
		trap_fdcb,			/* 0x94 */
		trap_fdcb,			/* 0x95 */
		op_rb2iyd,			/* 0x96 */
		trap_fdcb,			/* 0x97 */
		trap_fdcb,			/* 0x98 */
		trap_fdcb,			/* 0x99 */
		trap_fdcb,			/* 0x9a */
		trap_fdcb,			/* 0x9b */
		trap_fdcb,			/* 0x9c */
		trap_fdcb,			/* 0x9d */
		op_rb3iyd,			/* 0x9e */
		trap_fdcb,			/* 0x9f */
		trap_fdcb,			/* 0xa0 */
		trap_fdcb,			/* 0xa1 */
		trap_fdcb,			/* 0xa2 */
		trap_fdcb,			/* 0xa3 */
		trap_fdcb,			/* 0xa4 */
		trap_fdcb,			/* 0xa5 */
		op_rb4iyd,			/* 0xa6 */
		trap_fdcb,			/* 0xa7 */
		trap_fdcb,			/* 0xa8 */
		trap_fdcb,			/* 0xa9 */
		trap_fdcb,			/* 0xaa */
		trap_fdcb,			/* 0xab */
		trap_fdcb,			/* 0xac */
		trap_fdcb,			/* 0xad */
		op_rb5iyd,			/* 0xae */
		trap_fdcb,			/* 0xaf */
		trap_fdcb,			/* 0xb0 */
		trap_fdcb,			/* 0xb1 */
		trap_fdcb,			/* 0xb2 */
		trap_fdcb,			/* 0xb3 */
		trap_fdcb,			/* 0xb4 */
		trap_fdcb,			/* 0xb5 */
		op_rb6iyd,			/* 0xb6 */
		trap_fdcb,			/* 0xb7 */
		trap_fdcb,			/* 0xb8 */
		trap_fdcb,			/* 0xb9 */
		trap_fdcb,			/* 0xba */
		trap_fdcb,			/* 0xbb */
		trap_fdcb,			/* 0xbc */
		trap_fdcb,			/* 0xbd */
		op_rb7iyd,			/* 0xbe */
		trap_fdcb,			/* 0xbf */
		trap_fdcb,			/* 0xc0 */
		trap_fdcb,			/* 0xc1 */
		trap_fdcb,			/* 0xc2 */
		trap_fdcb,			/* 0xc3 */
		trap_fdcb,			/* 0xc4 */
		trap_fdcb,			/* 0xc5 */
		op_sb0iyd,			/* 0xc6 */
		trap_fdcb,			/* 0xc7 */
		trap_fdcb,			/* 0xc8 */
		trap_fdcb,			/* 0xc9 */
		trap_fdcb,			/* 0xca */
		trap_fdcb,			/* 0xcb */
		trap_fdcb,			/* 0xcc */
		trap_fdcb,			/* 0xcd */
		op_sb1iyd,			/* 0xce */
		trap_fdcb,			/* 0xcf */
		trap_fdcb,			/* 0xd0 */
		trap_fdcb,			/* 0xd1 */
		trap_fdcb,			/* 0xd2 */
		trap_fdcb,			/* 0xd3 */
		trap_fdcb,			/* 0xd4 */
		trap_fdcb,			/* 0xd5 */
		op_sb2iyd,			/* 0xd6 */
		trap_fdcb,			/* 0xd7 */
		trap_fdcb,			/* 0xd8 */
		trap_fdcb,			/* 0xd9 */
		trap_fdcb,			/* 0xda */
		trap_fdcb,			/* 0xdb */
		trap_fdcb,			/* 0xdc */
		trap_fdcb,			/* 0xdd */
		op_sb3iyd,			/* 0xde */
		trap_fdcb,			/* 0xdf */
		trap_fdcb,			/* 0xe0 */
		trap_fdcb,			/* 0xe1 */
		trap_fdcb,			/* 0xe2 */
		trap_fdcb,			/* 0xe3 */
		trap_fdcb,			/* 0xe4 */
		trap_fdcb,			/* 0xe5 */
		op_sb4iyd,			/* 0xe6 */
		trap_fdcb,			/* 0xe7 */
		trap_fdcb,			/* 0xe8 */
		trap_fdcb,			/* 0xe9 */
		trap_fdcb,			/* 0xea */
		trap_fdcb,			/* 0xeb */
		trap_fdcb,			/* 0xec */
		trap_fdcb,			/* 0xed */
		op_sb5iyd,			/* 0xee */
		trap_fdcb,			/* 0xef */
		trap_fdcb,			/* 0xf0 */
		trap_fdcb,			/* 0xf1 */
		trap_fdcb,			/* 0xf2 */
		trap_fdcb,			/* 0xf3 */
		trap_fdcb,			/* 0xf4 */
		trap_fdcb,			/* 0xf5 */
		op_sb6iyd,			/* 0xf6 */
		trap_fdcb,			/* 0xf7 */
		trap_fdcb,			/* 0xf8 */
		trap_fdcb,			/* 0xf9 */
		trap_fdcb,			/* 0xfa */
		trap_fdcb,			/* 0xfb */
		trap_fdcb,			/* 0xfc */
		trap_fdcb,			/* 0xfd */
		op_sb7iyd,			/* 0xfe */
		trap_fdcb			/* 0xff */
	};

#undef UNDOC

	register int d;
	register int t;

	d = (SBYTE) memrdr(PC++);
	t = (*op_fdcb[memrdr(PC++)])(d); /* execute next opcode */

	return t;
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xfd 0xcb of a 4 byte opcode.
 */
static int trap_fdcb(int data)
{
	UNUSED(data);

	cpu_error = OPTRAP4;
	cpu_state = ST_STOPPED;
	return 0;
}

static int op_tb0iyd(int data)		/* BIT 0,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb1iyd(int data)		/* BIT 1,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb2iyd(int data)		/* BIT 2,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb3iyd(int data)		/* BIT 3,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				: (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb4iyd(int data)		/* BIT 4,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb5iyd(int data)		/* BIT 5,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return 20;
}

static int op_tb6iyd(int data)		/* BIT 6,(IY+d) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(IY + data) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				 : (F |= (Z_FLAG | P_FLAG));
	return 20;
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
	return 20;
}

static int op_rb0iyd(int data)		/* RES 0,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~1);
	return 23;
}

static int op_rb1iyd(int data)		/* RES 1,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~2);
	return 23;
}

static int op_rb2iyd(int data)		/* RES 2,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~4);
	return 23;
}

static int op_rb3iyd(int data)		/* RES 3,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~8);
	return 23;
}

static int op_rb4iyd(int data)		/* RES 4,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~16);
	return 23;
}

static int op_rb5iyd(int data)		/* RES 5,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~32);
	return 23;
}

static int op_rb6iyd(int data)		/* RES 6,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~64);
	return 23;
}

static int op_rb7iyd(int data)		/* RES 7,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) & ~128);
	return 23;
}

static int op_sb0iyd(int data)		/* SET 0,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 1);
	return 23;
}

static int op_sb1iyd(int data)		/* SET 1,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 2);
	return 23;
}

static int op_sb2iyd(int data)		/* SET 2,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 4);
	return 23;
}

static int op_sb3iyd(int data)		/* SET 3,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 8);
	return 23;
}

static int op_sb4iyd(int data)		/* SET 4,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 16);
	return 23;
}

static int op_sb5iyd(int data)		/* SET 5,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 32);
	return 23;
}

static int op_sb6iyd(int data)		/* SET 6,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 64);
	return 23;
}

static int op_sb7iyd(int data)		/* SET 7,(IY+d) */
{
	memwrt(IY + data, memrdr(IY + data) | 128);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
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
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

static int op_undoc_slliyd(int data)	/* SLL (IY+d) */
{
	register BYTE P;
	WORD addr;

	if (u_flag)
		return trap_fdcb(0);

	addr = IY + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P << 1) | 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return 23;
}

#endif /* UNDOC_INST */

#endif /* !EXCLUDE_Z80 && !ALT_Z80 */
