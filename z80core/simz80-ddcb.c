/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xdd 0xcb
 */

INSTRD(0x46, op_tb0ixd)			/* BIT 0,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x4e, op_tb1ixd)			/* BIT 1,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x56, op_tb2ixd)			/* BIT 2,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x5e, op_tb3ixd)			/* BIT 3,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x66, op_tb4ixd)			/* BIT 4,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x6e, op_tb5ixd)			/* BIT 5,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x76, op_tb6ixd)			/* BIT 6,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x7e, op_tb7ixd)			/* BIT 7,(IX+d) */
{
	WORD addr;

	addr = IX + data;
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

INSTRD(0x86, op_rb0ixd)			/* RES 0,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~1);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8e, op_rb1ixd)			/* RES 1,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~2);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x96, op_rb2ixd)			/* RES 2,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~4);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9e, op_rb3ixd)			/* RES 3,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~8);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa6, op_rb4ixd)			/* RES 4,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~16);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xae, op_rb5ixd)			/* RES 5,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~32);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb6, op_rb6ixd)			/* RES 6,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~64);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbe, op_rb7ixd)			/* RES 7,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) & ~128);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc6, op_sb0ixd)			/* SET 0,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 1);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xce, op_sb1ixd)			/* SET 1,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 2);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd6, op_sb2ixd)			/* SET 2,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 4);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xde, op_sb3ixd)			/* SET 3,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 8);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe6, op_sb4ixd)			/* SET 4,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 16);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xee, op_sb5ixd)			/* SET 5,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 32);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf6, op_sb6ixd)			/* SET 6,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 64);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfe, op_sb7ixd)			/* SET 7,(IX+d) */
{
	WORD addr;

	addr = IX + data;
	memwrt(addr, memrdr(addr) | 128);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x06, op_rlcixd)			/* RLC (IX+d) */
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

INSTRD(0x0e, op_rrcixd)			/* RRC (IX+d) */
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

INSTRD(0x16, op_rlixd)			/* RL (IX+d) */
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

INSTRD(0x1e, op_rrixd)			/* RR (IX+d) */
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

INSTRD(0x26, op_slaixd)			/* SLA (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + data;
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

INSTRD(0x2e, op_sraixd)			/* SRA (IX+d) */
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

INSTRD(0x3e, op_srlixd)			/* SRL (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + data;
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

INSTRD(0x36, op_undoc_sllixd)		/* SLL (IX+d) */
{
	register BYTE P;
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb0ixd(int data)	/* BIT 0,(IX+d) */
#else
case 0x40:				/* BIT 0,(IX+d) */
case 0x41:				/* BIT 0,(IX+d) */
case 0x42:				/* BIT 0,(IX+d) */
case 0x43:				/* BIT 0,(IX+d) */
case 0x44:				/* BIT 0,(IX+d) */
case 0x45:				/* BIT 0,(IX+d) */
case 0x47:				/* BIT 0,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb1ixd(int data)	/* BIT 1,(IX+d) */
#else
case 0x48:				/* BIT 1,(IX+d) */
case 0x49:				/* BIT 1,(IX+d) */
case 0x4a:				/* BIT 1,(IX+d) */
case 0x4b:				/* BIT 1,(IX+d) */
case 0x4c:				/* BIT 1,(IX+d) */
case 0x4d:				/* BIT 1,(IX+d) */
case 0x4f:				/* BIT 1,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb2ixd(int data)	/* BIT 2,(IX+d) */
#else
case 0x50:				/* BIT 2,(IX+d) */
case 0x51:				/* BIT 2,(IX+d) */
case 0x52:				/* BIT 2,(IX+d) */
case 0x53:				/* BIT 2,(IX+d) */
case 0x54:				/* BIT 2,(IX+d) */
case 0x55:				/* BIT 2,(IX+d) */
case 0x57:				/* BIT 2,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb3ixd(int data)	/* BIT 3,(IX+d) */
#else
case 0x58:				/* BIT 3,(IX+d) */
case 0x59:				/* BIT 3,(IX+d) */
case 0x5a:				/* BIT 3,(IX+d) */
case 0x5b:				/* BIT 3,(IX+d) */
case 0x5c:				/* BIT 3,(IX+d) */
case 0x5d:				/* BIT 3,(IX+d) */
case 0x5f:				/* BIT 3,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb4ixd(int data)	/* BIT 4,(IX+d) */
#else
case 0x60:				/* BIT 4,(IX+d) */
case 0x61:				/* BIT 4,(IX+d) */
case 0x62:				/* BIT 4,(IX+d) */
case 0x63:				/* BIT 4,(IX+d) */
case 0x64:				/* BIT 4,(IX+d) */
case 0x65:				/* BIT 4,(IX+d) */
case 0x67:				/* BIT 4,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb5ixd(int data)	/* BIT 5,(IX+d) */
#else
case 0x68:				/* BIT 5,(IX+d) */
case 0x69:				/* BIT 5,(IX+d) */
case 0x6a:				/* BIT 5,(IX+d) */
case 0x6b:				/* BIT 5,(IX+d) */
case 0x6c:				/* BIT 5,(IX+d) */
case 0x6d:				/* BIT 5,(IX+d) */
case 0x6f:				/* BIT 5,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb6ixd(int data)	/* BIT 6,(IX+d) */
#else
case 0x70:				/* BIT 6,(IX+d) */
case 0x71:				/* BIT 6,(IX+d) */
case 0x72:				/* BIT 6,(IX+d) */
case 0x73:				/* BIT 6,(IX+d) */
case 0x74:				/* BIT 6,(IX+d) */
case 0x75:				/* BIT 6,(IX+d) */
case 0x77:				/* BIT 6,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
static int op_undoc_tb7ixd(int data)	/* BIT 7,(IX+d) */
#else
case 0x78:				/* BIT 7,(IX+d) */
case 0x79:				/* BIT 7,(IX+d) */
case 0x7a:				/* BIT 7,(IX+d) */
case 0x7b:				/* BIT 7,(IX+d) */
case 0x7c:				/* BIT 7,(IX+d) */
case 0x7d:				/* BIT 7,(IX+d) */
case 0x7f:				/* BIT 7,(IX+d) */
#endif
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x87, op_undoc_rb0ixda)		/* RES 0,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~1;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8f, op_undoc_rb1ixda)		/* RES 1,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~2;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x97, op_undoc_rb2ixda)		/* RES 2,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~4;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9f, op_undoc_rb3ixda)		/* RES 3,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~8;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa7, op_undoc_rb4ixda)		/* RES 4,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~16;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xaf, op_undoc_rb5ixda)		/* RES 5,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~32;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb7, op_undoc_rb6ixda)		/* RES 6,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~64;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbf, op_undoc_rb7ixda)		/* RES 7,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) & ~128;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x80, op_undoc_rb0ixdb)		/* RES 0,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~1;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x88, op_undoc_rb1ixdb)		/* RES 1,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~2;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x90, op_undoc_rb2ixdb)		/* RES 2,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~4;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x98, op_undoc_rb3ixdb)		/* RES 3,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~8;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa0, op_undoc_rb4ixdb)		/* RES 4,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~16;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa8, op_undoc_rb5ixdb)		/* RES 5,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~32;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb0, op_undoc_rb6ixdb)		/* RES 6,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~64;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb8, op_undoc_rb7ixdb)		/* RES 7,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) & ~128;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x81, op_undoc_rb0ixdc)		/* RES 0,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~1;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x89, op_undoc_rb1ixdc)		/* RES 1,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~2;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x91, op_undoc_rb2ixdc)		/* RES 2,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~4;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x99, op_undoc_rb3ixdc)		/* RES 3,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~8;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa1, op_undoc_rb4ixdc)		/* RES 4,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~16;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa9, op_undoc_rb5ixdc)		/* RES 5,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~32;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb1, op_undoc_rb6ixdc)		/* RES 6,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~64;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb9, op_undoc_rb7ixdc)		/* RES 7,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) & ~128;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x82, op_undoc_rb0ixdd)		/* RES 0,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~1;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8a, op_undoc_rb1ixdd)		/* RES 1,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~2;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x92, op_undoc_rb2ixdd)		/* RES 2,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~4;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9a, op_undoc_rb3ixdd)		/* RES 3,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~8;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa2, op_undoc_rb4ixdd)		/* RES 4,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~16;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xaa, op_undoc_rb5ixdd)		/* RES 5,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~32;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb2, op_undoc_rb6ixdd)		/* RES 6,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~64;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xba, op_undoc_rb7ixdd)		/* RES 7,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) & ~128;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x83, op_undoc_rb0ixde)		/* RES 0,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~1;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8b, op_undoc_rb1ixde)		/* RES 1,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~2;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x93, op_undoc_rb2ixde)		/* RES 2,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~4;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9b, op_undoc_rb3ixde)		/* RES 3,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~8;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa3, op_undoc_rb4ixde)		/* RES 4,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~16;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xab, op_undoc_rb5ixde)		/* RES 5,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~32;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb3, op_undoc_rb6ixde)		/* RES 6,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~64;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbb, op_undoc_rb7ixde)		/* RES 7,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) & ~128;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x84, op_undoc_rb0ixdh)		/* RES 0,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~1;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8c, op_undoc_rb1ixdh)		/* RES 1,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~2;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x94, op_undoc_rb2ixdh)		/* RES 2,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~4;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9c, op_undoc_rb3ixdh)		/* RES 3,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~8;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa4, op_undoc_rb4ixdh)		/* RES 4,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~16;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xac, op_undoc_rb5ixdh)		/* RES 5,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~32;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb4, op_undoc_rb6ixdh)		/* RES 6,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~64;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbc, op_undoc_rb7ixdh)		/* RES 7,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) & ~128;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x85, op_undoc_rb0ixdl)		/* RES 0,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~1;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x8d, op_undoc_rb1ixdl)		/* RES 1,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~2;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x95, op_undoc_rb2ixdl)		/* RES 2,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~4;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x9d, op_undoc_rb3ixdl)		/* RES 3,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~8;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xa5, op_undoc_rb4ixdl)		/* RES 4,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~16;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xad, op_undoc_rb5ixdl)		/* RES 5,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~32;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xb5, op_undoc_rb6ixdl)		/* RES 6,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~64;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xbd, op_undoc_rb7ixdl)		/* RES 7,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) & ~128;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc7, op_undoc_sb0ixda)		/* SET 0,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 1;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcf, op_undoc_sb1ixda)		/* SET 1,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 2;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd7, op_undoc_sb2ixda)		/* SET 2,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 4;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdf, op_undoc_sb3ixda)		/* SET 3,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 8;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe7, op_undoc_sb4ixda)		/* SET 4,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 16;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xef, op_undoc_sb5ixda)		/* SET 5,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 32;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf7, op_undoc_sb6ixda)		/* SET 6,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 64;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xff, op_undoc_sb7ixda)		/* SET 7,(IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	A = memrdr(addr) | 128;
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc0, op_undoc_sb0ixdb)		/* SET 0,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 1;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc8, op_undoc_sb1ixdb)		/* SET 1,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 2;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd0, op_undoc_sb2ixdb)		/* SET 2,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 4;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd8, op_undoc_sb3ixdb)		/* SET 3,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 8;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe0, op_undoc_sb4ixdb)		/* SET 4,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 16;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe8, op_undoc_sb5ixdb)		/* SET 5,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 32;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf0, op_undoc_sb6ixdb)		/* SET 6,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 64;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf8, op_undoc_sb7ixdb)		/* SET 7,(IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	B = memrdr(addr) | 128;
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc1, op_undoc_sb0ixdc)		/* SET 0,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 1;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc9, op_undoc_sb1ixdc)		/* SET 1,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 2;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd1, op_undoc_sb2ixdc)		/* SET 2,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 4;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd9, op_undoc_sb3ixdc)		/* SET 3,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 8;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe1, op_undoc_sb4ixdc)		/* SET 4,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 16;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe9, op_undoc_sb5ixdc)		/* SET 5,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 32;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf1, op_undoc_sb6ixdc)		/* SET 6,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 64;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf9, op_undoc_sb7ixdc)		/* SET 7,(IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	C = memrdr(addr) | 128;
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc2, op_undoc_sb0ixdd)		/* SET 0,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 1;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xca, op_undoc_sb1ixdd)		/* SET 1,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 2;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd2, op_undoc_sb2ixdd)		/* SET 2,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 4;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xda, op_undoc_sb3ixdd)		/* SET 3,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 8;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe2, op_undoc_sb4ixdd)		/* SET 4,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 16;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xea, op_undoc_sb5ixdd)		/* SET 5,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 32;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf2, op_undoc_sb6ixdd)		/* SET 6,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 64;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfa, op_undoc_sb7ixdd)		/* SET 7,(IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	D = memrdr(addr) | 128;
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc3, op_undoc_sb0ixde)		/* SET 0,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 1;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcb, op_undoc_sb1ixde)		/* SET 1,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 2;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd3, op_undoc_sb2ixde)		/* SET 2,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 4;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdb, op_undoc_sb3ixde)		/* SET 3,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 8;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe3, op_undoc_sb4ixde)		/* SET 4,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 16;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xeb, op_undoc_sb5ixde)		/* SET 5,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 32;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf3, op_undoc_sb6ixde)		/* SET 6,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 64;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfb, op_undoc_sb7ixde)		/* SET 7,(IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	E = memrdr(addr) | 128;
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc4, op_undoc_sb0ixdh)		/* SET 0,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 1;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcc, op_undoc_sb1ixdh)		/* SET 1,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 2;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd4, op_undoc_sb2ixdh)		/* SET 2,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 4;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdc, op_undoc_sb3ixdh)		/* SET 3,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 8;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe4, op_undoc_sb4ixdh)		/* SET 4,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 16;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xec, op_undoc_sb5ixdh)		/* SET 5,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 32;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf4, op_undoc_sb6ixdh)		/* SET 6,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 64;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfc, op_undoc_sb7ixdh)		/* SET 7,(IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	H = memrdr(addr) | 128;
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xc5, op_undoc_sb0ixdl)		/* SET 0,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 1;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xcd, op_undoc_sb1ixdl)		/* SET 1,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 2;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xd5, op_undoc_sb2ixdl)		/* SET 2,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 4;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xdd, op_undoc_sb3ixdl)		/* SET 3,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 8;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xe5, op_undoc_sb4ixdl)		/* SET 4,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 16;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xed, op_undoc_sb5ixdl)		/* SET 5,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 32;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xf5, op_undoc_sb6ixdl)		/* SET 6,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 64;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0xfd, op_undoc_sb7ixdl)		/* SET 7,(IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
	L = memrdr(addr) | 128;
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(23);
}

INSTRD(0x07, op_undoc_rlcixda)		/* RLC (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x00, op_undoc_rlcixdb)		/* RLC (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x01, op_undoc_rlcixdc)		/* RLC (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x02, op_undoc_rlcixdd)		/* RLC (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x03, op_undoc_rlcixde)		/* RLC (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x04, op_undoc_rlcixdh)		/* RLC (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x05, op_undoc_rlcixdl)		/* RLC (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x0f, op_undoc_rrcixda)		/* RRC (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x08, op_undoc_rrcixdb)		/* RRC (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x09, op_undoc_rrcixdc)		/* RRC (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x0a, op_undoc_rrcixdd)		/* RRC (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x0b, op_undoc_rrcixde)		/* RRC (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x0c, op_undoc_rrcixdh)		/* RRC (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x0d, op_undoc_rrcixdl)		/* RRC (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x17, op_undoc_rlixda)		/* RL (IX+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x10, op_undoc_rlixdb)		/* RL (IX+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x11, op_undoc_rlixdc)		/* RL (IX+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x12, op_undoc_rlixdd)		/* RL (IX+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x13, op_undoc_rlixde)		/* RL (IX+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x14, op_undoc_rlixdh)		/* RL (IX+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x15, op_undoc_rlixdl)		/* RL (IX+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x1f, op_undoc_rrixda)		/* RR (IX+d),A */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x18, op_undoc_rrixdb)		/* RR (IX+d),B */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x19, op_undoc_rrixdc)		/* RR (IX+d),C */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x1a, op_undoc_rrixdd)		/* RR (IX+d),D */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x1b, op_undoc_rrixde)		/* RR (IX+d),E */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x1c, op_undoc_rrixdh)		/* RR (IX+d),H */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x1d, op_undoc_rrixdl)		/* RR (IX+d),L */
{
	WORD addr;
	int old_c_flag;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x27, op_undoc_slaixda)		/* SLA (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x20, op_undoc_slaixdb)		/* SLA (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x21, op_undoc_slaixdc)		/* SLA (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x22, op_undoc_slaixdd)		/* SLA (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x23, op_undoc_slaixde)		/* SLA (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x24, op_undoc_slaixdh)		/* SLA (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x25, op_undoc_slaixdl)		/* SLA (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x2f, op_undoc_sraixda)		/* SRA (IX+d),A */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x28, op_undoc_sraixdb)		/* SRA (IX+d),B */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x29, op_undoc_sraixdc)		/* SRA (IX+d),C */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x2a, op_undoc_sraixdd)		/* SRA (IX+d),D */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x2b, op_undoc_sraixde)		/* SRA (IX+d),E */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x2c, op_undoc_sraixdh)		/* SRA (IX+d),H */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x2d, op_undoc_sraixdl)		/* SRA (IX+d),L */
{
	WORD addr;
	int i;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x37, op_undoc_sllixda)		/* SLL (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x30, op_undoc_sllixdb)		/* SLL (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x31, op_undoc_sllixdc)		/* SLL (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x32, op_undoc_sllixdd)		/* SLL (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x33, op_undoc_sllixde)		/* SLL (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x34, op_undoc_sllixdh)		/* SLL (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x35, op_undoc_sllixdl)		/* SLL (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x3f, op_undoc_srlixda)		/* SRL (IX+d),A */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x38, op_undoc_srlixdb)		/* SRL (IX+d),B */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x39, op_undoc_srlixdc)		/* SRL (IX+d),C */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x3a, op_undoc_srlixdd)		/* SRL (IX+d),D */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x3b, op_undoc_srlixde)		/* SRL (IX+d),E */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x3c, op_undoc_srlixdh)		/* SRL (IX+d),H */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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

INSTRD(0x3d, op_undoc_srlixdl)		/* SRL (IX+d),L */
{
	WORD addr;

	if (u_flag) {
		STATES(trap_ddcb(0));
	}

	addr = IX + data;
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
