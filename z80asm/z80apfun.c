/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	processing of all PSEUDO ops except macro related
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "z80a.h"
#include "z80aglb.h"
#include "z80amain.h"
#include "z80anum.h"
#include "z80aopc.h"
#include "z80aout.h"
#include "z80atab.h"
#include "z80apfun.h"

/*
 *	.Z80 and .8080
 */
WORD op_instrset(BYTE op_code, BYTE dummy)
{
	UNUSED(dummy);

	a_mode = A_NONE;
	switch (op_code) {
	case 1:				/* .Z80 */
		instrset(INSTR_Z80);
		break;
	case 2:				/* .8080 */
		instrset(INSTR_8080);
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_instrset");
		break;
	}
	return 0;
}

/*
 *	ORG, .PHASE, .DEPHASE
 */
WORD op_org(BYTE op_code, BYTE dummy)
{
	register WORD n;

	UNUSED(dummy);

	a_mode = A_NONE;
	switch (op_code) {
	case 1:				/* ORG */
		if (phs_flag) {
			asmerr(E_ORGPHS);
			return 0;
		}
		n = eval(operand);
		if (pass == 1) {	/* PASS 1 */
			if (!load_flag) {
				/* first ORG sets load address,
				   without any ORG it defaults to 0 */
				load_addr = n;
				load_flag = TRUE;
			}
		} else			/* PASS 2 */
			obj_org(n);
		rpc = pc = n;
		break;
	case 2:				/* .PHASE */
		if (phs_flag)
			asmerr(E_PHSNST);
		else {
			phs_flag = TRUE;
			pc = eval(operand);
		}
		break;
	case 3:				/* .DEPHASE */
		if (!phs_flag)
			asmerr(E_MISPHS);
		else {
			phs_flag = FALSE;
			pc = rpc;
		}
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_org");
		break;
	}
	return 0;
}

/*
 *	.RADIX
 */
WORD op_radix(BYTE dummy1, BYTE dummy2)
{
	BYTE r;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	radix = 10;
	r = chk_byte(eval(operand));
	if (r < 2 || r > 16)
		asmerr(E_VALOUT);
	else
		radix = r;
	return 0;
}

/*
 *	EQU
 */
WORD op_equ(BYTE dummy1, BYTE dummy2)
{
	register struct sym *sp;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_EQU;
	a_addr = eval(operand);
	if ((sp = look_sym(label)) == NULL)
		new_sym(label)->sym_val = a_addr;
	else if (sp->sym_val != a_addr)
		asmerr(E_MULSYM);
	return 0;
}

/*
 *	DEFL, ASET, and SET (in 8080 mode)
 */
WORD op_dl(BYTE dummy1, BYTE dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_SET;
	a_addr = eval(operand);
	put_sym(label, a_addr);
	return 0;
}

/*
 *	DEFS and DS
 */
WORD op_ds(BYTE dummy1, BYTE dummy2)
{
	register char *p;
	register WORD count;
	register WORD value;

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
	return count;
}

/*
 *	DEFB, DB, DEFM, DEFC, DC, DEFT, DEFZ
 */
WORD op_db(BYTE op_code, BYTE dummy)
{
	register char *p;
	register int i;
	register char c;
	char *p1;
	int sf;

	UNUSED(dummy);

	i = (op_code == 3 ? 1 : 0);	/* DEFT length byte */
	p = operand;
	while (p != NULL) {
		p1 = next_arg(p, &sf);
		if (sf < 0) {		/* a non-terminated string */
			asmerr(E_MISDEL);
			return i;
		} else if (sf > 0) {	/* a valid string */
			c = *p++;
			while (*p != c || *++p == c) /* double delim? */
				ops[i++] = *p++;
		} else if (*p != '\0') { /* an expression */
			if (pass == 2)
				ops[i] = chk_byte(eval(p));
			i++;
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
	case 3:				/* DEFT */
		ops[0] = i - 1;
		break;
	case 4:				/* DEFZ */
		ops[i++] = '\0';
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_db");
		break;
	}
	return i;
}

/*
 *	DEFW and DW
 */
WORD op_dw(BYTE dummy1, BYTE dummy2)
{
	register char *p;
	register int i;
	register WORD n;
	char *p1;

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
		}
		p = p1;
	}
	return i;
}

/*
 *	EJECT, PAGE, LIST, .LIST, NOLIST, .XLIST, .PRINTX, PRINT, INCLUDE,
 *	MACLIB, TITLE, .XALL, .LALL, .SALL, .SFCOND, .LFCOND
 */
