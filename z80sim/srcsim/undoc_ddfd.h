/*
 * Expands to functions implementing DD and FD-prefixed Z80 undocumented
 * opcodes
 *
 * Copyright (c) 2018 Christophe Staiesse
 */

#ifdef UNDOC_GEN_DD
#	define IXY IX
#	define TRAP if (u_flag) return trap_dd()
#	define ixyh ixh
#	define ixyl ixl
#endif

#ifdef UNDOC_GEN_FD
#	define IXY IY
#	define TRAP if (u_flag) return trap_fd()
#	define ixyh iyh
#	define ixyl iyl
#endif

#define CAT(a, b) XCAT(a, b)
#define XCAT(a, b) a ## b

/*
 *	Macros to assign and read registers from X Macros
 */

#define set(r, v) CAT(set, r)(v)
#define get(r) CAT(get, r)

#define seta(v) (A = (v))
#define setb(v) (B = (v))
#define setc(v) (C = (v))
#define setd(v) (D = (v))
#define sete(v) (E = (v))
#define geta    A
#define getb    B
#define getc    C
#define getd    D
#define gete    E

#define setixyl(v) (IXY = (IXY & 0xff00) | (v))
#define setixyh(v) (IXY = (IXY & 0x00ff) | (v) << 8)
#define getixyl    (IXY & 0x00ff)
#define getixyh    (IXY >> 8)

#define setixl(v) setixyl(v)
#define setixh(v) setixyh(v)
#define setiyl(v) setixyl(v)
#define setiyh(v) setixyh(v)
#define getixl    getixyl
#define getixh    getixyh
#define getiyl    getixyl
#define getiyh    getixyh

/*
 *	X Macros defining the functions
 */

#define XABCDE \
	X(a) \
	X(b) \
	X(c) \
	X(d) \
	X(e)

#define XIXYHIXYL \
	X(ixyh) \
	X(ixyl)

/* LD {A,B,C,D,E},{IXYH,IXYL} */
/* LD {IXYH,IXYL},{A,B,C,D,E} */

#define Y(reg1, reg2) \
static int CAT(CAT(op_undoc_ld, reg1), reg2)(void) \
{ \
	TRAP; \
	set(reg1, get(reg2)); \
	return(8); \
}

#define X(reg) \
	Y(reg, ixyl) \
	Y(reg, ixyh) \
	Y(ixyl, reg) \
	Y(ixyh, reg)
XABCDE
Y(ixyl, ixyl)
Y(ixyl, ixyh)
Y(ixyh, ixyl)
Y(ixyh, ixyh)
#undef X
#undef Y

/* LD {IXYH,IXYL},n */

#define X(reg) \
static int CAT(CAT(op_undoc_ld, reg), n)(void) \
{ \
	TRAP; \
	set(reg, memrdr(PC++)); \
	return(11); \
}
XIXYHIXYL
#undef X

/* AND {IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_and, reg)(void) \
{ \
	TRAP; \
	A &= get(reg); \
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F |= H_FLAG; \
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	F &= ~(N_FLAG | C_FLAG); \
	return(8); \
}
XIXYHIXYL
#undef X

/* OR {IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_ora, reg)(void) \
{ \
	TRAP; \
	A |= get(reg); \
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	F &= ~(H_FLAG | N_FLAG | C_FLAG); \
	return(8); \
}
XIXYHIXYL
#undef X

/* XOR {IXYH,IXYL} */
#define X(reg) \
static int CAT(op_undoc_xor, reg)(void) \
{ \
	TRAP; \
	A ^= get(reg); \
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG); \
	F &= ~(H_FLAG | N_FLAG | C_FLAG); \
	return(8); \
}
XIXYHIXYL
#undef X

/* ADD A,{IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_ada, reg)(void) \
{ \
	register int i; \
	BYTE P; \
\
	TRAP; \
	P = get(reg); \
	((A & 0xf) + (P & 0xf) > 0xf) ?(F |= H_FLAG) : (F &= ~H_FLAG); \
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	A = i = (signed char) A + (signed char) P; \
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG); \
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F &= ~N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* ADC A,{IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_aca, reg)(void) \
{ \
	register int i, carry; \
	register BYTE P; \
\
	TRAP; \
	carry = (F & C_FLAG) ? 1 : 0; \
	P = get(reg); \
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	\
	    (F |= H_FLAG) : (F &= ~H_FLAG); \
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG); \
	A = i = (signed char) A + (signed char) P + carry; \
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG); \
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F &= ~N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* SUB A,{IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_sua, reg)(void) \
{ \
	register int i; \
	BYTE P; \
\
	TRAP; \
	P = get(reg); \
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG); \
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	A = i = (signed char) A - (signed char) P; \
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG); \
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F |= N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* SBC A,{IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_sca, reg)(void) \
{ \
	register int i, carry; \
	register BYTE P; \
\
	TRAP; \
	carry = (F & C_FLAG) ? 1 : 0; \
	P = get(reg); \
	((P & 0xf) + carry > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG); \
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	A = i = (signed char) A - (signed char) P - carry; \
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG); \
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F |= N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* CP {IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_cp, reg)(void) \
{ \
	register int i; \
	register BYTE P; \
\
	TRAP; \
	P = get(reg); \
	((P & 0xf) > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG); \
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG); \
	i = (signed char) A - (signed char) P; \
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG); \
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F |= N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* INC {IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_inc, reg)(void) \
{ \
	register BYTE P; \
\
	TRAP; \
	P = (get(reg)) + 1; \
	set(reg, P); \
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG); \
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	P ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F &= ~N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X

/* DEC {IXYH,IXYL} */

#define X(reg) \
static int CAT(op_undoc_dec, reg)(void) \
{ \
	BYTE P; \
\
	TRAP; \
	P = (get(reg) - 1); \
	set(reg, P); \
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG); \
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG); \
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG); \
	P ? (F &= ~Z_FLAG) : (F |= Z_FLAG); \
	F |= N_FLAG; \
	return(8); \
}
XIXYHIXYL
#undef X
