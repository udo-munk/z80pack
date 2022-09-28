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
 *	processing of all PSEUDO ops
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

extern void fatal(int, char *);
extern void p1_file(char *);
extern void p2_file(char *);
extern int eval(char *);
extern void asmerr(int);
extern void lst_header(void);
extern void lst_attl(void);
extern void lst_line(int, int);
extern void obj_fill(int);
extern struct sym *get_sym(char *);
extern int put_sym(char *, int);
extern void put_label(void);

#define UNUSED(x)	(void)(x)

/*
 *	.8080 and .Z80
 */
int op_opset(int new_opset, int dummy)
{
	UNUSED(dummy);

	opset = new_opset;
	return(0);
}

/*
 *	ORG
 */
int op_org(int dummy1, int dummy2)
{
	register int i;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	if (phs_flag) {
		asmerr(E_ORGPHS);
		return(0);
	}
	i = eval(operand);
	if (i < pc) {
		asmerr(E_MEMOVR);
		return(0);
	}
	if (pass == 1) {		/* PASS 1 */
		if (!prg_flag) {
			prg_addr = i;
			prg_flag++;
		}
	} else {			/* PASS 2 */
		if (++prg_flag > 2)
			obj_fill(i - pc);
		sd_flag = 2;
	}
	rpc = pc = i;
	return(0);
}

/*
 *	.PHASE
 */
int op_phase(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	if (pass == 1)
		if (*label)
			put_label();
	if (phs_flag)
		asmerr(E_PHSNEST);
	else {
		phs_flag = 1;
		pc = eval(operand);
		sd_flag = 2;
	}
	return(0);
}

/*
 *	.DEPHASE
 */
int op_dephase(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	if (!phs_flag)
		asmerr(E_MISPHS);
	else {
		phs_flag = 0;
		pc = rpc;
		sd_flag = 2;
	}
	return(0);
}

/*
 *	EQU
 */
int op_equ(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	if (pass == 1) {		/* PASS 1 */
		if (get_sym(label) == NULL) {
			sd_val = eval(operand);
			if (put_sym(label, sd_val))
				fatal(F_OUTMEM, "symbols");
		} else
			asmerr(E_MULSYM);
	} else {			/* PASS 2 */
		sd_flag = 1;
		sd_val = eval(operand);
	}
	return(0);
}

/*
 *	DEFL and ASET
 */
int op_dl(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	sd_flag = 1;
	sd_val = eval(operand);
	if (put_sym(label, sd_val))
		fatal(F_OUTMEM, "symbols");
	return(0);
}

/*
 *	DEFS and DS
 */
int op_ds(int dummy1, int dummy2)
{
	register int val;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	if (pass == 1)
		if (*label)
			put_label();
	sd_val = pc;
	sd_flag = 3;
	val = eval(operand);
	if ((pass == 2) && !dump_flag)
		obj_fill(val);
	pc += val;
	rpc += val;
	return(0);
}

/*
 *	DEFB and DB
 */
int op_db(int dummy1, int dummy2)
{
	register int i;
	register char *p;
	register char *s;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	i = 0;
	p = operand;
	if (pass == 1)
		if (*label)
			put_label();
	while (*p) {
		if (*p == STRSEP) {
			p++;
			while (*p != STRSEP) {
				if (*p == '\n' || *p == '\0') {
					asmerr(E_MISHYP);
					goto hyp_error;
				}
				ops[i++] = *p++;
				if (i >= OPCARRAY)
				    fatal(F_INTERN, "Op-code buffer overflow");
			}
			p++;
		} else {
			s = tmp;
			while (*p != ',' && *p != '\0')
				*s++ = *p++;
			*s = '\0';
			ops[i++] = eval(tmp);
			if (i >= OPCARRAY)
				fatal(F_INTERN, "Op-code buffer overflow");
		}
		if (*p == ',')
			p++;
	}
hyp_error:
	return(i);
}

/*
 *	DEFM, DEFC, DC, DEFZ
 */
int op_dm(int op_code, int dummy)
{
	register int i;
	register char *p;

	UNUSED(dummy);

	if (!gencode)
		return(0);
	i = 0;
	p = operand;
	if (pass == 1)
		if (*label)
			put_label();
	if (*p != STRSEP) {
		asmerr(E_MISHYP);
		return(0);
	}
	p++;
	while (*p != STRSEP) {
		if (*p == '\n' || *p == '\0') {
			asmerr(E_MISHYP);
			break;
		}
		ops[i++] = *p++;
		if (i >= OPCARRAY)
			fatal(F_INTERN, "Op-code buffer overflow");
	}
	switch (op_code) {
	case 1:				/* DEFM */
		break;
	case 2:				/* DEFC, DC */
		if (i)
			ops[i - 1] |= 0x80;
		break;
	case 3:				/* DEFZ */
		ops[i++] = '\0';
		if (i >= OPCARRAY)
			fatal(F_INTERN, "Op-code buffer overflow");
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_dm");
	}
	return(i);
}

