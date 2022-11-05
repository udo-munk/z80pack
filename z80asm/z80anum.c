/*
 *	Z80 - Macro - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	21-OCT-2006 changed to ANSI C for modern POSIX OS's
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

int expr(WORD *);

/* z80aout.c */
extern void asmerr(int);

/* z80atab.c */
extern struct sym *get_sym(char *);

/*
 *	definition of token types
 */
#define T_EMPTY		0	/* nothing */
#define T_VAL		1	/* number, string, $, or symbol */
#define T_UNDSYM	2	/* undefined symbol */
#define T_SUB		3	/* - */
#define T_ADD		4	/* + */
#define T_NOT		5	/* NOT or ~ */
#define T_HIGH		6	/* HIGH */
#define T_LOW		7	/* LOW */
#define T_NUL		8	/* NUL */
#define T_TYPE		9	/* TYPE */
#define T_MUL		10	/* * */
#define T_DIV		11	/* / */
#define T_MOD		12	/* MOD or % */
#define T_SHR		13	/* SHR or >> */
#define T_SHL		14	/* SHL or << */
#define T_EQ		15	/* EQ, = or == */
#define T_NE		16	/* NE, <> or != */
#define T_LT		17	/* LT or < */
#define T_LE		18	/* LE or <= */
#define T_GT		19	/* GT or > */
#define T_GE		20	/* GE or >= */
#define T_AND		21	/* AND or & */
#define T_XOR		22	/* XOR or ^ */
#define T_OR		23	/* OR or | */
#define T_LPAREN	24	/* ( */
#define T_RPAREN	25	/* ) */

/*
 *	structure operator table
 */
struct opr {
	const char *opr_name;	/* operator name */
	BYTE opr_type;		/* operator token type */
};

/*
 *	table with operators
 *	must be sorted in ascending order!
 */
static struct opr oprtab[] = {
	{ "AND",	T_AND		},
	{ "EQ",		T_EQ		},
	{ "GE",		T_GE		},
	{ "GT",		T_GT		},
	{ "HIGH",	T_HIGH		},
	{ "LE",		T_LE		},
	{ "LOW",	T_LOW		},
	{ "LT",		T_LT		},
	{ "MOD",	T_MOD		},
	{ "NE",		T_NE		},
	{ "NOT",	T_NOT		},
	{ "NUL",	T_NUL		},
	{ "OR",		T_OR		},
	{ "SHL",	T_SHL		},
	{ "SHR",	T_SHR		},
	{ "TYPE",	T_TYPE		},
	{ "XOR",	T_XOR		}
};
static unsigned no_operators = sizeof(oprtab) / sizeof(struct opr);

static BYTE tok_type;	/* token type and flags */
static WORD tok_val;	/* token value for T_VAL type */
static char tok_sym[MAXLINE]; /* last symbol scanned (T_VAL) */
static char *scan_pos;	/* current scanning position */

void init_ctype(void)
{
	register BYTE i;

	for (i = '0'; i <= '9'; i++)
		ctype[i] = C_SYM | C_DIG | C_XDIG;
	for (i = 'A'; i <= 'F'; i++)
		ctype[i] = C_FSYM | C_SYM | C_XDIG;
	for (i = 'G'; i <= 'Z'; i++)
		ctype[i] = C_FSYM | C_SYM;
	for (i = 'a'; i <= 'f'; i++)
		ctype[i] = C_FSYM | C_SYM | C_LOW | C_XDIG;
	for (i = 'g'; i <= 'z'; i++)
		ctype[i] = C_FSYM | C_SYM | C_LOW;
	ctype[(BYTE) '$'] = C_FSYM | C_SYM;
	ctype[(BYTE) '.'] = C_FSYM | C_SYM;
	ctype[(BYTE) '?'] = C_FSYM | C_SYM;
	ctype[(BYTE) '@'] = C_FSYM | C_SYM;
	ctype[(BYTE) '_'] = C_FSYM | C_SYM;
	ctype[(BYTE) ' '] = C_SPC;
	ctype[(BYTE) '\f'] = C_SPC;
	ctype[(BYTE) '\n'] = C_SPC;
	ctype[(BYTE) '\r'] = C_SPC;
	ctype[(BYTE) '\t'] = C_SPC;
	ctype[(BYTE) '\v'] = C_SPC;
}

