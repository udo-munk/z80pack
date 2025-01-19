/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	processing of all PSEUDO ops except include and macro related
 */

#include <stddef.h>
#include <stdio.h>
#include <limits.h>

#include "z80asm.h"
#include "z80alst.h"
#include "z80amfun.h"
#include "z80anum.h"
#include "z80aobj.h"
#include "z80aopc.h"
#include "z80atab.h"
#include "z80apfun.h"

static int phase_flag;		/* inside a .PHASE section flag */

static int false_sect_flag;	/* in false conditional section flag */
static int iflevel;		/* IF nesting level */
static int act_iflevel;		/* active IF nesting level */
static int act_elselevel;	/* active ELSE nesting level */

/*
 *	return TRUE if inside a .PHASE section
 */
int in_phase_section(void)
{
	return phase_flag;
}

/*
 *	returns TRUE if in TRUE section (i.e. generate code)
 */
int in_true_section(void)
{
	return !false_sect_flag;
}

/*
 *	returns TRUE if in IF/ELSE/ENDIF section
 */
int in_cond_section(void)
{
	return iflevel > 0;
}

/*
 *	save IF/ELSE/ENDIF state
 */
void save_cond_state(int *p)
{
	p[0] = iflevel;
	p[1] = act_iflevel;
	p[2] = act_elselevel;
}

/*
 *	restore IF/ELSE/ENDIF state
 */
void restore_cond_state(int *p)
{
	iflevel = p[0];
	act_iflevel = p[1];
	act_elselevel = p[2];
	false_sect_flag = FALSE;
}

/*
 *	.Z80 and .8080
 */
