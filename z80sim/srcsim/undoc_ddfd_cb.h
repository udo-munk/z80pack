/*
 * Expands to functions implementing DDCB and FDCB-prefixed Z80 undocumented
 * opcodes
 *
 * Copyright (c) 2018 Christophe Staiesse
 */

#ifdef UNDOC_GEN_DD
#	define IXY IX
#	define TRAP if (u_flag) return trap_ddcb()
#	define ixyd ixd
#endif

#ifdef UNDOC_GEN_FD
#	define IXY IY
#	define TRAP if (u_flag) return trap_fdcb()
#	define ixyd iyd
#endif

#define CAT(a, b) XCAT(a, b)
#define XCAT(a, b) a ## b

/*
 *	Macros to assign and read registers from X Macros
 */

#define rega A
#define regb B
#define regc C
#define regd D
#define rege E
#define regh H
#define regl L

#define setreg(r, v) CAT(reg, r) = (v)

/*
 *	X Macros defining the functions
 */

#define XR8 \
	X(a) \
	X(b) \
	X(c) \
	X(d) \
	X(e) \
	X(h) \
	X(l)

#define XBITREG(X, reg) \
	X(0, reg) \
	X(1, reg) \
	X(2, reg) \
	X(3, reg) \
	X(4, reg) \
	X(5, reg) \
	X(6, reg) \
	X(7, reg)

/* BIT {0..7},(IXY+d),{A,B,C,D,E,H,L} */
/* The registers are not affected, so we simply call the documented instr. */

#define X(bit, reg) \
static int CAT(CAT(op_undoc_tb, bit), ixyd)(int data) { \
        TRAP; \
        return CAT(CAT(op_tb, bit), ixyd)(data); \
}
/* no need to define register variants */
XBITREG(X,)
#undef X

/* RES {0..7},(IXY+d),{A,B,C,D,E,H,L} */

#define Y(bit, reg) \
static int CAT(CAT(CAT(op_undoc_rb,bit),ixyd),reg)(int data) \
{ \
	BYTE P; \
\
	TRAP; \
	P = memrdr(IXY + data) & ~(1 << bit); \
	setreg(reg, P); \
	memwrt(IXY + data, P); \
	return(23); \
}
#define X(reg) XBITREG(Y, reg)
XR8
#undef X
#undef Y

/* SET {0..7},(IXY+d),{A,B,C,D,E,H,L} */

#define Y(bit, reg) \
static int CAT(CAT(CAT(op_undoc_sb,bit),ixyd),reg)(int data) \
{ \
	BYTE P; \
\
	TRAP; \
	P = memrdr(IXY + data) | (1 << bit); \
	setreg(reg, P); \
	memwrt(IXY + data, P); \
	return(23); \
}

#define X(reg) XBITREG(Y, reg)
XR8
#undef X

/* RLC (IXY+d),{A,B,C,D,E,H,L} */

#define X(reg) \
static int CAT(CAT(op_undoc_rlc,ixyd),reg)(int data) \
{ \
	register BYTE P; \
        WORD addr; \
	int i; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	i = P & 128; \
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P <<= 1; \
	if (i) P |= 1; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* RRC (IXY+d),{A,B,C,D,E,H,L} */
#define X(reg) \
static int CAT(CAT(op_undoc_rrc,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
	int i; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	i = P & 1; \
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P >>= 1; \
	if (i) P |= 128; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* RL (IXY+d),{A,B,C,D,E,H,L} */
#define X(reg) \
static int CAT(CAT(op_undoc_rl,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
	int old_c_flag; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	old_c_flag = F & C_FLAG; \
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P <<= 1; \
	if (old_c_flag) P |= 1; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* RR (IXY+d),{A,B,C,D,E,H,L} */

#define X(reg) \
static int CAT(CAT(op_undoc_rr,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
	int old_c_flag; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	old_c_flag = F & C_FLAG; \
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P >>= 1; \
	if (old_c_flag) P |= 128; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* SLA (IXY+d),{A,B,C,D,E,H,L} */

#define X(reg) \
static int CAT(CAT(op_undoc_sla,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P <<= 1; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* SRA (IXY+d),{A,B,C,D,E,H,L} */

#define X(reg) \
static int CAT(CAT(op_undoc_sra,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
	int i; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	i = P & 128; \
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P = (P >> 1) | i; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ?(F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X

/* SLL (IXY+d),{A,B,C,D,E,H,L} */
/* SLL (IXY+d)                 */

#define X(reg) \
static int CAT(CAT(op_undoc_sll,ixyd), reg)(int data) \
{ \
	BYTE P; \
	WORD addr; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P = (P << 1) | 1; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(8); \
}
XR8
#define reg P
X()
#undef reg
#undef X

/* SRL (IXY+d),{A,B,C,D,E,H,L} */ \

#define X(reg) \
static int CAT(CAT(op_undoc_srl,ixyd),reg)(int data) \
{ \
	register BYTE P; \
	WORD addr; \
\
	TRAP; \
	addr = IXY + data; \
	P = memrdr(addr); \
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	P >>= 1; \
	setreg(reg, P); \
	memwrt(addr, P); \
	F &= ~(H_FLAG | N_FLAG); \
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	return(23); \
}
XR8
#undef X