/*
 *	binary search in sorted table oprtab
 *
 *	Input: pointer to string with operator
 *
 *	Output: symbol for operator, T_UNDSYM if operator not found
 */
BYTE search_opr(char *s)
{
	register int cond;
	register struct opr *low, *high, *mid;

	low = &oprtab[0];
	high = &oprtab[no_operators - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(s, mid->opr_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(mid->opr_type);
	}
	return(T_UNDSYM);
}

/*
 *	get next token
 *
 *	updates tok_type, tok_val, tok_sym and scan_pos.
 *	returns E_NOERR on success
 */
int get_token(void)
{
	register char *p1, *p2, *s;
	register WORD n, m, base;
	register struct sym *sp;

	s = scan_pos;
	tok_sym[0] = '\0';
	tok_val = 0;
	while (IS_SPC(*s))				/* skip white space */
		s++;
	if (*s == '\0') {				/* nothing there? */
		tok_type = T_EMPTY;
		goto done;
	}
	if (*s == 'X' && *(s + 1) == STRDEL) {		/* X'h' hex constant */
		s += 2;
		n = 0;
		while (IS_XDIG(*s)) {
			n *= 16;
			n += TO_UPP(*s) - ((*s <= '9') ? '0' : '7');
			s++;
		}
		if (*s != STRDEL)			/* missing final ' */
			return(E_MISDEL);
		else {
			s++;
			tok_type = T_VAL;
			tok_val = n;
			goto done;
		}
	}
	p1 = p2 = tok_sym;				/* gather symbol */
	while (IS_SYM(*s))
		*p2++ = *s++;
	*p2 = '\0';
	if (p1 != p2) {					/* a number/symbol */
		if (IS_DIG(*p1)) {			/* a number */
			p2--;
			if (radix < 12 && *p2 == 'B')
				base = 2;
			else if (*p2 == 'O' || *p2 == 'Q')
				base = 8;
			else if (radix < 14 && *p2 == 'D')
				base = 10;
			else if (*p2 == 'H')
				base = 16;
			else {
				base = radix;
				p2++;
			}
			n = 0;
			while (p1 < p2) {
				if (*p1 == '$') {	/* digit grouping */
					p1++;
					continue;
				}
				if (IS_XDIG(*p1)) {
					m = *p1 - ((*p1 <= '9') ? '0' : '7');
					if (m < base) {
						n *= base;
						n += m;
						p1++;
					} else		/* digit not of base */
						return(E_INVEXP);
				} else			/* not a digit */
					return(E_INVEXP);
			}
			tok_type = T_VAL;
			tok_val = n;
			goto done;
		}
		if (*p1 == '$' && *(p1 + 1) == '\0') {	/* location counter */
			tok_type = T_VAL;
			tok_val = pc;
		} else {				/* symbol / word opr */
			if (p2 - p1 > symlen)		/* trim for lookup */
				*(p1 + symlen) = '\0';
			if ((sp = get_sym(tok_sym)) != NULL) {	/* a symbol */
				tok_type = T_VAL;
				tok_val = sp->sym_val;
			} else				/* look for word opr */
				tok_type = search_opr(p1);
		}
		goto done;
	}
	switch (*s) {
	case STRDEL:					/* char constant */
	case STRDEL2:
		p1 = s;
		n = m = 0;
		while (*++s != '\0') {
			if (*s == *p1 && *++s != *p1) {	/* double delim? */
				tok_type = T_VAL;
				tok_val = n;
				goto done;
			}
			if (m++ == 2)
				return(E_VALOUT);
			n <<= 8;
			n |= (unsigned) *s;
		}
		return(E_MISDEL);
	case '!':
		if (*(s + 1) == '=') {
			s++;
			tok_type = T_NE;
		} else
			return(E_INVEXP);
		break;
	case '%':
		tok_type = T_MOD;
		break;
	case '&':
		tok_type = T_AND;
		break;
	case '(':
		tok_type = T_LPAREN;
		break;
	case ')':
		tok_type = T_RPAREN;
		break;
	case '*':
		tok_type = T_MUL;
		break;
	case '+':
		tok_type = T_ADD;
		break;
	case '-':
		tok_type = T_SUB;
		break;
	case '/':
		tok_type = T_DIV;
		break;
	case '<':
		if (*(s + 1) == '<') {
			s++;
			tok_type = T_SHL;
		} else if (*(s + 1) == '=') {
			s++;
			tok_type = T_LE;
		} else if (*(s + 1) == '>') {
			s++;
			tok_type = T_NE;
		} else
			tok_type = T_LT;
		break;
	case '=':
		if (*(s + 1) == '=')
			s++;
		tok_type = T_EQ;
		break;
	case '>':
		if (*(s + 1) == '=') {
			s++;
			tok_type = T_GE;
		} else if (*(s + 1) == '>') {
			s++;
			tok_type = T_SHR;
		} else
			tok_type = T_GT;
		break;
	case '^':
		tok_type = T_XOR;
		break;
	case '|':
		tok_type = T_OR;
		break;
	case '~':
		tok_type = T_NOT;
		break;
	default:
		return(E_INVEXP);
	}
	s++;
done:
	scan_pos = s;
	return(E_NOERR);
}

/*
 *	recursive descent parser & evaluator
 *
 *	inspired by the previous expression parser by Didier Derny.
 */

int factor(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	register char *s;
	WORD value;

	switch (tok_type) {
	case T_VAL:
		value = tok_val;
		if ((err = get_token()) != E_NOERR)
			return(err);
		*resultp = value;
		return(E_NOERR);
	case T_UNDSYM:
		if ((err = get_token()) != E_NOERR)
			return(err);
		return(E_UNDSYM);
	case T_NUL:
		s = scan_pos;
		while (IS_SPC(*s))
			s++;
		if (strcmp(s, "''") == 0 || strcmp(s, "\"\"") == 0)
			*resultp = -1;
		else
			*resultp = (*s == '\0') ? -1 : 0;
		/* short circuit to end of expression */
		while (*s != '\0')
			s++;
		scan_pos = s;
		tok_type = T_EMPTY;
		return(E_NOERR);
	case T_TYPE:
		if ((erru = get_token()) != E_NOERR && erru != E_UNDSYM)
			return(erru);
		*resultp = ((erru || factor(&value)) ? 0x00 : 0x20);
		return(E_NOERR);
	case T_ADD:
	case T_SUB:
	case T_NOT:
	case T_HIGH:
	case T_LOW:
		opr_type = tok_type;
		if ((err = get_token()) != E_NOERR
		    || (err = factor(&value)) != E_NOERR)
			return(err);
		switch (opr_type) {
		case T_ADD:
			*resultp = value;
			break;
		case T_SUB:
			*resultp = -value;
			break;
		case T_NOT:
			*resultp = ~value;
			break;
		case T_HIGH:
			*resultp = value >> 8;
			break;
		case T_LOW:
			*resultp = value & 0xff;
			break;
		default:
			break;
		}
		return(E_NOERR);
	case T_LPAREN:
		if ((err = get_token()) != E_NOERR)
			return(err);
		if ((erru = expr(&value)) != E_NOERR && erru != E_UNDSYM)
			return(erru);
		if (tok_type == T_RPAREN) {
			if ((err = get_token()) != E_NOERR)
				return(err);
			else if (erru != E_NOERR) /* E_UNDSYM */
				return(E_UNDSYM);
			else {
				*resultp = value;
				return(E_NOERR);
 			}
		} else
			return(E_MISPAR);
	default:
		return(E_INVEXP);
	}
}

int mul_term(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = factor(resultp)) != E_NOERR && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_MUL || tok_type == T_DIV || tok_type == T_MOD
				 || tok_type == T_SHR || tok_type == T_SHL) {
		opr_type = tok_type;
		if ((err = get_token()) != E_NOERR)
			return(err);
		if ((err = factor(&value)) != E_NOERR && err != E_UNDSYM)
			return(err);
		if (erru != E_NOERR || err != E_NOERR) { /* E_UNDSYM */
			erru = E_UNDSYM;
			continue;
		}
		switch (opr_type) {
		case T_MUL:
			*resultp *= value;
			break;
		case T_DIV:
			if (value == 0)		/* don't crash on div by 0 */
				return(E_DIVBY0);
			*resultp /= value;
			break;
		case T_MOD:
			if (value == 0)		/* don't crash on mod by 0 */
				return(E_DIVBY0);
			*resultp %= value;
			break;
		case T_SHR:
			*resultp >>= value;
			break;
		case T_SHL:
			*resultp <<= value;
			break;
		default:
			break;
		}
	}
	return(erru ? E_UNDSYM : E_NOERR);
}