WORD op_misc(BYTE op_code, BYTE dummy)
{
	register char *p, *d;
	register char c;
	BYTE n;
	unsigned long inc_line;
	char *inc_fn, *fn;
	FILE *inc_fp;
	static int incnest;
	static int page_done;

	UNUSED(dummy);

	a_mode = A_NONE;
	switch (op_code) {
	case 1:				/* EJECT and PAGE */
		if (operand[0] != '\0') {
			if ((pass == 1 && !page_done) || pass == 2) {
				n = chk_byte(eval(operand));
				if (n != 0 && (n < 6 || n > 144))
					asmerr(E_VALOUT);
				else
					ppl = n;
				page_done = TRUE;
			}
		} else if (pass == 2)
			p_line = ppl - 1;
		break;
	case 2:				/* LIST and .LIST */
		if (pass == 2)
			list_flag = TRUE;
		break;
	case 3:				/* NOLIST and .XLIST */
		if (pass == 2)
			list_flag = FALSE;
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
			while (TRUE) {
				if (*p == '\0') {
					putchar('\n');
					asmerr(E_MISDEL);
					return 0;
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
			asmerr(E_INCNST);
			break;
		}
		inc_line = c_line;
		inc_fn = srcfn;
		inc_fp = srcfp;
		incnest++;
		p = operand;
		while (!IS_SPC(*p) && *p != COMMENT && *p != '\0')
			p++;
		*p = '\0';
		fn = strsave(operand);
		if (ver_flag)
			printf("   Include %s\n", fn);
		process_file(fn);
		free(fn);
		incnest--;
		c_line = inc_line;
		srcfn = inc_fn;
		srcfp = inc_fp;
		if (ver_flag)
			printf("   Resume  %s\n", srcfn);
		break;
	case 7:				/* TITLE */
		if (pass == 2) {
			p = operand;
			d = title;
			c = *p;
			if (c == STRDEL || c == STRDEL2) {
				p++;
				while (TRUE) {
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
			nofalselist = TRUE;
		break;
	case 12:			/* .LFCOND */
		if (pass == 2)
			nofalselist = FALSE;
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_misc");
		break;
	}
	return 0;
}

/*
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, COND, IF, IFT, IFE, IFF, IF1, IF2
 */
WORD op_cond(BYTE op_code, BYTE dummy)
{
	register char *p;
	register int err = FALSE;

	UNUSED(dummy);

	a_mode = A_NONE;
	if (op_code < 90) {
		if (iflevel == INT_MAX) {
			asmerr(E_IFNEST);
			return 0;
		}
		iflevel++;
		if (!gencode)
			return 0;
		switch (op_code) {
		case 1:			/* IFDEF */
		case 2:			/* IFNDEF */
			if (get_sym(operand) == NULL)
				gencode = FALSE;
			break;
		case 3:			/* IFEQ */
		case 4:			/* IFNEQ */
			if ((p = next_arg(operand, NULL)) != NULL) {
				if (eval(operand) != eval(p))
					gencode = FALSE;
			} else {
				asmerr(E_MISOPE);
				err = TRUE;
			}
			break;
		case 5:			/* COND, IF, and IFT */
		case 6:			/* IFE and IFF */
			if (eval(operand) == 0)
				gencode = FALSE;
			break;
		case 7:			/* IF1 */
		case 8:			/* IF2 */
			if (pass == 2)
				gencode = FALSE;
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
		if (!err) {
			if ((op_code & 1) == 0) /* negate for inverse IF */
				gencode = !gencode;
		}
		act_iflevel = iflevel;
	} else {
		if (iflevel == 0) {
			asmerr(E_MISIFF);
			return 0;
		}
		switch (op_code) {
		case 98:		/* ELSE */
			if (iflevel == act_iflevel) {
				if (iflevel == act_elselevel)
					asmerr(E_MISEIF);
				else
					act_elselevel = iflevel;
				gencode = !gencode;
			}
			break;
		case 99:		/* ENDIF and ENDC */
			if (iflevel == act_iflevel) {
				gencode = TRUE;
				act_elselevel = 0;
				act_iflevel--;
			}
			iflevel--;
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
	}
	return 0;
}

/*
 *	EXTRN, EXTERNAL, EXT and PUBLIC, ENT, ENTRY, GLOBAL, and ABS, ASEG
 */
WORD op_glob(BYTE op_code, BYTE dummy)
{
	UNUSED(dummy);

	a_mode = A_NONE;
	switch (op_code) {
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
	return 0;
}

/*
 *	END
 */
WORD op_end(BYTE dummy1, BYTE dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	if (pass == 2 && operand[0] != '\0')
		start_addr = eval(operand);
	return 0;
}
