/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xfd 0xcb
 */

INSTRD(0x46, op_tb0iyd)			/* BIT 0,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x4e, op_tb1iyd)			/* BIT 1,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x56, op_tb2iyd)			/* BIT 2,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x5e, op_tb3iyd)			/* BIT 3,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x66, op_tb4iyd)			/* BIT 4,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x6e, op_tb5iyd)			/* BIT 5,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x76, op_tb6iyd)			/* BIT 6,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x7e, op_tb7iyd)			/* BIT 7,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (memrdr(addr) & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x86, op_rb0iyd)			/* RES 0,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~1);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8e, op_rb1iyd)			/* RES 1,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~2);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x96, op_rb2iyd)			/* RES 2,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~4);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9e, op_rb3iyd)			/* RES 3,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~8);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa6, op_rb4iyd)			/* RES 4,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~16);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xae, op_rb5iyd)			/* RES 5,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~32);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb6, op_rb6iyd)			/* RES 6,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~64);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbe, op_rb7iyd)			/* RES 7,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) & ~128);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc6, op_sb0iyd)			/* SET 0,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 1);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xce, op_sb1iyd)			/* SET 1,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 2);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd6, op_sb2iyd)			/* SET 2,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 4);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xde, op_sb3iyd)			/* SET 3,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 8);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe6, op_sb4iyd)			/* SET 4,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 16);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xee, op_sb5iyd)			/* SET 5,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 32);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf6, op_sb6iyd)			/* SET 6,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 64);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfe, op_sb7iyd)			/* SET 7,(IY+d) */
{
	WORD addr;

	addr = IY + data;
	memwrt(addr, memrdr(addr) | 128);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x06, op_rlciyd)			/* RLC (IY+d) */
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
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0e, op_rrciyd)			/* RRC (IY+d) */
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
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x16, op_rliyd)			/* RL (IY+d) */
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
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1e, op_rriyd)			/* RR (IY+d) */
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
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x26, op_slaiyd)			/* SLA (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2e, op_sraiyd)			/* SRA (IY+d) */
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
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3e, op_srliyd)			/* SRL (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + data;
	P = memrdr(addr);
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

INSTRD(0x36, op_undoc_slliyd)		/* SLL (IY+d) */
{
	register BYTE P;
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P << 1) | 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

#ifdef UNDOC_IALL

#ifndef INSTR_SWTCH
static int op_undoc_tb0iyd(int data)	/* BIT 0,(IY+d) */
#else
case 0x40:				/* BIT 0,(IY+d) */
case 0x41:				/* BIT 0,(IY+d) */
case 0x42:				/* BIT 0,(IY+d) */
case 0x43:				/* BIT 0,(IY+d) */
case 0x44:				/* BIT 0,(IY+d) */
case 0x45:				/* BIT 0,(IY+d) */
case 0x47:				/* BIT 0,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb1iyd(int data)	/* BIT 1,(IY+d) */
#else
case 0x48:				/* BIT 1,(IY+d) */
case 0x49:				/* BIT 1,(IY+d) */
case 0x4a:				/* BIT 1,(IY+d) */
case 0x4b:				/* BIT 1,(IY+d) */
case 0x4c:				/* BIT 1,(IY+d) */
case 0x4d:				/* BIT 1,(IY+d) */
case 0x4f:				/* BIT 1,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb2iyd(int data)	/* BIT 2,(IY+d) */
#else
case 0x50:				/* BIT 2,(IY+d) */
case 0x51:				/* BIT 2,(IY+d) */
case 0x52:				/* BIT 2,(IY+d) */
case 0x53:				/* BIT 2,(IY+d) */
case 0x54:				/* BIT 2,(IY+d) */
case 0x55:				/* BIT 2,(IY+d) */
case 0x57:				/* BIT 2,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb3iyd(int data)	/* BIT 3,(IY+d) */
#else
case 0x58:				/* BIT 3,(IY+d) */
case 0x59:				/* BIT 3,(IY+d) */
case 0x5a:				/* BIT 3,(IY+d) */
case 0x5b:				/* BIT 3,(IY+d) */
case 0x5c:				/* BIT 3,(IY+d) */
case 0x5d:				/* BIT 3,(IY+d) */
case 0x5f:				/* BIT 3,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
			   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb4iyd(int data)	/* BIT 4,(IY+d) */