WORD op_instrset(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
{
	UNUSED(pass);
	UNUSED(dummy);
	UNUSED(operand);
	UNUSED(ops);

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
 *	ORG, .PHASE, and .DEPHASE
 */
WORD op_org(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
{
	register WORD n;

	UNUSED(dummy);
	UNUSED(ops);

	switch (op_code) {
	case 1:				/* ORG */
		if (phase_flag) {
			asmerr(E_ORGPHS);
			return 0;
		}
		n = eval(operand);
		if (pass == 1)		/* PASS 1 */
			obj_load_addr(n);
		else			/* PASS 2 */
			obj_org(n);
		set_pc(PC_ORG, n);
		break;
	case 2:				/* .PHASE */
		if (phase_flag)
			asmerr(E_PHSNST);
		else {
			phase_flag = TRUE;
			set_pc(PC_PHASE, eval(operand));
		}
		break;
	case 3:				/* .DEPHASE */
		if (!phase_flag)
			asmerr(E_MISPHS);
		else {
			phase_flag = FALSE;
			set_pc(PC_DEPHASE, 0);
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
WORD op_radix(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
{
	BYTE r;

	UNUSED(pass);
	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(ops);

	set_radix(10);
	r = chk_byte(eval(operand));
	if (r < 2 || r > 16)
		asmerr(E_VALOUT);
	else
		set_radix(r);
	return 0;
}

/*
 *	EQU
 */
WORD op_equ(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
{
	register sym_t *sp;
	const char *label;
	WORD addr;

	UNUSED(pass);
	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(ops);

	label = get_label();
	addr = eval(operand);
	if ((sp = look_sym(label)) == NULL)
		new_sym(label, addr);
	else if (sp->sym_val != addr)
		asmerr(E_MULSYM);
	return 0;
}

/*
 *	DEFL, ASET, and SET (in 8080 mode)
 */
WORD op_dl(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
{
	UNUSED(pass);
	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(ops);

	put_sym(get_label(), eval(operand));
	return 0;
}

/*
 *	DEFS and DS
 */
WORD op_ds(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
{
	register char *p;
	register WORD count;
	register WORD value;

	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(ops);

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
 *	DEFB, DB, DEFM, DEFC, DC, DEFT, and DEFZ
 */
WORD op_db(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
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
WORD op_dw(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
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
 *	EJECT, PAGE, LIST, .LIST, NOLIST, .XLIST, .PRINTX, PRINT, TITLE,
 *	.XALL, .LALL, .SALL, .SFCOND, and .LFCOND
 */
WORD op_misc(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
{
	register char *p, *q;
	register char c;
	BYTE n;
	static int page_done;

	UNUSED(dummy);
	UNUSED(ops);

	switch (op_code) {
	case 1:				/* EJECT and PAGE */
		if (operand[0] != '\0') {
			if ((pass == 1 && !page_done) || pass == 2) {
				n = chk_byte(eval(operand));
				if (n != 0 && (n < 6 || n > 144))
					asmerr(E_VALOUT);
				else
					lst_set_ppl(n);
				page_done = TRUE;
			}
		} else if (pass == 2)
			lst_eject(FALSE);
		break;
	case 2:				/* LIST and .LIST */
		if (pass == 2)
			set_list_active(TRUE);
		break;
	case 3:				/* NOLIST and .XLIST */
		if (pass == 2)
			set_list_active(FALSE);
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
	case 6:				/* TITLE */
		if (pass == 2) {
			p = operand;
			c = *p;
			if (c == STRDEL || c == STRDEL2) {
				q = p + 1;
				while (TRUE) {
					if (*q == '\0') {
						asmerr(E_MISDEL);
						break;
					}
					/* check for double delim */
					if (*q == c && *++q != c)
						break;
					*p++ = *q++;
				}
			} else
				while (*p != '\0' && *p != COMMENT)
					p++;
			*p = '\0';
			lst_set_title(operand);
		}
		break;
	case 7:				/* .XALL */
		if (pass == 2)
			set_mac_list_opt(M_OPS);
		break;
	case 8:				/* .LALL */
		if (pass == 2)
			set_mac_list_opt(M_ALL);
		break;
	case 9:				/* .SALL */
		if (pass == 2)
			set_mac_list_opt(M_NONE);
		break;
	case 10:			/* .SFCOND */
		if (pass == 2)
			set_nofalselist(TRUE);
		break;
	case 11:			/* .LFCOND */
		if (pass == 2)
			set_nofalselist(FALSE);
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_misc");
		break;
	}
	return 0;
}

/*
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, COND, IF, IFT, IFE, IFF, IF1, IF2,
 *	IFB, IFNB, IFIDN, and IFDIF
 */
WORD op_cond(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
{
	register char *p;
	register int err = FALSE;

	UNUSED(dummy);
	UNUSED(ops);

	if (op_code < 90) {
		if (iflevel == INT_MAX) {
			asmerr(E_IFNEST);
			return 0;
		}
		iflevel++;
		if (false_sect_flag)
			return 0;
		switch (op_code) {
		case 1:			/* IFDEF */
		case 2:			/* IFNDEF */
			if (get_sym(operand) == NULL)
				false_sect_flag = TRUE;
			break;
		case 3:			/* IFEQ */
		case 4:			/* IFNEQ */
			if ((p = next_arg(operand, NULL)) != NULL) {
				if (eval(operand) != eval(p))
					false_sect_flag = TRUE;
			} else {
				asmerr(E_MISOPE);
				err = TRUE;
			}
			break;
		case 5:			/* COND, IF, and IFT */
		case 6:			/* IFE and IFF */
			if (eval(operand) == 0)
				false_sect_flag = TRUE;
			break;
		case 7:			/* IF1 */
		case 8:			/* IF2 */
			if (pass == 2)
				false_sect_flag = TRUE;
			break;
		case 9:			/* IFB */
		case 10:		/* IFNB */
			err = mac_op_ifb(operand, &false_sect_flag);
			break;
		case 11:		/* IFIDN */
		case 12:		/* IFDIF */
			err = mac_op_ifidn(operand, &false_sect_flag);
			break;
		default:
			fatal(F_INTERN, "invalid opcode for function op_cond");
			break;
		}
		if (!err) {
			if ((op_code & 1) == 0) /* negate for inverse IF */
				false_sect_flag = !false_sect_flag;
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
				false_sect_flag = !false_sect_flag;
			}
			break;
		case 99:		/* ENDIF and ENDC */
			if (iflevel == act_iflevel) {
				false_sect_flag = FALSE;
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
 *	EXTRN, EXTERNAL, EXT, PUBLIC, ENT, ENTRY, GLOBAL, ABS, and ASEG
 */
WORD op_glob(int pass, BYTE op_code, BYTE dummy, char *operand, BYTE *ops)
{
	UNUSED(pass);
	UNUSED(dummy);
	UNUSED(operand);
	UNUSED(ops);

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
WORD op_end(int pass, BYTE dummy1, BYTE dummy2, char *operand, BYTE *ops)
{
	UNUSED(dummy1);
	UNUSED(dummy2);
	UNUSED(ops);

	if (pass == 2 && operand[0] != '\0')
		obj_start_addr(eval(operand));
	return 0;
}
