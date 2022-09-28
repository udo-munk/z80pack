/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	22-OCT-2006 changed to ANSI C for modern POSIX OS's
 *	03-FEB-2007 more ANSI C conformance and reduced compiler warnings
 *	18-MAR-2007 use default output file extension dependent on format
 *	04-OCT-2008 fixed comment bug, ';' string argument now working
 *	22-FEB-2014 fixed is...() compiler warnings
 *	13-JAN-2016 fixed buffer overflow, new expression parser from Didier
 *	02-OCT-2017 bug fixes in expression parser from Didier
 *	28-OCT-2017 added variable symbol lenght and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 */

/*
 *	processing of all real Z80/8080 opcodes
 */

#include <stdio.h>
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

char *get_first_second(void);
int ldreg(int, int, char *), ldixhl(int, int, char *);
int ldiyhl(int, int, char *), ldbcde(int, char *), ldhl(char *);
int ldixy(int, char *), ldsp(char *), ldihl(char *);
int ldiixy(int, char *), ldinn(char *);
int addhl(char *), addix(char *), addiy(char *), sbadchl(int, char *);
int aluop(int, char *);
void cbgrp_iixy(int, int, int, char *);

extern int eval(char *);
extern int calc_val(char *);
extern int chk_byte(int);
extern int chk_sbyte(int);
extern void asmerr(int);
extern int get_reg(char *);
extern void put_label(void);

#define UNUSED(x)	(void)(x)

/*
 *	process 1byte opcodes without arguments
 */
int op_1b(int b1, int dummy)
{
	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	ops[0] = b1;
	return(1);
}

/*
 *	process 2byte opcodes without arguments
 */
int op_2b(int b1, int b2)
{
	if (pass == 1)
		if (*label)
			put_label();
	ops[0] = b1;
	ops[1] = b2;
	return(2);
}

/*
 *	IM
 */
int op_im(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		ops[0] = 0xed;
		switch(eval(operand)) {
		case 0:			/* IM 0 */
			ops[1] = 0x46;
			break;
		case 1:			/* IM 1 */
			ops[1] = 0x56;
			break;
		case 2:			/* IM 2 */
			ops[1] = 0x5e;
			break;
		default:
			ops[1] = 0;
			asmerr(E_ILLOPE);
			break;
		}
	}
	return(2);
}

/*
 *	PUSH and POP
 */
