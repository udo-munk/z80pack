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
 *	28-OCT-2017 added variable symbol length and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
 */

/*
 *	processing of all PSEUDO ops
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

/* z80amain.c */
extern void fatal(int, char *);
extern void p1_file(char *);
extern void p2_file(char *);
extern char *copy_arg(char *, char *, int *);

/* z80anum.c */
extern int eval(char *);
extern int chk_byte(int);

/* z80aout.c */
extern void asmerr(int);
extern void lst_header(void);
extern void lst_attl(void);
extern void lst_line(int, int);
extern void obj_org(int);
extern void obj_fill(int);
extern void obj_fill_value(int, int);

/* z80atab.c */
extern struct sym *get_sym(char *);
extern int put_sym(char *, int);

/*
 *	.8080 and .Z80
 */
int op_opset(int op_code, int dummy)
{
	UNUSED(dummy);

	ad_mode = AD_NONE;
	switch (op_code) {
	case 1:				/* .Z80 */
		opset = OPSET_Z80;
		break;
	case 2:				/* .8080 */
		opset = OPSET_8080;
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_opset");
	}
	return(0);
}

/*
 *	ORG, .PHASE, .DEPHASE
 */
int op_org(int op_code, int dummy)
{
	register int i;

	UNUSED(dummy);

	ad_mode = AD_NONE;
	switch (op_code) {
	case 1:				/* ORG */
		if (phs_flag) {
			asmerr(E_ORGPHS);
			return(0);
		}
		i = eval(operand);
		if (pass == 1) {	/* PASS 1 */
			if (!load_flag) {
				load_addr = i;
				load_flag = 1;
			}
		} else			/* PASS 2 */
			obj_org(i);
		rpc = pc = i;
		break;
	case 2:				/* .PHASE */
		if (phs_flag)
			asmerr(E_PHSNEST);
		else {
			phs_flag = 1;
			pc = eval(operand);
		}
		break;
	case 3:				/* .DEPHASE */
		if (!phs_flag)
			asmerr(E_MISPHS);
		else {
			phs_flag = 0;
			pc = rpc;
		}
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_org");
	}
	return(0);
}

/*
 *	.RADIX
 */
int op_radix(int dummy1, int dummy2)
{
	int i;

	UNUSED(dummy1);
	UNUSED(dummy2);

	ad_mode = AD_NONE;
	i = eval(operand);
	if (i < 2 || i > 16)
		asmerr(E_VALOUT);
	else
		radix = i;
	return(0);
}

/*
 *	EQU
 */
int op_equ(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (get_sym(label) == NULL) {
			ad_addr = eval(operand);
			if (put_sym(label, ad_addr))
				fatal(F_OUTMEM, "symbols");
		} else
			asmerr(E_MULSYM);
	} else {			/* PASS 2 */
		ad_mode = AD_ADDR;
		ad_addr = eval(operand);
	}
	return(0);
}

/*
 *	DEFL, ASET, and SET (in 8080 mode)
 */
int op_dl(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	ad_mode = AD_ADDR;
	ad_addr = eval(operand);
	if (put_sym(label, ad_addr))
		fatal(F_OUTMEM, "symbols");
	return(0);
}

/*
 *	DEFS and DS
 */
int op_ds(int dummy1, int dummy2)
{
	register char *p;
	register int count, value;

	UNUSED(dummy1);
	UNUSED(dummy2);

	ad_mode = AD_ADDR;
	ad_addr = pc;
	p = operand;
	if (*p == '\0')
		asmerr(E_MISOPE);
	else {
		p = copy_arg(tmp, p, NULL);
		count = eval(tmp);
		if (pass == 2) {
			if (*p++ == ',') {
				if (*p == '\0')
					asmerr(E_MISOPE);
				else {
					value = eval(p);
					obj_fill_value(count, value);
				}
			} else
				obj_fill(count);
		}
		pc += count;
		rpc += count;
	}
	return(0);
}

/*
 *	DEFB, DB, DEFM, DEFC, DC, DEFZ
 */