#else
case 0x60:				/* BIT 4,(IY+d) */
case 0x61:				/* BIT 4,(IY+d) */
case 0x62:				/* BIT 4,(IY+d) */
case 0x63:				/* BIT 4,(IY+d) */
case 0x64:				/* BIT 4,(IY+d) */
case 0x65:				/* BIT 4,(IY+d) */
case 0x67:				/* BIT 4,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb5iyd(int data)	/* BIT 5,(IY+d) */
#else
case 0x68:				/* BIT 5,(IY+d) */
case 0x69:				/* BIT 5,(IY+d) */
case 0x6a:				/* BIT 5,(IY+d) */
case 0x6b:				/* BIT 5,(IY+d) */
case 0x6c:				/* BIT 5,(IY+d) */
case 0x6d:				/* BIT 5,(IY+d) */
case 0x6f:				/* BIT 5,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb6iyd(int data)	/* BIT 6,(IY+d) */
#else
case 0x70:				/* BIT 6,(IY+d) */
case 0x71:				/* BIT 6,(IY+d) */
case 0x72:				/* BIT 6,(IY+d) */
case 0x73:				/* BIT 6,(IY+d) */
case 0x74:				/* BIT 6,(IY+d) */
case 0x75:				/* BIT 6,(IY+d) */
case 0x77:				/* BIT 6,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr(addr) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
			    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

#ifndef INSTR_SWTCH
static int op_undoc_tb7iyd(int data)	/* BIT 7,(IY+d) */
#else
case 0x78:				/* BIT 7,(IY+d) */
case 0x79:				/* BIT 7,(IY+d) */
case 0x7a:				/* BIT 7,(IY+d) */
case 0x7b:				/* BIT 7,(IY+d) */
case 0x7c:				/* BIT 7,(IY+d) */
case 0x7d:				/* BIT 7,(IY+d) */
case 0x7f:				/* BIT 7,(IY+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (memrdr(addr) & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
#ifdef UNDOC_FLAGS
	(addr & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(addr & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(20);
}

INSTRD(0x87, op_undoc_rb0iyda)		/* RES 0,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~1;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8f, op_undoc_rb1iyda)		/* RES 1,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~2;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x97, op_undoc_rb2iyda)		/* RES 2,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~4;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9f, op_undoc_rb3iyda)		/* RES 3,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~8;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa7, op_undoc_rb4iyda)		/* RES 4,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~16;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xaf, op_undoc_rb5iyda)		/* RES 5,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~32;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb7, op_undoc_rb6iyda)		/* RES 6,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~64;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbf, op_undoc_rb7iyda)		/* RES 7,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) & ~128;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x80, op_undoc_rb0iydb)		/* RES 0,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~1;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x88, op_undoc_rb1iydb)		/* RES 1,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~2;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x90, op_undoc_rb2iydb)		/* RES 2,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~4;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x98, op_undoc_rb3iydb)		/* RES 3,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~8;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa0, op_undoc_rb4iydb)		/* RES 4,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~16;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa8, op_undoc_rb5iydb)		/* RES 5,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~32;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb0, op_undoc_rb6iydb)		/* RES 6,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~64;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb8, op_undoc_rb7iydb)		/* RES 7,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) & ~128;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x81, op_undoc_rb0iydc)		/* RES 0,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~1;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x89, op_undoc_rb1iydc)		/* RES 1,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~2;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x91, op_undoc_rb2iydc)		/* RES 2,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~4;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x99, op_undoc_rb3iydc)		/* RES 3,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~8;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa1, op_undoc_rb4iydc)		/* RES 4,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~16;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa9, op_undoc_rb5iydc)		/* RES 5,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~32;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb1, op_undoc_rb6iydc)		/* RES 6,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~64;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb9, op_undoc_rb7iydc)		/* RES 7,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) & ~128;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x82, op_undoc_rb0iydd)		/* RES 0,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~1;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8a, op_undoc_rb1iydd)		/* RES 1,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~2;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x92, op_undoc_rb2iydd)		/* RES 2,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~4;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9a, op_undoc_rb3iydd)		/* RES 3,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~8;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa2, op_undoc_rb4iydd)		/* RES 4,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~16;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xaa, op_undoc_rb5iydd)		/* RES 5,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~32;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb2, op_undoc_rb6iydd)		/* RES 6,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~64;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xba, op_undoc_rb7iydd)		/* RES 7,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) & ~128;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x83, op_undoc_rb0iyde)		/* RES 0,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~1;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8b, op_undoc_rb1iyde)		/* RES 1,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~2;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x93, op_undoc_rb2iyde)		/* RES 2,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~4;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9b, op_undoc_rb3iyde)		/* RES 3,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~8;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa3, op_undoc_rb4iyde)		/* RES 4,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~16;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xab, op_undoc_rb5iyde)		/* RES 5,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~32;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb3, op_undoc_rb6iyde)		/* RES 6,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~64;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbb, op_undoc_rb7iyde)		/* RES 7,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) & ~128;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x84, op_undoc_rb0iydh)		/* RES 0,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~1;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8c, op_undoc_rb1iydh)		/* RES 1,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~2;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x94, op_undoc_rb2iydh)		/* RES 2,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~4;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9c, op_undoc_rb3iydh)		/* RES 3,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~8;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa4, op_undoc_rb4iydh)		/* RES 4,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~16;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xac, op_undoc_rb5iydh)		/* RES 5,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~32;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb4, op_undoc_rb6iydh)		/* RES 6,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~64;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbc, op_undoc_rb7iydh)		/* RES 7,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) & ~128;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x85, op_undoc_rb0iydl)		/* RES 0,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~1;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8d, op_undoc_rb1iydl)		/* RES 1,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~2;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x95, op_undoc_rb2iydl)		/* RES 2,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~4;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9d, op_undoc_rb3iydl)		/* RES 3,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~8;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa5, op_undoc_rb4iydl)		/* RES 4,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~16;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xad, op_undoc_rb5iydl)		/* RES 5,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~32;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb5, op_undoc_rb6iydl)		/* RES 6,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~64;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbd, op_undoc_rb7iydl)		/* RES 7,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) & ~128;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc7, op_undoc_sb0iyda)		/* SET 0,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 1;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcf, op_undoc_sb1iyda)		/* SET 1,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 2;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd7, op_undoc_sb2iyda)		/* SET 2,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 4;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdf, op_undoc_sb3iyda)		/* SET 3,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 8;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe7, op_undoc_sb4iyda)		/* SET 4,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 16;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xef, op_undoc_sb5iyda)		/* SET 5,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 32;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf7, op_undoc_sb6iyda)		/* SET 6,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 64;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xff, op_undoc_sb7iyda)		/* SET 7,(IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr) | 128;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc0, op_undoc_sb0iydb)		/* SET 0,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 1;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc8, op_undoc_sb1iydb)		/* SET 1,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 2;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd0, op_undoc_sb2iydb)		/* SET 2,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 4;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd8, op_undoc_sb3iydb)		/* SET 3,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 8;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe0, op_undoc_sb4iydb)		/* SET 4,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 16;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe8, op_undoc_sb5iydb)		/* SET 5,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 32;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf0, op_undoc_sb6iydb)		/* SET 6,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 64;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf8, op_undoc_sb7iydb)		/* SET 7,(IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr) | 128;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc1, op_undoc_sb0iydc)		/* SET 0,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 1;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc9, op_undoc_sb1iydc)		/* SET 1,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 2;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd1, op_undoc_sb2iydc)		/* SET 2,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 4;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd9, op_undoc_sb3iydc)		/* SET 3,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 8;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe1, op_undoc_sb4iydc)		/* SET 4,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 16;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe9, op_undoc_sb5iydc)		/* SET 5,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 32;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf1, op_undoc_sb6iydc)		/* SET 6,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 64;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf9, op_undoc_sb7iydc)		/* SET 7,(IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr) | 128;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc2, op_undoc_sb0iydd)		/* SET 0,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 1;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xca, op_undoc_sb1iydd)		/* SET 1,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 2;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd2, op_undoc_sb2iydd)		/* SET 2,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 4;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xda, op_undoc_sb3iydd)		/* SET 3,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 8;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe2, op_undoc_sb4iydd)		/* SET 4,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 16;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xea, op_undoc_sb5iydd)		/* SET 5,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 32;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf2, op_undoc_sb6iydd)		/* SET 6,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 64;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfa, op_undoc_sb7iydd)		/* SET 7,(IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr) | 128;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc3, op_undoc_sb0iyde)		/* SET 0,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 1;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcb, op_undoc_sb1iyde)		/* SET 1,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 2;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd3, op_undoc_sb2iyde)		/* SET 2,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 4;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdb, op_undoc_sb3iyde)		/* SET 3,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 8;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe3, op_undoc_sb4iyde)		/* SET 4,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 16;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xeb, op_undoc_sb5iyde)		/* SET 5,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 32;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf3, op_undoc_sb6iyde)		/* SET 6,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 64;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfb, op_undoc_sb7iyde)		/* SET 7,(IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr) | 128;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc4, op_undoc_sb0iydh)		/* SET 0,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 1;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcc, op_undoc_sb1iydh)		/* SET 1,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 2;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd4, op_undoc_sb2iydh)		/* SET 2,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 4;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdc, op_undoc_sb3iydh)		/* SET 3,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 8;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe4, op_undoc_sb4iydh)		/* SET 4,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 16;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xec, op_undoc_sb5iydh)		/* SET 5,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 32;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf4, op_undoc_sb6iydh)		/* SET 6,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 64;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfc, op_undoc_sb7iydh)		/* SET 7,(IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr) | 128;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc5, op_undoc_sb0iydl)		/* SET 0,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 1;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcd, op_undoc_sb1iydl)		/* SET 1,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 2;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd5, op_undoc_sb2iydl)		/* SET 2,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 4;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdd, op_undoc_sb3iydl)		/* SET 3,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 8;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe5, op_undoc_sb4iydl)		/* SET 4,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 16;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xed, op_undoc_sb5iydl)		/* SET 5,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 32;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf5, op_undoc_sb6iydl)		/* SET 6,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 64;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfd, op_undoc_sb7iydl)		/* SET 7,(IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr) | 128;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x07, op_undoc_rlciyda)		/* RLC (IY+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	i = A & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (i) A |= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x00, op_undoc_rlciydb)		/* RLC (IY+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	i = B & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (i) B |= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x01, op_undoc_rlciydc)		/* RLC (IY+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	i = C & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (i) C |= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x02, op_undoc_rlciydd)		/* RLC (IY+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	i = D & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (i) D |= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x03, op_undoc_rlciyde)		/* RLC (IY+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	i = E & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (i) E |= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x04, op_undoc_rlciydh)		/* RLC (IY+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	i = H & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (i) H |= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x05, op_undoc_rlciydl)		/* RLC (IY+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	i = L & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (i) L |= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0f, op_undoc_rrciyda)		/* RRC (IY+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (i) A |= 128;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x08, op_undoc_rrciydb)		/* RRC (IY+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	i = B & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (i) B |= 128;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x09, op_undoc_rrciydc)		/* RRC (IY+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	i = C & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (i) C |= 128;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0a, op_undoc_rrciydd)		/* RRC (IY+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	i = D & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (i) D |= 128;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0b, op_undoc_rrciyde)		/* RRC (IY+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	i = E & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (i) E |= 128;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0c, op_undoc_rrciydh)		/* RRC (IY+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	i = H & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (i) H |= 128;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x0d, op_undoc_rrciydl)		/* RRC (IY+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	i = L & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (i) L |= 128;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x17, op_undoc_rliyda)		/* RL (IY+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x10, op_undoc_rliydb)		/* RL (IY+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (old_c_flag) B |= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x11, op_undoc_rliydc)		/* RL (IY+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (old_c_flag) C |= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x12, op_undoc_rliydd)		/* RL (IY+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (old_c_flag) D |= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x13, op_undoc_rliyde)		/* RL (IY+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (old_c_flag) E |= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x14, op_undoc_rliydh)		/* RL (IY+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (old_c_flag) H |= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x15, op_undoc_rliydl)		/* RL (IY+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (old_c_flag) L |= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1f, op_undoc_rriyda)		/* RR (IY+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x18, op_undoc_rriydb)		/* RR (IY+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (old_c_flag) B |= 128;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x19, op_undoc_rriydc)		/* RR (IY+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (old_c_flag) C |= 128;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1a, op_undoc_rriydd)		/* RR (IY+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (old_c_flag) D |= 128;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1b, op_undoc_rriyde)		/* RR (IY+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (old_c_flag) E |= 128;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1c, op_undoc_rriydh)		/* RR (IY+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (old_c_flag) H |= 128;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x1d, op_undoc_rriydl)		/* RR (IY+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (old_c_flag) L |= 128;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x27, op_undoc_slaiyda)		/* SLA (IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x20, op_undoc_slaiydb)		/* SLA (IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x21, op_undoc_slaiydc)		/* SLA (IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x22, op_undoc_slaiydd)		/* SLA (IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x23, op_undoc_slaiyde)		/* SLA (IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x24, op_undoc_slaiydh)		/* SLA (IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x25, op_undoc_slaiydl)		/* SLA (IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2f, op_undoc_sraiyda)		/* SRA (IY+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	i = A & 128;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A >> 1) | i;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x28, op_undoc_sraiydb)		/* SRA (IY+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	i = B & 128;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = (B >> 1) | i;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x29, op_undoc_sraiydc)		/* SRA (IY+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	i = C & 128;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = (C >> 1) | i;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2a, op_undoc_sraiydd)		/* SRA (IY+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	i = D & 128;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = (D >> 1) | i;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2b, op_undoc_sraiyde)		/* SRA (IY+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	i = E & 128;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = (E >> 1) | i;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2c, op_undoc_sraiydh)		/* SRA (IY+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	i = H & 128;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = (H >> 1) | i;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x2d, op_undoc_sraiydl)		/* SRA (IY+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	i = L & 128;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = (L >> 1) | i;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x37, op_undoc_slliyda)		/* SLL (IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A << 1) | 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x30, op_undoc_slliydb)		/* SLL (IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = (B << 1) | 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x31, op_undoc_slliydc)		/* SLL (IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = (C << 1) | 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x32, op_undoc_slliydd)		/* SLL (IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = (D << 1) | 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x33, op_undoc_slliyde)		/* SLL (IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = (E << 1) | 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x34, op_undoc_slliydh)		/* SLL (IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = (H << 1) | 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x35, op_undoc_slliydl)		/* SLL (IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = (L << 1) | 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3f, op_undoc_srliyda)		/* SRL (IY+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	A = memrdr(addr);
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	memwrt(addr, A);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x38, op_undoc_srliydb)		/* SRL (IY+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	B = memrdr(addr);
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	memwrt(addr, B);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x39, op_undoc_srliydc)		/* SRL (IY+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	C = memrdr(addr);
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	memwrt(addr, C);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3a, op_undoc_srliydd)		/* SRL (IY+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	D = memrdr(addr);
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	memwrt(addr, D);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3b, op_undoc_srliyde)		/* SRL (IY+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	E = memrdr(addr);
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	memwrt(addr, E);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3c, op_undoc_srliydh)		/* SRL (IY+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	H = memrdr(addr);
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	memwrt(addr, H);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTRD(0x3d, op_undoc_srliydl)		/* SRL (IY+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_fdcb(0));
	}

	addr = IY + data;
	L = memrdr(addr);
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	memwrt(addr, L);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

#endif /* UNDOC_IALL */

#endif /* UNDOC_INST */