int op_pupo(int base_op, int dummy)
{
	register int len;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (get_reg(operand)) {
	case REGAF:			/* PUSH/POP AF */
		len = 1;
		ops[0] = base_op + 0x30;
		break;
	case REGBC:			/* PUSH/POP BC */
		len = 1;
		ops[0] = base_op;
		break;
	case REGDE:			/* PUSH/POP DE */
		len = 1;
		ops[0] = base_op + 0x10;
		break;
	case REGHL:			/* PUSH/POP HL */
		len = 1;
		ops[0] = base_op + 0x20;
		break;
	case REGIX:			/* PUSH/POP IX */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x20;
		break;
	case REGIY:			/* PUSH/POP IY */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x20;
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	EX
 */
int op_ex(int dummy1, int dummy2)
{
	register int len;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	if (strncmp(operand, "DE,HL", 5) == 0) {
		len = 1;
		ops[0] = 0xeb;
	} else if (strncmp(operand, "AF,AF'", 7) == 0) {
		len = 1;
		ops[0] = 0x08;
	} else if (strncmp(operand, "(SP),HL", 7) == 0) {
		len = 1;
		ops[0] = 0xe3;
	} else if (strncmp(operand, "(SP),IX", 7) == 0) {
		len = 2;
		ops[0] = 0xdd;
		ops[1] = 0xe3;
	} else if (strncmp(operand, "(SP),IY", 7) == 0) {
		len = 2;
		ops[0] = 0xfd;
		ops[1] = 0xe3;
	} else {
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	RST
 */
int op_rst(int dummy1, int dummy2)
{
	register int op;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		op = eval(operand);
		if (op < 0 || (op / 8 > 7) || (op % 8 != 0)) {
			ops[0] = 0;
			asmerr(E_VALOUT);
		} else
			ops[0] = 0xc7 + op;
	}
	return(1);
}

/*
 *	RET
 */
int op_ret(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		switch (get_reg(operand)) {
		case NOOPERA:		/* RET */
			ops[0] = 0xc9;
			break;
		case REGC:		/* RET C */
			ops[0] = 0xd8;
			break;
		case FLGNC:		/* RET NC */
			ops[0] = 0xd0;
			break;
		case FLGZ:		/* RET Z */
			ops[0] = 0xc8;
			break;
		case FLGNZ:		/* RET NZ */
			ops[0] = 0xc0;
			break;
		case FLGPE:		/* RET PE */
			ops[0] = 0xe8;
			break;
		case FLGPO:		/* RET PO */
			ops[0] = 0xe0;
			break;
		case FLGM:		/* RET M */
			ops[0] = 0xf8;
			break;
		case FLGP:		/* RET P */
			ops[0] = 0xf0;
			break;
		default:		/* invalid operand */
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
	}
	return(1);
}

/*
 *	JP and CALL
 */
int op_jpcall(int base_op, int base_opd)
{
	register char *sec;
	register int i, len, op;

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (op = get_reg(tmp)) {
	case REGC:			/* JP/CALL C,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x18;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGNC:			/* JP/CALL NC,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x10;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGZ:			/* JP/CALL Z,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x08;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGNZ:			/* JP/CALL NZ,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGPE:			/* JP/CALL PE,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x28;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGPO:			/* JP/CALL PO,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x20;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGM:			/* JP/CALL M,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x38;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case FLGP:			/* JP/CALL P,nn */
		len = 3;
		if (pass == 2) {
			i = eval(sec);
			ops[0] = base_op + 0x30;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case REGIHL:			/* JP/CALL (HL) */
	case REGIIX:			/* JP/CALL (IX) */
	case REGIIY:			/* JP/CALL (IY) */
		if (base_op == 0xc2) {
			/* only for JP */
			switch (op) {
			case REGIHL:	/* JP (HL) */
				len = 1;
				ops[0] = 0xe9;
				break;
			case REGIIX:	/* JP (IX) */
				len = 2;
				ops[0] = 0xdd;
				ops[1] = 0xe9;
				break;
			case REGIIY:	/* JP (IY) */
				len = 2;
				ops[0] = 0xfd;
				ops[1] = 0xe9;
				break;
			}
		} else {
			/* not for CALL */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOREG:			/* JP/CALL nn */
		len = 3;
		if (pass == 2) {
			i = eval(operand);
			ops[0] = base_opd;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	JR
 */
int op_jr(int dummy1, int dummy2)
{
	register char *sec;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		sec = get_first_second();
		switch (get_reg(tmp)) {
		case REGC:		/* JR C,n */
			ops[0] = 0x38;
			ops[1] = chk_sbyte(eval(sec) - pc - 2);
			break;
		case FLGNC:		/* JR NC,n */
			ops[0] = 0x30;
			ops[1] = chk_sbyte(eval(sec) - pc - 2);
			break;
		case FLGZ:		/* JR Z,n */
			ops[0] = 0x28;
			ops[1] = chk_sbyte(eval(sec) - pc - 2);
			break;
		case FLGNZ:		/* JR NZ,n */
			ops[0] = 0x20;
			ops[1] = chk_sbyte(eval(sec) - pc - 2);
			break;
		case NOREG:		/* JR n */
			ops[0] = 0x18;
			ops[1] = chk_sbyte(eval(operand) - pc - 2);
			break;
		case NOOPERA:		/* missing operand */
			ops[0] = 0;
			ops[1] = 0;
			asmerr(E_MISOPE);
			break;
		default:		/* invalid operand */
			ops[0] = 0;
			ops[1] = 0;
			asmerr(E_ILLOPE);
		}
	}
	return(2);
}

/*
 *	DJNZ
 */
int op_djnz(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		ops[0] = 0x10;
		ops[1] = chk_sbyte(eval(operand) - pc - 2);
	}
	return(2);
}

/*
 *	LD
 */
int op_ld(int dummy1, int dummy2)
{
	register int len, op;
	register char *sec;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (op = get_reg(tmp)) {
	case REGA:			/* LD A,? */
	case REGB:			/* LD B,? */
	case REGC:			/* LD C,? */
	case REGD:			/* LD D,? */
	case REGE:			/* LD E,? */
	case REGH:			/* LD H,? */
	case REGL:			/* LD L,? */
		len = ldreg(0x40 + (op << 3), 0x06 + (op << 3), sec);
		break;
	case REGIXH:			/* LD IXH,? (undocumented) */
		len = ldixhl(0x60, 0x26, sec);
		break;
	case REGIXL:			/* LD IXL,? (undocumented) */
		len = ldixhl(0x68, 0x2e, sec);
		break;
	case REGIYH:			/* LD IYH,? (undocumented) */
		len = ldiyhl(0x60, 0x26, sec);
		break;
	case REGIYL:			/* LD IYL,? (undocumented) */
		len = ldiyhl(0x68, 0x2e, sec);
		break;
	case REGI:			/* LD I,A */
		if (get_reg(sec) == REGA) {
			len = 2;
			ops[0] = 0xed;
			ops[1] = 0x47;
			break;
		}
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
		break;
	case REGR:			/* LD R,A */
		if (get_reg(sec) == REGA) {
			len = 2;
			ops[0] = 0xed;
			ops[1] = 0x4f;
			break;
		}
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
		break;
	case REGBC:			/* LD BC,? */
		len = ldbcde(0x01, sec);
		break;
	case REGDE:			/* LD DE,? */
		len = ldbcde(0x11, sec);
		break;
	case REGHL:			/* LD HL,? */
		len = ldhl(sec);
		break;
	case REGIX:			/* LD IX,? */
		len = ldixy(0xdd, sec);
		break;
	case REGIY:			/* LD IY,? */
		len = ldixy(0xfd, sec);
		break;
	case REGSP:			/* LD SP,? */
		len = ldsp(sec);
		break;
	case REGIHL:			/* LD (HL),? */
		len = ldihl(sec);
		break;
	case REGIBC:			/* LD (BC),A */
		if (get_reg(sec) == REGA) {
			len = 1;
			ops[0] = 0x02;
			break;
		}
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
		break;
	case REGIDE:			/* LD (DE),A */
		if (get_reg(sec) == REGA) {
			len = 1;
			ops[0] = 0x12;
			break;
		}
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:
		if (strncmp(operand, "(IX+", 4) == 0)
			len = ldiixy(0xdd, sec);	/* LD (IX+d),? */
		else if (strncmp(operand, "(IY+", 4) == 0)
			len = ldiixy(0xfd, sec);	/* LD (IY+d),? */
		else if (*operand == '(')
			len = ldinn(sec);		/* LD (nn),? */
		else {			/* invalid operand */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
	}
	return(len);
}

/*
 *	LD [A,B,C,D,E,H,L],?
 */
int ldreg(int base_op, int base_opn, char *sec)
{
	register int op;
	register int i, len;

	switch (op = get_reg(sec)) {
	case REGA:			/* LD reg,A */
	case REGB:			/* LD reg,B */
	case REGC:			/* LD reg,C */
	case REGD:			/* LD reg,D */
	case REGE:			/* LD reg,E */
	case REGH:			/* LD reg,H */
	case REGL:			/* LD reg,L */
	case REGIHL:			/* LD reg,(HL) */
		len = 1;
		ops[0] = base_op + op;
		break;
	case REGIXH:			/* LD reg,IXH (undocumented) */
	case REGIXL:			/* LD reg,IXL (undocumented) */
	case REGIYH:			/* LD reg,IYH (undocumented) */
	case REGIYL:			/* LD reg,IYL (undocumented) */
		if ((base_op & 0xf0) != 0x60) {
			/* only for A,B,C,D,E */
			switch (op) {
			case REGIXH:	/* LD [ABCDE],IXH (undocumented) */
				len = 2;
				ops[0] = 0xdd;
				ops[1] = base_op + 0x04;
				break;
			case REGIXL:	/* LD [ABCDE],IXL (undocumented) */
				len = 2;
				ops[0] = 0xdd;
				ops[1] = base_op + 0x05;
				break;
			case REGIYH:	/* LD [ABCDE],IYH (undocumented) */
				len = 2;
				ops[0] = 0xfd;
				ops[1] = base_op + 0x04;
				break;
			case REGIYL:	/* LD [ABCDE],IYL (undocumented) */
				len = 2;
				ops[0] = 0xfd;
				ops[1] = base_op + 0x05;
				break;
			}
		}
		else {
			/* not for H, L */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case REGI:			/* LD reg,I */
	case REGR:			/* LD reg,R */
	case REGIBC:			/* LD reg,(BC) */
	case REGIDE:			/* LD reg,(DE) */
		if (base_op == 0x78) {
			/* only for A */
			switch (op) {
			case REGI:	/* LD A,I */
				len = 2;
				ops[0] = 0xed;
				ops[1] = 0x57;
				break;
			case REGR:	/* LD A,R */
				len = 2;
				ops[0] = 0xed;
				ops[1] = 0x5f;
				break;
			case REGIBC:	/* LD A,(BC) */
				len = 1;
				ops[0] = 0x0a;
				break;
			case REGIDE:	/* LD A,(DE) */
				len = 1;
				ops[0] = 0x1a;
				break;
			}
		}
		else {
			/* not A */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOREG:			/* operand isn't register */
		if (strncmp(sec, "(IX+", 4) == 0) {
			len = 3;	/* LD reg,(IX+d) */
			if (pass == 2) {
				ops[0] = 0xdd;
				ops[1] = base_op + 0x06;
				ops[2] = chk_sbyte(calc_val(sec + 4));
			}
		} else if (strncmp(sec, "(IY+", 4) == 0) {
			len = 3;	/* LD reg,(IY+d) */
			if (pass == 2) {
				ops[0] = 0xfd;
				ops[1] = base_op + 0x06;
				ops[2] = chk_sbyte(calc_val(sec + 4));
			}
		} else if (base_op == 0x78 && *sec == '('
					   && *(sec + strlen(sec) - 1) == ')') {
			/* only for A */
			len = 3;	/* LD A,(nn) */
			if (pass == 2) {
				i = calc_val(sec + 1);
				ops[0] = 0x3a;
				ops[1] = i & 255;
				ops[2] = i >> 8;
			}
		} else {		/* LD reg,n */
			len = 2;
			if (pass == 2) {
				ops[0] = base_opn;
				ops[1] = chk_byte(eval(sec));
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD IX[HL],? (undocumented)
 */
int ldixhl(int base_op, int base_opn, char *sec)
{
	register int op;
	register int len;

	switch (op = get_reg(sec)) {
	case REGA:			/* LD IX[HL],A (undocumented) */
	case REGB:			/* LD IX[HL],B (undocumented) */
	case REGC:			/* LD IX[HL],C (undocumented) */
	case REGD:			/* LD IX[HL],D (undocumented) */
	case REGE:			/* LD IX[HL],E (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + op;
		break;
	case REGIXH:			/* LD IX[HL],IXH (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x04;
		break;
	case REGIXL:			/* LD IX[HL],IXL (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x05;
		break;
	case NOREG:			/* LD IX[HL],n (undocumented) */
		len = 3;
		if (pass == 2) {
			ops[0] = 0xdd;
			ops[1] = base_opn;
			ops[2] = chk_byte(eval(sec));
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD IY[HL],? (undocumented)
 */
int ldiyhl(int base_op, int base_opn, char *sec)
{
	register int op;
	register int len;

	switch (op = get_reg(sec)) {
	case REGA:			/* LD IY[HL],A (undocumented) */
	case REGB:			/* LD IY[HL],B (undocumented) */
	case REGC:			/* LD IY[HL],C (undocumented) */
	case REGD:			/* LD IY[HL],D (undocumented) */
	case REGE:			/* LD IY[HL],E (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + op;
		break;
	case REGIYH:			/* LD IY[HL],IYH (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x04;
		break;
	case REGIYL:			/* LD IY[HL],IYL (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x05;
		break;
	case NOREG:			/* LD IY[HL],n (undocumented) */
		len = 3;
		if (pass == 2) {
			ops[0] = 0xfd;
			ops[1] = base_opn;
			ops[2] = chk_byte(eval(sec));
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD {BC,DE},?
 */
int ldbcde(int base_op, char *sec)
{
	register int i, len;

	switch (get_reg(sec)) {
	case NOREG:			/* operand isn't register */
		if (*sec == '(' && *(sec + strlen(sec) - 1) == ')') {
			len = 4;	/* LD {BC,DE},(nn) */
			if (pass == 2) {
				i = calc_val(sec + 1);
				ops[0] = 0xed;
				ops[1] = base_op + 0x4a;
				ops[2] = i & 0xff;
				ops[3] = i >> 8;
			}
		} else {
			len = 3;	/* LD {BC,DE},nn */
			if (pass == 2) {
				i = eval(sec);
				ops[0] = base_op;
				ops[1] = i & 0xff;
				ops[2] = i >> 8;
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD HL,?
 */
int ldhl(char *sec)
{
	register int i, len;

	switch (get_reg(sec)) {
	case NOREG:			/* operand isn't register */
		if (*sec == '(' && *(sec + strlen(sec) - 1) == ')') {
			len = 3;	/* LD HL,(nn) */
			if (pass == 2) {
				i = calc_val(sec + 1);
				ops[0] = 0x2a;
				ops[1] = i & 0xff;
				ops[2] = i >> 8;
			}
		} else {
			len = 3;	/* LD HL,nn */
			if (pass == 2) {
				i = eval(sec);
				ops[0] = 0x21;
				ops[1] = i & 0xff;
				ops[2] = i >> 8;
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD I[XY],?
 */
int ldixy(int prefix, char *sec)
{
	register int i, len;

	switch (get_reg(sec)) {
	case NOREG:			/* operand isn't register */
		if (*sec == '(' && *(sec + strlen(sec) - 1) == ')') {
			len = 4;	/* LD I[XY],(nn) */
			if (pass == 2) {
				i = calc_val(sec + 1);
				ops[0] = prefix;
				ops[1] = 0x2a;
				ops[2] = i & 0xff;
				ops[3] = i >> 8;
			}
		} else {
			len = 4;	/* LD I[XY],nn */
			if (pass == 2) {
				i = eval(sec);
				ops[0] = prefix;
				ops[1] = 0x21;
				ops[2] = i & 0xff;
				ops[3] = i >> 8;
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD SP,?
 */
int ldsp(char *sec)
{
	register int i, len;

	switch (get_reg(sec)) {
	case REGHL:			/* LD SP,HL */
		len = 1;
		ops[0] = 0xf9;
		break;
	case REGIX:			/* LD SP,IX */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = 0xf9;
		break;
	case REGIY:			/* LD SP,IY */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = 0xf9;
		break;
	case NOREG:			/* operand isn't register */
		if (*sec == '(' && *(sec + strlen(sec) - 1) == ')') {
			len = 4;	/* LD SP,(nn) */
			if (pass == 2) {
				i = calc_val(sec + 1);
				ops[0] = 0xed;
				ops[1] = 0x7b;
				ops[2] = i & 0xff;
				ops[3] = i >> 8;
			}
		} else {
			len = 3;	/* LD SP,nn */
			if (pass == 2) {
				i = eval(sec);
				ops[0] = 0x31;
				ops[1] = i & 0xff;
				ops[2] = i >> 8;
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD (HL),?
 */
int ldihl(char *sec)
{
	register int op;
	register int len;

	switch (op = get_reg(sec)) {
	case REGA:			/* LD (HL),A */
	case REGB:			/* LD (HL),B */
	case REGC:			/* LD (HL),C */
	case REGD:			/* LD (HL),D */
	case REGE:			/* LD (HL),E */
	case REGH:			/* LD (HL),H */
	case REGL:			/* LD (HL),L */
		len = 1;
		ops[0] = 0x70 + op;
		break;
	case NOREG:			/* LD (HL),n */
		len = 2;
		if (pass == 2) {
			ops[0] = 0x36;
			ops[1] = chk_byte(eval(sec));
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD (I[XY]+d),?
 */
int ldiixy(int prefix, char *sec)
{
	register int op;
	register int len;

	switch (op = get_reg(sec)) {
	case REGA:			/* LD (I[XY]+d),A */
	case REGB:			/* LD (I[XY]+d),B */
	case REGC:			/* LD (I[XY]+d),C */
	case REGD:			/* LD (I[XY]+d),D */
	case REGE:			/* LD (I[XY]+d),E */
	case REGH:			/* LD (I[XY]+d),H */
	case REGL:			/* LD (I[XY]+d),L */
		len = 3;
		if (pass == 2) {
			ops[0] = prefix;
			ops[1] = 0x70 + op;
			ops[2] = chk_sbyte(calc_val(operand + 4));
		}
		break;
	case NOREG:			/* LD (I[XY]+d),n */
		len = 4;
		if (pass == 2) {
			ops[0] = prefix;
			ops[1] = 0x36;
			ops[2] = chk_sbyte(calc_val(operand + 4));
			ops[3] = chk_byte(eval(sec));
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	LD (nn),?
 */
int ldinn(char *sec)
{
	register int i, len;

	switch (get_reg(sec)) {
	case REGA:			/* LD (nn),A */
		len = 3;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0x32;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case REGBC:			/* LD (nn),BC */
		len = 4;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0xed;
			ops[1] = 0x43;
			ops[2] = i & 0xff;
			ops[3] = i >> 8;
		}
		break;
	case REGDE:			/* LD (nn),DE */
		len = 4;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0xed;
			ops[1] = 0x53;
			ops[2] = i & 0xff;
			ops[3] = i >> 8;
		}
		break;
	case REGHL:			/* LD (nn),HL */
		len = 3;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0x22;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case REGSP:			/* LD (nn),SP */
		len = 4;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0xed;
			ops[1] = 0x73;
			ops[2] = i & 0xff;
			ops[3] = i >> 8;
		}
		break;
	case REGIX:			/* LD (nn),IX */
		len = 4;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0xdd;
			ops[1] = 0x22;
			ops[2] = i & 0xff;
			ops[3] = i >> 8;
		}
		break;
	case REGIY:			/* LD (nn),IY */
		len = 4;
		if (pass == 2) {
			i = calc_val(operand + 1);
			ops[0] = 0xfd;
			ops[1] = 0x22;
			ops[2] = i & 0xff;
			ops[3] = i >> 8;
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	ADD ?,?
 */
int op_add(int dummy1, int dummy2)
{
	register int len;
	register char *sec;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (get_reg(tmp)) {
	case REGA:			/* ADD A,? */
		len = aluop(0x80, sec);
		break;
	case REGHL:			/* ADD HL,? */
		len = addhl(sec);
		break;
	case REGIX:			/* ADD IX,? */
		len = addix(sec);
		break;
	case REGIY:			/* ADD IY,? */
		len = addiy(sec);
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	ADD HL,?
 */
int addhl(char *sec)
{
	switch (get_reg(sec)) {
	case REGBC:			/* ADD HL,BC */
		ops[0] = 0x09;
		break;
	case REGDE:			/* ADD HL,DE */
		ops[0] = 0x19;
		break;
	case REGHL:			/* ADD HL,HL */
		ops[0] = 0x29;
		break;
	case REGSP:			/* ADD HL,SP */
		ops[0] = 0x39;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	ADD IX,?
 */
int addix(char *sec)
{
	switch (get_reg(sec)) {
	case REGBC:			/* ADD IX,BC */
		ops[0] = 0xdd;
		ops[1] = 0x09;
		break;
	case REGDE:			/* ADD IX,DE */
		ops[0] = 0xdd;
		ops[1] = 0x19;
		break;
	case REGIX:			/* ADD IX,IX */
		ops[0] = 0xdd;
		ops[1] = 0x29;
		break;
	case REGSP:			/* ADD IX,SP */
		ops[0] = 0xdd;
		ops[1] = 0x39;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_ILLOPE);
	}
	return(2);
}

/*
 *	ADD IY,?
 */
int addiy(char *sec)
{
	switch (get_reg(sec)) {
	case REGBC:			/* ADD IY,BC */
		ops[0] = 0xfd;
		ops[1] = 0x09;
		break;
	case REGDE:			/* ADD IY,DE */
		ops[0] = 0xfd;
		ops[1] = 0x19;
		break;
	case REGIY:			/* ADD IY,IY */
		ops[0] = 0xfd;
		ops[1] = 0x29;
		break;
	case REGSP:			/* ADD IY,SP */
		ops[0] = 0xfd;
		ops[1] = 0x39;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_ILLOPE);
	}
	return(2);
}

/*
 *	SBC ?,? and ADC ?,?
 */
int op_sbadc(int base_op, int base_op16)
{
	register int len;
	register char *sec;

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (get_reg(tmp)) {
	case REGA:			/* SBC/ADC A,? */
		len = aluop(base_op, sec);
		break;
	case REGHL:			/* SBC/ADC HL,? */
		len = sbadchl(base_op16, sec);
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	SBC HL,? and ADC HL,?
 */
int sbadchl(int base_op, char *sec)
{
	switch (get_reg(sec)) {
	case REGBC:			/* SBC/ADC HL,BC */
		ops[0] = 0xed;
		ops[1] = base_op;
		break;
	case REGDE:			/* SBC/ADC HL,DE */
		ops[0] = 0xed;
		ops[1] = base_op + 0x10;
		break;
	case REGHL:			/* SBC/ADC HL,HL */
		ops[0] = 0xed;
		ops[1] = base_op + 0x20;
		break;
	case REGSP:			/* SBC/ADC HL,SP */
		ops[0] = 0xed;
		ops[1] = base_op + 0x30;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_ILLOPE);
	}
	return(2);
}

/*
 *	DEC and INC
 */
int op_decinc(int base_op, int base_op16)
{
	register int len, op;

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGA:			/* INC/DEC A */
	case REGB:			/* INC/DEC B */
	case REGC:			/* INC/DEC C */
	case REGD:			/* INC/DEC D */
	case REGE:			/* INC/DEC E */
	case REGH:			/* INC/DEC H */
	case REGL:			/* INC/DEC L */
	case REGIHL:			/* INC/DEC (HL) */
		len = 1;
		ops[0] = base_op + (op << 3);
		break;
	case REGBC:			/* INC/DEC BC */
		len = 1;
		ops[0] = base_op16;
		break;
	case REGDE:			/* INC/DEC DE */
		len = 1;
		ops[0] = base_op16 + 0x10;
		break;
	case REGHL:			/* INC/DEC HL */
		len = 1;
		ops[0] = base_op16 + 0x20;
		break;
	case REGSP:			/* INC/DEC SP */
		len = 1;
		ops[0] = base_op16 + 0x30;
		break;
	case REGIX:			/* INC/DEC IX */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op16 + 0x20;
		break;
	case REGIY:			/* INC/DEC IY */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op16 + 0x20;
		break;
	case REGIXH:			/* INC/DEC IXH (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x20;
		break;
	case REGIXL:			/* INC/DEC IXL (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x28;
		break;
	case REGIYH:			/* INC/DEC IYH (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x20;
		break;
	case REGIYL:			/* INC/DEC IYL (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x28;
		break;
	case NOREG:			/* operand isn't register */
		if (strncmp(operand, "(IX+", 4) == 0) {
			len = 3;	/* INC/DEC (IX+d) */
			if (pass == 2) {
				ops[0] = 0xdd;
				ops[1] = base_op + 0x30;
				ops[2] = chk_sbyte(calc_val(operand + 4));
			}
		} else if (strncmp(operand, "(IY+", 4) == 0) {
			len = 3;	/* INC/DEC (IY+d) */
			if (pass == 2) {
				ops[0] = 0xfd;
				ops[1] = base_op + 0x30;
				ops[2] = chk_sbyte(calc_val(operand + 4));
			}
		} else {
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	SUB, AND, XOR, OR, CP
 */
int op_alu(int base_op, int dummy)
{
	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	return(aluop(base_op, operand));
}

/*
 *	ADD A, ADC A, SUB, SBC A, AND, XOR, OR, CP
 */
int aluop(int base_op, char *sec)
{
	register int len, op;

	switch (op = get_reg(sec)) {
	case REGA:			/* ALUOP {A,}A */
	case REGB:			/* ALUOP {A,}B */
	case REGC:			/* ALUOP {A,}C */
	case REGD:			/* ALUOP {A,}D */
	case REGE:			/* ALUOP {A,}E */
	case REGH:			/* ALUOP {A,}H */
	case REGL:			/* ALUOP {A,}L */
	case REGIHL:			/* ALUOP {A,}(HL) */
		len = 1;
		ops[0] = base_op + op;
		break;
	case REGIXH:			/* ALUOP {A,}IXH (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x04;
		break;
	case REGIXL:			/* ALUOP {A,}IXL (undocumented) */
		len = 2;
		ops[0] = 0xdd;
		ops[1] = base_op + 0x05;
		break;
	case REGIYH:			/* ALUOP {A,}IYH (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x04;
		break;
	case REGIYL:			/* ALUOP {A,}IYL (undocumented) */
		len = 2;
		ops[0] = 0xfd;
		ops[1] = base_op + 0x05;
		break;
	case NOREG:			/* operand isn't register */
		if (strncmp(sec, "(IX+", 4) == 0) {
			len = 3;	/* ALUOP {A,}(IX+d) */
			if (pass == 2) {
				ops[0] = 0xdd;
				ops[1] = base_op + 0x06;
				ops[2] = chk_sbyte(calc_val(sec + 4));
			}
		} else if (strncmp(sec, "(IY+", 4) == 0) {
			len = 3;	/* ALUOP {A,}(IY+d) */
			if (pass == 2) {
				ops[0] = 0xfd;
				ops[1] = base_op + 0x06;
				ops[2] = chk_sbyte(calc_val(sec + 4));
			}
		} else {
			len = 2;	/* ALUOP {A,}n */
			if (pass == 2) {
				ops[0] = base_op + 0x46;
				ops[1] = chk_byte(eval(sec));
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	OUT
 */
int op_out(int dummy1, int dummy2)
{
	register int op;
	register char *p;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		if (strncmp(operand, "(C),", 4) == 0) {
			switch(op = get_reg(operand + 4)) {
			case REGA:	/* OUT (C),A */
			case REGB:	/* OUT (C),B */
			case REGC:	/* OUT (C),C */
			case REGD:	/* OUT (C),D */
			case REGE:	/* OUT (C),E */
			case REGH:	/* OUT (C),H */
			case REGL:	/* OUT (C),L */
				ops[0] = 0xed;
				ops[1] = 0x41 + (op << 3);
				break;
			case NOOPERA:	/* missing operand */
				ops[0] = 0;
				ops[1] = 0;
				asmerr(E_MISOPE);
				break;
			default:
				if (undoc_flag && operand[4] == '0') {
					ops[0] = 0xed;	/* OUT (C),0 (undocumented) */
					ops[1] = 0x71;
				} else {	/* invalid operand */
					ops[0] = 0;
					ops[1] = 0;
					asmerr(E_ILLOPE);
				}
			}
		} else {
			/* check syntax for OUT (n),A */
			p = strchr(operand, ')');
			if (operand[0] != '(' || strncmp(p, "),A", 3)) {
				ops[0] = 0;
				ops[1] = 0;
				asmerr(E_ILLOPE);
			}
			ops[0] = 0xd3;	/* OUT (n),A */
			ops[1] = chk_byte(calc_val(operand + 1));
		}
	}
	return(2);
}

/*
 *	IN
 */
int op_in(int dummy1, int dummy2)
{
	register char *sec;
	register int op;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	if ((sec = get_first_second())) {
		if (*sec == '(') {
			switch (op = get_reg(tmp)) {
			case REGA:
				if (strncmp(sec, "(C)", 3) == 0) {
					ops[0] = 0xed;	/* IN A,(C) */
					ops[1] = 0x78;
				} else if (pass == 2) {
					ops[0] = 0xdb;	/* IN A,(n) */
					ops[1] = chk_byte(calc_val(sec + 1));
				}
				break;
			case REGB:			/* IN B,(C) */
			case REGC:			/* IN C,(C) */
			case REGD:			/* IN D,(C) */
			case REGE:			/* IN E,(C) */
			case REGH:			/* IN H,(C) */
			case REGL:			/* IN L,(C) */
				ops[0] = 0xed;
				ops[1] = 0x40 + (op << 3);
				break;
			default:
				if (undoc_flag
				    && strncmp(operand, "F,(C)", 5) == 0) {
					ops[0] = 0xed;	/* IN F,(C) (undocumented) */
					ops[1] = 0x70;
				} else {		/* invalid operand */
					ops[0] = 0;
					ops[1] = 0;
					asmerr(E_ILLOPE);
				}
			}
		} else {				/* invalid operand */
			ops[0] = 0;
			ops[1] = 0;
			asmerr(E_ILLOPE);
		}
	} else {					/* missing operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_MISOPE);
	}
	return(2);
}

/*
 *	RLC, RRC, RL, RR, SLA, SRA, SLL, SRL, BIT, RES, SET
 */
int op_cbgrp(int base_op, int dummy)
{
	register char *sec;
	register int len;
	register int i;
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	len = 2;
	i = 0;
	if (base_op >= 0x40) {		/* TRSBIT n,? */
		sec = get_first_second();
		if (pass == 2) {
			i = eval(tmp);
			if (i < 0 || i > 7)
				asmerr(E_VALOUT);
		}
	} else				/* ROTSHF ? */
		sec = operand;
	switch (op = get_reg(sec)) {
	case REGA:			/* CBOP {n,}A */
	case REGB:			/* CBOP {n,}B */
	case REGC:			/* CBOP {n,}C */
	case REGD:			/* CBOP {n,}D */
	case REGE:			/* CBOP {n,}E */
	case REGH:			/* CBOP {n,}H */
	case REGL:			/* CBOP {n,}L */
	case REGIHL:			/* CBOP {n,}(HL) */
		ops[0] = 0xcb;
		ops[1] = base_op + (i << 3) + op;
		break;
	case NOREG:			/* CBOP {n,}(I[XY]+d){,reg} */
		len = 4;
		if (pass == 2) {
			if (strncmp(sec, "(IX+", 4) == 0)
				cbgrp_iixy(0xdd, base_op, i, sec);
			else if (strncmp(sec, "(IY+", 4) == 0)
				cbgrp_iixy(0xfd, base_op, i, sec);
			else {		/* invalid operand */
				ops[0] = 0;
				ops[1] = 0;
				ops[2] = 0;
				ops[3] = 0;
				asmerr(E_ILLOPE);
			}
		}
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		ops[1] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	CBOP {n,}(I[XY]+d){,reg}
 */
void cbgrp_iixy(int prefix, int base_op, int bit, char *sec)
{
	register char *p;
	register int op;

	if ((p = strrchr(sec, ')')) && *(p + 1) == ',') {
		if (undoc_flag && base_op != 0x40) {
			/* not for BIT */
			switch (op = get_reg(p + 2)) {
			case REGA:	/* CBOP {n,}(I[XY]+d),A (undocumented) */
			case REGB:	/* CBOP {n,}(I[XY]+d),B (undocumented) */
			case REGC:	/* CBOP {n,}(I[XY]+d),C (undocumented) */
			case REGD:	/* CBOP {n,}(I[XY]+d),D (undocumented) */
			case REGE:	/* CBOP {n,}(I[XY]+d),E (undocumented) */
			case REGH:	/* CBOP {n,}(I[XY]+d),H (undocumented) */
			case REGL:	/* CBOP {n,}(I[XY]+d),L (undocumented) */
				ops[0] = prefix;
				ops[1] = 0xcb;
				ops[2] = chk_sbyte(calc_val(sec + 4));
				ops[3] = base_op + (bit << 3) + op;
				break;
			default:	/* invalid operand */
				ops[0] = 0;
				ops[1] = 0;
				ops[2] = 0;
				ops[3] = 0;
				asmerr(E_ILLOPE);
			}
		} else {		/* invalid operand */
			ops[0] = 0;
			ops[1] = 0;
			ops[2] = 0;
			ops[3] = 0;
			asmerr(E_ILLOPE);
		}
	} else {			/* CBOP {n,}(I[XY]+d) */
		ops[0] = prefix;
		ops[1] = 0xcb;
		ops[2] = chk_sbyte(calc_val(sec + 4));
		ops[3] = base_op + (bit << 3) + 0x06;
	}
}

/*
 *	8080 MOV
 */
int op8080_mov(int dummy1, int dummy2)
{
	register char *sec;
	register int op1, op2;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (op1 = get_reg(tmp)) {
	case REGA:			/* MOV A,reg */
	case REGB:			/* MOV B,reg */
	case REGC:			/* MOV C,reg */
	case REGD:			/* MOV D,reg */
	case REGE:			/* MOV E,reg */
	case REGH:			/* MOV H,reg */
	case REGL:			/* MOV L,reg */
	case REGM:			/* MOV M,reg */
		switch (op2 = get_reg(sec)) {
		case REGA:		/* MOV reg,A */
		case REGB:		/* MOV reg,B */
		case REGC:		/* MOV reg,C */
		case REGD:		/* MOV reg,D */
		case REGE:		/* MOV reg,E */
		case REGH:		/* MOV reg,H */
		case REGL:		/* MOV reg,L */
		case REGM:		/* MOV reg,M */
			if (op1 == REGM && op2 == REGM) {
				ops[0] = 0;
				asmerr(E_ILLOPE);
			}
			else
				ops[0] = 0x40 + (op1 << 3) + op2;
			break;
		case NOOPERA:		/* missing operand */
			ops[0] = 0;
			asmerr(E_MISOPE);
			break;
		default:		/* invalid operand */
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 ADC, ADD, ANA, CMP, ORA, SBB, SUB, XRA
 */
int op8080_alu(int base_op, int dummy)
{
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGA:			/* ALUOP A */
	case REGB:			/* ALUOP B */
	case REGC:			/* ALUOP C */
	case REGD:			/* ALUOP D */
	case REGE:			/* ALUOP E */
	case REGH:			/* ALUOP H */
	case REGL:			/* ALUOP L */
	case REGM:			/* ALUOP M */
		ops[0] = base_op + op;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 DCR and INR
 */
int op8080_decinc(int base_op, int dummy)
{
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGA:			/* DEC/INC A */
	case REGB:			/* DEC/INC B */
	case REGC:			/* DEC/INC C */
	case REGD:			/* DEC/INC D */
	case REGE:			/* DEC/INC E */
	case REGH:			/* DEC/INC H */
	case REGL:			/* DEC/INC L */
	case REGM:			/* DEC/INC M */
		ops[0] = base_op + (op << 3);
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 INX, DAD, DCX
 */
int op8080_reg16(int base_op, int dummy)
{
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGB:			/* INX/DAD/DCX B */
	case REGD:			/* INX/DAD/DCX D */
	case REGH:			/* INX/DAD/DCX H */
		ops[0] = base_op + (op << 3);
		break;
	case REGSP:			/* INX/DAD/DCX SP */
		ops[0] = base_op + 0x30;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 STAX and LDAX
 */
int op8080_regbd(int base_op, int dummy)
{
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGB:			/* STAX/LDAX B */
	case REGD:			/* STAX/LDAX D */
		ops[0] = base_op + (op << 3);
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 ACI, ADI, ANI, CPI, ORI, SBI, SUI, XRI, OUT, IN
 */
int op8080_imm(int base_op, int dummy)
{
	register int len;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (get_reg(operand)) {
	case NOREG:			/* IMMOP n */
		len = 2;
		if (pass == 2) {
			ops[0] = base_op;
			ops[1] = chk_byte(eval(operand));
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	8080 RST
 */
int op8080_rst(int dummy1, int dummy2)
{
	register int op;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (*label)
			put_label();
	} else {			/* PASS 2 */
		op = eval(operand);
		if (op < 0 || op > 7) {
			ops[0] = 0;
			asmerr(E_VALOUT);
		} else
			ops[0] = 0xc7 + (op << 3);
	}
	return(1);
}
/*
 *	8080 PUSH and POP
 */
int op8080_pupo(int base_op, int dummy)
{
	register int op;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (op = get_reg(operand)) {
	case REGB:			/* PUSH/POP B */
	case REGD:			/* PUSH/POP D */
	case REGH:			/* PUSH/POP H */
		ops[0] = base_op + (op << 3);
		break;
	case REGPSW:			/* PUSH/POP PSW */
		ops[0] = base_op + 0x30;
		break;
	case NOOPERA:			/* missing operand */
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(1);
}

/*
 *	8080 SHLD, LHLD, STA, LDA
 *	     JMP, JNZ, JZ, JNC, JC, JPO, JPE, JP, JM
 *	     CALL, CNZ, CZ, CNC, CC, CPO, CPE, CP, CM
 */
int op8080_addr(int base_op, int dummy)
{
	register int i, len;

	UNUSED(dummy);

	if (pass == 1)
		if (*label)
			put_label();
	switch (get_reg(operand)) {
	case NOREG:			/* OP nn */
		len = 3;
		if (pass == 2) {
			i = eval(operand);
			ops[0] = base_op;
			ops[1] = i & 0xff;
			ops[2] = i >> 8;
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	8080 MVI
 */
int op8080_mvi(int dummy1, int dummy2)
{
	register int len;
	register char *sec;
	register int op;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (op = get_reg(tmp)) {
	case REGA:			/* MVI A,n */
	case REGB:			/* MVI B,n */
	case REGC:			/* MVI C,n */
	case REGD:			/* MVI D,n */
	case REGE:			/* MVI E,n */
	case REGH:			/* MVI H,n */
	case REGL:			/* MVI L,n */
	case REGM:			/* MVI M,n */
		switch (get_reg(sec)) {
		case NOREG:		/* MVI reg,n */
			len = 2;
			if (pass == 2) {
				ops[0] = 0x06 + (op << 3);
				ops[1] = chk_byte(eval(sec));
			}
			break;
		case NOOPERA:		/* missing operand */
			len = 1;
			ops[0] = 0;
			asmerr(E_MISOPE);
			break;
		default:		/* invalid operand */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	8080 LXI
 */
int op8080_lxi(int dummy1, int dummy2)
{
	register int i, len;
	register char *sec;
	register int op;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1)
		if (*label)
			put_label();
	sec = get_first_second();
	switch (op = get_reg(tmp)) {
	case REGB:			/* LXI B,nn */
	case REGD:			/* LXI D,nn */
	case REGH:			/* LXI H,nn */
	case REGSP:			/* LXI SP,nn */
		switch (get_reg(sec)) {
		case NOREG:		/* LXI reg,nn */
			len = 3;
			if (pass == 2) {
				i = eval(sec);
				if (op == REGSP)
					ops[0] = 0x31;
				else
					ops[0] = 0x01 + (op << 3);
				ops[1] = i & 0xff;
				ops[2] = i >> 8;
			}
			break;
		case NOOPERA:		/* missing operand */
			len = 1;
			ops[0] = 0;
			asmerr(E_MISOPE);
			break;
		default:		/* invalid operand */
			len = 1;
			ops[0] = 0;
			asmerr(E_ILLOPE);
		}
		break;
	case NOOPERA:			/* missing operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_MISOPE);
		break;
	default:			/* invalid operand */
		len = 1;
		ops[0] = 0;
		asmerr(E_ILLOPE);
	}
	return(len);
}

/*
 *	copies the first operand from "operand" to "tmp"
 *	and returns a pointer to the second operand for
 *	opcodes:	opcode destination,source
 *	if source is missing returns NULL
 */
char *get_first_second(void)
{
	register char *p1, *p2;

	p1 = operand;
	p2 = tmp;
	while (*p1 != ',' && *p1 != '\0')
		*p2++ = *p1++;
	*p2 = '\0';
	if (*p1 == ',')
		return(p1 + 1);
	else
		return(NULL);
}

/*
 *	computes value of the following expressions:
 *	LD A,(IX+7)   LD A,(1234)
 *		 --         -----
 */
int calc_val(char *s)
{
	register char *p;
	register int i;

	if ((p = strrchr(s, ')')) == NULL) {
		asmerr(E_MISPAR);
		return(0);
	}
	i = p - s;
	strncpy(tmp, s, i);
	*(tmp + i) = '\0';
	return(eval(tmp));
}