/*
 *	DEFW and DW
 */
int op_dw(int dummy1, int dummy2)
{
	register int i, len, temp;
	register char *p;
	register char *s;

	UNUSED(dummy1);
	UNUSED(dummy2);

	if (!gencode)
		return(0);
	p = operand;
	i = len = 0;
	if (pass == 1)
		if (*label)
			put_label();
	while (*p) {
		s = tmp;
		while (*p != ',' && *p != '\0')
			*s++ = *p++;
		*s = '\0';
		if (pass == 2) {
			temp = eval(tmp);
			ops[i++] = temp & 0xff;
			ops[i++] = temp >> 8;
			if (i >= OPCARRAY)
				fatal(F_INTERN, "Op-code buffer overflow");
		}
		len += 2;
		if (*p == ',')
			p++;
	}
	return(len);
}

/*
 *	EJECT, LIST, NOLIST, PAGE, PRINT, TITLE, INCLUDE
 */
int op_misc(int op_code, int dummy)
{
	register char *p, *d;
	static char fn[LENFN];
	static int incnest;
	static struct inc incl[INCNEST];

	UNUSED(dummy);

	if (!gencode)
		return(0);
	sd_flag = 2;
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
			while (*p) {
				if (*p != STRSEP)
					putchar(*p++);
				else
					p++;
			}
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
		p = line;
		d = fn;
		while(isspace((int)*p))	/* ignore white space until INCLUDE */
			p++;
		while(!isspace((int)*p))/* ignore INCLUDE */
			p++;
		while(isspace((int)*p))	/* ignore white space until filename */
			p++;
		while(!isspace((int)*p) && *p != COMMENT) /* get filename */
			*d++ = *p++;
		*d = '\0';
		if (pass == 1) {	/* PASS 1 */
			if (ver_flag)
				printf("   Include %s\n", fn);
			p1_file(fn);
		} else {		/* PASS 2 */
			sd_flag = 2;
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
		sd_flag = 4;
		break;
	case 7:				/* TITLE */
		if (pass == 2) {
			p = line;
			d = title;
			while (isspace((int)*p)) /* ignore white space until TITLE */
				p++;
			while (!isspace((int)*p))/* ignore TITLE */
				p++;
			while (isspace((int)*p)) /* ignore white space until text */
				p++;
			if (*p == STRSEP)
				p++;
			while (*p != '\n' && *p != STRSEP && *p != COMMENT)
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
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, ELSE, ENDIF
 */
int op_cond(int op_code, int dummy)
{
	register char *p, *p1, *p2;
	static int condnest[IFNEST];

	UNUSED(dummy);

	switch(op_code) {
	case 1:				/* IFDEF */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		if (gencode)
			if (get_sym(operand) == NULL)
				gencode = 0;
		break;
	case 2:				/* IFNDEF */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		if (gencode)
			if (get_sym(operand) != NULL)
				gencode = 0;
		break;
	case 3:				/* IFEQ */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		p = operand;
		if (!*p || !(p1 = strchr(operand, ','))) {
			asmerr(E_MISOPE);
			break;
		}
		if (gencode) {
			p2 = tmp;
			while (*p != ',')
				*p2++ = *p++;
			*p2 = '\0';
			if (eval(tmp) != eval(++p1))
				gencode = 0;
		}
		break;
	case 4:				/* IFNEQ */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		p = operand;
		if (!*p || !(p1 = strchr(operand, ','))) {
			asmerr(E_MISOPE);
			break;
		}
		if (gencode) {
			p2 = tmp;
			while (*p != ',')
				*p2++ = *p++;
			*p2 = '\0';
			if (eval(tmp) == eval(++p1))
				gencode = 0;
		}
		break;
	case 98:			/* ELSE */
		if (!iflevel)
			asmerr(E_MISIFF);
		else
			if ((iflevel == 0) || (condnest[iflevel - 1] == 1))
				gencode = !gencode;
		break;
	case 99:			/* ENDIF */
		if (!iflevel)
			asmerr(E_MISIFF);
		else
			gencode = condnest[--iflevel];
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_cond");
		break;
	}
	sd_flag = 2;
	return(0);
}

/*
 *	EXTRN, EXTERNAL, EXT and PUBLIC, ENT, ENTRY, GLOBAL
 */
int op_glob(int op_code, int dummy)
{
	UNUSED(dummy);

	if (!gencode)
		return(0);
	sd_flag = 2;
	switch(op_code) {
	case 1:				/* EXTRN, EXTERNAL, EXT */
		break;
	case 2:				/* PUBLIC, ENT, ENTRY, GLOBAL */
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_glob");
		break;
	}
	return(0);
}