int op_db(int op_code, int dummy)
{
	register int i;
	register char *p;
	register char *s;
	int sf;

	UNUSED(dummy);

	i = 0;
	p = operand;
	while (*p != '\0') {
		p = copy_arg(tmp, p, &sf);
		if (sf < 0) {		/* a non-terminated string */
			asmerr(E_MISDEL);
			goto delim_error;
		} else if (sf > 0) {	/* a valid string, delimiter in *tmp */
			s = tmp + 1;
			while (1) {
				/* check for double delim */
				if (*s == *tmp && *++s != *tmp)
					break;
				ops[i] = *s++;
				if (++i >= OPCARRAY)
				    fatal(F_INTERN, "op-code buffer overflow");
			}
		} else {		/* an expression */
			if (*tmp != '\0') {
				if (pass == 2)
					ops[i] = chk_byte(eval(tmp));
				if (++i >= OPCARRAY)
				    fatal(F_INTERN, "op-code buffer overflow");
			}
		}
		if (*p == ',')
			p++;
	}
	switch (op_code) {
	case 1:				/* DEFB, DB, DEFM */
		break;
	case 2:				/* DEFC, DC */
		if (i > 0)
			ops[i - 1] |= 0x80;
		break;
	case 3:				/* DEFZ */
		ops[i++] = '\0';
		if (i >= OPCARRAY)
			fatal(F_INTERN, "op-code buffer overflow");
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_db");
	}
delim_error:
	return(i);
}

/*
 *	DEFW and DW
 */
int op_dw(int dummy1, int dummy2)
{
	register int i, temp;
	register char *p;

	UNUSED(dummy1);
	UNUSED(dummy2);

	p = operand;
	i = 0;
	while (*p != '\0') {
		p = copy_arg(tmp, p, NULL);
		if (*tmp != '\0') {
			if (pass == 2) {
				temp = eval(tmp);
				ops[i] = temp & 0xff;
				ops[i + 1] = temp >> 8;
			}
			i += 2;
			if (i >= OPCARRAY)
				fatal(F_INTERN, "op-code buffer overflow");
		}
		if (*p == ',')
			p++;
	}
	return(i);
}

/*
 *	EJECT, LIST, NOLIST, PAGE, PRINT, TITLE, INCLUDE
 */
int op_misc(int op_code, int dummy)
{
	register char *p, *d, *s;
	static char fn[LENFN];
	static int incnest;
	static struct inc incl[INCNEST];

	UNUSED(dummy);

	ad_mode = AD_NONE;
	switch(op_code) {
	case 1:				/* EJECT */
		if (pass == 2)
			p_line = ppl;
		break;
	case 2:				/* LIST */
		if (pass == 2)
			list_flag = 1;
		break;
	case 3:				/* NOLIST */
		if (pass == 2)
			list_flag = 0;
		break;
	case 4:				/* PAGE */
		if (pass == 2)
			ppl = eval(operand);
		break;
	case 5:				/* PRINT */
		if (pass == 1) {
			p = operand;
			if (*p == STRDEL || *p == STRDEL2) {
				p++;
				while (1) {
					/* check for double delim */
					if (*p == *operand && *++p != *operand)
						break;
					if (*p == '\0') {
						putchar('\n');
						asmerr(E_MISDEL);
						return(0);
					}
					putchar(*p++);
				}
			} else
				fputs(operand, stdout);
			putchar('\n');
		}
		break;
	case 6:				/* INCLUDE */
		if (incnest >= INCNEST) {
			asmerr(E_INCNEST);
			break;
		}
		incl[incnest].inc_line = c_line;
		incl[incnest].inc_fn = srcfn;
		incl[incnest].inc_fp = srcfp;
		incnest++;
		p = operand;
		d = fn;
		while (!isspace((unsigned char) *p) && *p != COMMENT
						    && *p != '\0')
			*d++ = *p++;
		*d = '\0';
		if (pass == 1) {	/* PASS 1 */
			if (ver_flag)
				printf("   Include %s\n", fn);
			p1_file(fn);
		} else {		/* PASS 2 */
			lst_line(0, 0);
			if (ver_flag)
				printf("   Include %s\n", fn);
			p2_file(fn);
		}
		incnest--;
		c_line = incl[incnest].inc_line;
		srcfn = incl[incnest].inc_fn;
		srcfp = incl[incnest].inc_fp;
		if (ver_flag)
			printf("   Resume  %s\n", srcfn);
		if (list_flag && (pass == 2)) {
			lst_header();
			lst_attl();
		}
		ad_mode = AD_SUPPR;
		break;
	case 7:				/* TITLE */
		if (pass == 2) {
			p = operand;
			d = title;
			if (*p == STRDEL || *p == STRDEL2) {
				s = p + 1;
				while (1) {
					/* check for double delim */
					if (*s == *p && *++s != *p)
						break;
					if (*s == '\0') {
						asmerr(E_MISDEL);
						break;
					}
					*d++ = *s++;
				}
			} else
				while (*p != '\0' && *p != COMMENT)
					*d++ = *p++;
			*d = '\0';
		}
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_misc");
		break;
	}
	return(0);
}