int add_term(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = mul_term(resultp)) != E_NOERR && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_ADD || tok_type == T_SUB) {
		opr_type = tok_type;
		if ((err = get_token()) != E_NOERR)
			return(err);
		if ((err = mul_term(&value)) != E_NOERR && err != E_UNDSYM)
			return(err);
		if (erru != E_NOERR || err != E_NOERR) { /* E_UNDSYM */
			erru = E_UNDSYM;
			continue;
		}
		switch (opr_type) {
		case T_ADD:
			*resultp += value;
			break;
		case T_SUB:
			*resultp -= value;
			break;
		default:
			break;
		}
	}
	return(erru ? E_UNDSYM : E_NOERR);
}

int cmp_term(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = add_term(resultp)) != E_NOERR && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_EQ || tok_type == T_NE
				|| tok_type == T_LT || tok_type == T_LE
				|| tok_type == T_GT || tok_type == T_GE) {
		opr_type = tok_type;
		if ((err = get_token()) != E_NOERR)
			return(err);
		if ((err = add_term(&value)) != E_NOERR && err != E_UNDSYM)
			return(err);
		if (erru != E_NOERR || err != E_NOERR) { /* E_UNDSYM */
			erru = E_UNDSYM;
			continue;
		}
		switch (opr_type) {
		case T_EQ:
			*resultp = (*resultp == value) ? -1 : 0;
			break;
		case T_NE:
			*resultp = (*resultp != value) ? -1 : 0;
			break;
		case T_LT:
			*resultp = (*resultp < value) ? -1 : 0;
			break;
		case T_LE:
			*resultp = (*resultp <= value) ? -1 : 0;
			break;
		case T_GT:
			*resultp = (*resultp > value) ? -1 : 0;
			break;
		case T_GE:
			*resultp = (*resultp >= value) ? -1 : 0;
			break;
		default:
			break;
		}
	}
	return(erru ? E_UNDSYM : E_NOERR);
}

