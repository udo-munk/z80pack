/*
 *	Z80 - Macro - Assembler
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
 *	25-OCT-2022 Intel-like macros (TE)
 */

/*
 *	processing of all PSEUDO ops except macro related
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

/* z80amain.c */
extern void fatal(int, const char *);
extern void process_file(char *);
extern char *next_arg(char *, int *);

/* z80anum.c */
extern WORD eval(char *);
extern BYTE chk_byte(WORD);

/* z80aout.c */
extern void asmerr(int);
extern void lst_header(void);
extern void lst_attl(void);
extern void obj_org(WORD);
extern void obj_fill(WORD);
extern void obj_fill_value(WORD, WORD);

/* z80atab.c */
extern struct sym *get_sym(char *);
extern int put_sym(char *, int);

/*
 *	.Z80 and .8080
 */
int op_opset(BYTE op_code, BYTE dummy)
{
	UNUSED(dummy);

	a_mode = A_NONE;
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
int op_org(BYTE op_code, BYTE dummy)
{
	register WORD n;

	UNUSED(dummy);

	a_mode = A_NONE;
	switch (op_code) {
	case 1:				/* ORG */
		if (phs_flag) {
			asmerr(E_ORGPHS);
			return(0);
		}
		n = eval(operand);
		if (pass == 1) {	/* PASS 1 */
			if (!load_flag) {
				load_addr = n;
				load_flag = 1;
			}
		} else			/* PASS 2 */
			obj_org(n);
		rpc = pc = n;
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
int op_radix(BYTE dummy1, BYTE dummy2)
{
	int i;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
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
int op_equ(BYTE dummy1, BYTE dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 1) {		/* PASS 1 */
		if (get_sym(label) == NULL) {
			a_addr = eval(operand);
			if (put_sym(label, a_addr))
				fatal(F_OUTMEM, "symbols");
		} else
			asmerr(E_MULSYM);
	} else {			/* PASS 2 */
		a_mode = A_EQU;
		a_addr = eval(operand);
	}
	return(0);
}

/*
 *	DEFL, ASET, and SET (in 8080 mode)
 */
int op_dl(BYTE dummy1, BYTE dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_SET;
	a_addr = eval(operand);
	if (put_sym(label, a_addr))
		fatal(F_OUTMEM, "symbols");
	return(0);
}

/*
 *	DEFS and DS
 */
int op_ds(BYTE dummy1, BYTE dummy2)
{
	register char *p;
	register WORD count, value;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_DS;
	a_addr = pc;
	p = next_arg(operand, NULL);
	count = eval(operand);
	if (pass == 2) {
		if (p != NULL) {
			value = eval(p);
			obj_fill_value(count, value);
		} else
			obj_fill(count);
	}
	return(count);
}

/*
 *	DEFB, DB, DEFM, DEFC, DC, DEFZ
 */
int op_db(BYTE op_code, BYTE dummy)
{
	register char *p, *p1, c;
	register int i;
	int sf;

	UNUSED(dummy);

	i = 0;
	p = operand;
	while (p != NULL) {
		p1 = next_arg(p, &sf);
		if (sf < 0) {		/* a non-terminated string */
			asmerr(E_MISDEL);
			goto delim_error;
		} else if (sf > 0) {	/* a valid string */
			c = *p++;
			while (*p != c || *++p == c) { /* double delim? */
				ops[i] = *p++;
				if (++i >= OPCARRAY)
				    fatal(F_INTERN, "op-code buffer overflow");
			}
		} else {		/* an expression */
			if (*p != '\0') {
				if (pass == 2)
					ops[i] = chk_byte(eval(p));
				if (++i >= OPCARRAY)
				    fatal(F_INTERN, "op-code buffer overflow");
			}
		}
		p = p1;
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
int op_dw(BYTE dummy1, BYTE dummy2)
{
	register char *p, *p1;
	register int i;
	register WORD n;

	UNUSED(dummy1);
	UNUSED(dummy2);

	i = 0;
	p = operand;
	while (p != NULL) {
		p1 = next_arg(p, NULL);
		if (*p != '\0') {
			if (pass == 2) {
				n = eval(p);
				ops[i] = n & 0xff;
				ops[i + 1] = n >> 8;
			}
			i += 2;
			if (i >= OPCARRAY)
				fatal(F_INTERN, "op-code buffer overflow");
		}
		p = p1;
	}
	return(i);
}

/*
 *	EJECT, PAGE, LIST, .LIST, NOLIST, .XLIST, .PRINTX, PRINT, INCLUDE,
 *	MACLIB, TITLE, .XALL, .LALL, .SALL, .SFCOND, .LFCOND
 */
int op_misc(BYTE op_code, BYTE dummy)
{
	register char *p, *d, c;
	register BYTE n;
	static char fn[LENFN];
	static int incnest;
	static struct inc incl[INCNEST];
	static int page_done;

	UNUSED(dummy);

	a_mode = A_NONE;
	switch(op_code) {
	case 1:				/* EJECT and PAGE */
		if (*operand != '\0') {
			if ((pass == 1 && !page_done) || pass == 2) {
				n = chk_byte(eval(operand));
				if (n != 0 && (n < 6 || n > 144))
					asmerr(E_VALOUT);
				else
					ppl = n;
				page_done = 1;
			}
		} else if (pass == 2)
			p_line = ppl - 1;
		break;
	case 2:				/* LIST and .LIST */
		if (pass == 2)
			list_flag = 1;
		break;
	case 3:				/* NOLIST and .XLIST */
		if (pass == 2)
			list_flag = 0;
		break;
	case 4:				/* .PRINTX */
		p = operand;
		c = *p++;
		putchar(c);
		while (*p != '\0') {
			putchar(*p);
			if (*p++ == c)
				break;
		}
		putchar('\n');
		break;
	case 5:				/* PRINT */
		p = operand;
		c = *p++;
		if (c == STRDEL || c == STRDEL2) {
			while (1) {
				if (*p == '\0') {
					putchar('\n');
					asmerr(E_MISDEL);
					return(0);
				}
				/* check for double delim */
				if (*p == c && *++p != c)
					break;
				putchar(*p++);
			}
		} else
			fputs(operand, stdout);
		putchar('\n');
		break;
	case 6:				/* INCLUDE and MACLIB */
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
		if (ver_flag)
			printf("   Include %s\n", fn);
		process_file(fn);
		incnest--;
		c_line = incl[incnest].inc_line;
		srcfn = incl[incnest].inc_fn;
		srcfp = incl[incnest].inc_fp;
		if (ver_flag)
			printf("   Resume  %s\n", srcfn);
		if (list_flag && pass == 2) {
			lst_header();
			lst_attl();
		}
		break;
	case 7:				/* TITLE */
		if (pass == 2) {
			p = operand;
			d = title;
			c = *p;
			if (c == STRDEL || c == STRDEL2) {
				p++;
				while (1) {
					if (*p == '\0') {
						asmerr(E_MISDEL);
						break;
					}
					/* check for double delim */
					if (*p == c && *++p != c)
						break;
					*d++ = *p++;
				}
			} else
				while (*p != '\0' && *p != COMMENT)
					*d++ = *p++;
			*d = '\0';
		}
		break;
	case 8:				/* .XALL */
		if (pass == 2)
			mac_list_flag = M_OPS;
		break;
	case 9:				/* .LALL */
		if (pass == 2)
			mac_list_flag = M_ALL;
		break;
	case 10:			/* .SALL */
		if (pass == 2)
			mac_list_flag = M_NONE;
		break;
	case 11:			/* .SFCOND */
		if (pass == 2)
			nofalselist = 1;
		break;
	case 12:			/* .LFCOND */
		if (pass == 2)
			nofalselist = 0;
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_misc");
		break;
	}
	return(0);
}

/*
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, COND, IF, IFT, IFE, IFF, IF1, IF2
 */
int op_cond(BYTE op_code, BYTE dummy)
{
	register char *p;

	UNUSED(dummy);

	a_mode = A_NONE;
	if (op_code < 90) {
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			return(0);
		}
		condnest[iflevel++] = gencode;
		if (gencode < 0)
			return(0);
		switch(op_code) {
		case 1:			/* IFDEF */
		case 2:			/* IFNDEF */
			gencode = (get_sym(operand) != NULL) ? pass : -pass;
			break;
		case 3:			/* IFEQ */
		case 4:			/* IFNEQ */
			p = next_arg(operand, NULL);
			if (p == NULL) {
				asmerr(E_MISOPE);
				return(0);
			}
			gencode = (eval(operand) == eval(p)) ? pass : -pass;
			break;
		case 5:			/* COND, IF, and IFT */
		case 6:			/* IFE and IFF */
			gencode = (eval(operand) != 0) ? pass : -pass;
			break;
		case 7:			/* IF1 */
			gencode = (pass == 1) ? 2 : -2;
			break;
		case 8:			/* IF2 */
			gencode = (pass == 2) ? -1 : 1;
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
		if ((op_code & 1) == 0)		/* negate for inverse IF */
			gencode = -gencode;
	} else {
		if (iflevel == 0) {
			asmerr(E_MISIFF);
			return(0);
		}
		switch(op_code) {
		case 98:		/* ELSE */
			if (condnest[iflevel - 1] > 0)
				gencode = -gencode;
			break;
		case 99:		/* ENDIF and ENDC */
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
int op_glob(BYTE op_code, BYTE dummy)
{
	UNUSED(dummy);

	a_mode = A_NONE;
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
int op_end(BYTE dummy1, BYTE dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 2 && *operand != '\0')
		start_addr = eval(operand);
	return(0);
}