/*
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, COND, IF, IFT, IFE, IFF,
 *	IF1, IF2, IFB, IFNB, IFIDN, IFDIF
 */
int op_cond(int op_code, int dummy)
{
	register char *p, *p1;
	static int condnest[IFNEST];

	UNUSED(dummy);

	ad_mode = AD_NONE;
	if (op_code < 90) {
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			return(0);
		}
		condnest[iflevel++] = gencode;
		if (!gencode)
			return(0);
		switch(op_code) {
		case 1:				/* IFDEF */
		case 2:				/* IFNDEF */
			gencode = (get_sym(operand) != NULL);
			break;
		case 3:				/* IFEQ */
		case 4:				/* IFNEQ */
			p = copy_arg(tmp, operand, NULL);
			if (*tmp == '\0' || *p != ',' || *++p == '\0') {
				asmerr(E_MISOPE);
				return(0);
			}
			gencode = (eval(tmp) == eval(p));
			break;
		case 5:				/* COND, IF, and IFT */
		case 6:				/* IFE and IFF */
			gencode = (eval(operand) != 0);
			break;
		case 7:				/* IF1 */
		case 8:				/* IF2 */
			gencode = (pass == 1);
			break;
		case 9:				/* IFB */
		case 10:			/* IFNB */
		case 11:			/* IFIDN */
		case 12:			/* IFDIF */
			p = operand;
			if (*p == '\0') {
				asmerr(E_MISOPE);
				return(0);
			}
			if (*p != '<') {
				asmerr(E_ILLOPE);
				return(0);
			}
			p1 = tmp;
			while (*p != '\0' && *p != '>')
				*p1++ = *p++;
			if ((*p1++ = *p++) != '>') {
				asmerr(E_MISPAR);
				return(0);
			}
			*p1 = '\0';
			if (op_code == 9 || op_code == 10) /* IFB and IFNB */
				gencode = ((p1 - tmp) == 2);
			else {			/* IFIDN and IFDIF */
				if (*p++ != ',') {
					asmerr(E_MISOPE);
					return(0);
				}
				while (isspace((unsigned char) *p))
					p++;
				if (*p != '<') {
					asmerr(E_ILLOPE);
					return(0);
				}
				p1 = p;
				while (*p != '\0' && *p != '>')
					p++;
				if (*p++ != '>') {
					asmerr(E_MISPAR);
					return(0);
				}
				*p = '\0';
				gencode = (strcmp(tmp, p1) == 0);
			}
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
		if ((op_code & 1) == 0)		/* negate for negative IF variation */
			gencode = !gencode;
	} else {
		if (iflevel == 0) {
			asmerr(E_MISIFF);
			return(0);
		}
		switch(op_code) {
		case 98:			/* ELSE */
			if (condnest[iflevel - 1] == 1)
				gencode = !gencode;
			break;
		case 99:			/* ENDIF and ENDC */
			gencode = condnest[--iflevel];
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
	}
	return(0);
}

/*
 *	EXTRN, EXTERNAL, EXT and PUBLIC, ENT, ENTRY, GLOBAL, and ABS, ASEG
 */
int op_glob(int op_code, int dummy)
{
	UNUSED(dummy);

	ad_mode = AD_NONE;
	switch(op_code) {
	case 1:				/* EXTRN, EXTERNAL, EXT */
		break;
	case 2:				/* PUBLIC, ENT, ENTRY, GLOBAL */
		break;
	case 3:				/* ABS, ASEG */
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_glob");
		break;
	}
	return(0);
}

/*
 *	END
 */
int op_end(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 2 && *operand != '\0')
		start_addr = eval(operand);
	return(0);
}