int expr(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = cmp_term(resultp)) != E_NOERR && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_AND || tok_type == T_XOR || tok_type == T_OR) {
		opr_type = tok_type;
		if ((err = get_token()) != E_NOERR)
			return(err);
		if ((err = cmp_term(&value)) != E_NOERR && err != E_UNDSYM)
			return(err);
		if (erru != E_NOERR || err != E_NOERR) { /* E_UNDSYM */
			erru = E_UNDSYM;
			continue;
		}
		switch (opr_type) {
		case T_AND:
			*resultp &= value;
			break;
		case T_XOR:
			*resultp ^= value;
			break;
		case T_OR:
			*resultp |= value;
			break;
		default:
			break;
		}
	}
	return(erru ? E_UNDSYM : E_NOERR);
}

WORD eval(char *s)
{
	register int err;
	WORD result;

	if (s == NULL || *s == '\0') {
		asmerr(E_MISOPE);
		return(0);
	}
	result = 0;
	scan_pos = s;
	if ((err = get_token()) != E_NOERR
	    || (err = expr(&result)) != E_NOERR) {
		asmerr(err);
		return(0);
	} else if (tok_type != T_EMPTY) {	/* leftovers, error out */
		asmerr(E_INVEXP);
		return(0);
	} else
		return(result);
}

/*
 *	check value for range -256 <= value <= 255
 *	Output: value if in range, otherwise 0 and error message
 */
BYTE chk_byte(WORD n)
{
	if (n >= (WORD) -256 || n <= 255)
		return(n);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}

/*
 *	check value for range -128 <= value <= 127
 *	Output: value if in range, otherwise 0 and error message
 */
BYTE chk_sbyte(WORD n)
{
	if (n >= (WORD) -128 || n <= 127)
		return(n);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}
