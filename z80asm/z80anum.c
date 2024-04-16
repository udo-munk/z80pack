/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
 */

/*
 *	expression parser and evaluator module
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
static int no_operators = sizeof(oprtab) / sizeof(struct opr);

static BYTE tok_type;			/* token type and flags */
static WORD tok_val;			/* token value for T_VAL type */
static char tok_sym[MAXLINE + 1];	/* buffer for symbol/number */
static char *scan_pos;			/* current scanning position */

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
 *	do binary search for operator s in sorted table oprtab
 *	returns symbol for operator or T_UNDSYM if not found
 */
BYTE search_opr(char *s)
{
	register struct opr *low, *mid;
	register struct opr *high;
	int cond;

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
 *	updates tok_type, tok_val and scan_pos
 *	returns E_OK on success, otherwise a hard error code
 */
int get_token(void)
{
	register char *s, *p1;
	register char *p2;
	WORD n, m, base;
	struct sym *sp;

	s = scan_pos;
	tok_val = 0;
	while (IS_SPC(*s))				/* skip white space */
		s++;
	if (*s == '\0') {				/* nothing there? */
		tok_type = T_EMPTY;
		scan_pos = s;
		return(E_OK);
	}
	if (*s == 'X' && *(s + 1) == STRDEL) {		/* X'h' hex constant */
		s += 2;
		n = 0;
		while (IS_XDIG(*s)) {
			n *= 16;
			n += TO_UPP(*s) - ((*s <= '9') ? '0' : '7');
			s++;
		}
		if (*s == STRDEL) {
			tok_type = T_VAL;
			tok_val = n;
			scan_pos = s + 1;
			return(E_OK);
		} else					/* missing final ' */
			return(E_MISDEL);
	}
	p1 = p2 = tok_sym;				/* gather symbol */
	while (IS_SYM(*s))
		*p2++ = *s++;
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
			*p2 = '\0';
			n = 0;
			while (*p1 != '\0') {
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
			scan_pos = s;
			return(E_OK);
		}
		*p2 = '\0';
		if (*p1 == '$' && *(p1 + 1) == '\0') {	/* location counter */
			tok_type = T_VAL;
			tok_val = pc;
		} else {				/* symbol / word opr */
			if ((p2 - p1) > symlen)		/* trim for lookup */
				*(p1 + symlen) = '\0';
			if ((sp = get_sym(tok_sym)) != NULL) { /* a symbol */
				tok_type = T_VAL;
				tok_val = sp->sym_val;
			} else				/* look for word opr */
				tok_type = search_opr(p1);
		}
		scan_pos = s;
		return(E_OK);
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
				scan_pos = s;
				return(E_OK);
			}
			if (m++ == 2)
				return(E_VALOUT);
			n <<= 8;
			n |= (BYTE) *s;
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
	scan_pos = s + 1;
	return(E_OK);
}

/*
 *	recursive descent parser & evaluator
 *	error values > E_UNDSYM are hard errors that abort parsing
 *
 *	inspired by the previous expression parser by Didier Derny.
 */

int factor(WORD *resultp)
{
	register int err, erru;
	register char *s;
	BYTE opr_type;
	WORD value;

	switch (tok_type) {
	case T_VAL:
		value = tok_val;
		if ((err = get_token()) != E_OK)
			return(err);
		*resultp = value;
		return(E_OK);
	case T_UNDSYM:
		if ((err = get_token()) != E_OK)
			return(err);
		return(E_UNDSYM);
	case T_NUL:
		s = scan_pos;
		while (IS_SPC(*s))
			s++;
		/* if there is nothing or an empty string return true */
		if (*s == '\0' || (((*s == STRDEL && *(s + 1) == STRDEL)
				    || (*s == STRDEL2 && *(s + 1) == STRDEL2))
				   && *(s + 2) == '\0'))
			*resultp = -1;
		else
			*resultp = 0;
		/* short circuit parsing to end of expression */
		while (*s != '\0')
			s++;
		tok_type = T_EMPTY;
		scan_pos = s;
		return(E_OK);
	case T_TYPE:
		if (get_token() != E_OK || factor(&value) != E_OK)
			*resultp = 0;
		else
			*resultp = 0x20; /* local defined absolute */
		return(E_OK);
	case T_ADD:
	case T_SUB:
	case T_NOT:
	case T_HIGH:
	case T_LOW:
		opr_type = tok_type;
		if ((err = get_token()) != E_OK
		    || (err = factor(&value)) != E_OK)
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
		return(E_OK);
	case T_LPAREN:
		if ((err = get_token()) != E_OK)
			return(err);
		if ((erru = expr(&value)) > E_UNDSYM)
			return(erru);
		if (tok_type == T_RPAREN)
			if ((err = get_token()) != E_OK)
				return(err);
			else if (erru != E_OK)
				return(erru);
			else {
				*resultp = value;
				return(E_OK);
 			}
		else
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

	if ((erru = factor(resultp)) > E_UNDSYM)
		return(erru);
	while (tok_type == T_MUL || tok_type == T_DIV || tok_type == T_MOD
				 || tok_type == T_SHR || tok_type == T_SHL) {
		opr_type = tok_type;
		if ((err = get_token()) != E_OK)
			return(err);
		if ((err = factor(&value)) > E_UNDSYM)
			return(err);
		if (err != E_OK) {
			erru = err;
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
	return(erru);
}

int add_term(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = mul_term(resultp)) > E_UNDSYM)
		return(erru);
	while (tok_type == T_ADD || tok_type == T_SUB) {
		opr_type = tok_type;
		if ((err = get_token()) != E_OK)
			return(err);
		if ((err = mul_term(&value)) > E_UNDSYM)
			return(err);
		if (err != E_OK) {
			erru = err;
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
	return(erru);
}

int cmp_term(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = add_term(resultp)) > E_UNDSYM)
		return(erru);
	while (tok_type == T_EQ || tok_type == T_NE
				|| tok_type == T_LT || tok_type == T_LE
				|| tok_type == T_GT || tok_type == T_GE) {
		opr_type = tok_type;
		if ((err = get_token()) != E_OK)
			return(err);
		if ((err = add_term(&value)) > E_UNDSYM)
			return(err);
		if (err != E_OK) {
			erru = err;
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
	return(erru);
}

int expr(WORD *resultp)
{
	register int err, erru;
	register BYTE opr_type;
	WORD value;

	if ((erru = cmp_term(resultp)) > E_UNDSYM)
		return(erru);
	while (tok_type == T_AND || tok_type == T_XOR || tok_type == T_OR) {
		opr_type = tok_type;
		if ((err = get_token()) != E_OK)
			return(err);
		if ((err = cmp_term(&value)) > E_UNDSYM)
			return(err);
		if (err != E_OK) {
			erru = err;
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
	return(erru);
}

/*
 *	parse and evaluate string s
 *	returns result if valid expression, otherwise 0 and error message
 */
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
	if ((err = get_token()) != E_OK || (err = expr(&result)) != E_OK) {
		asmerr(err);
		return(0);
	} else if (tok_type != T_EMPTY) {	/* leftovers, error out */
		asmerr(E_INVEXP);
		return(0);
	} else
		return(result);
}

/*
 *	check w for range -256 <= w <= 255
 *	returns w as BYTE if in range, otherwise 0 and error message
 */
BYTE chk_byte(WORD w)
{
	if (w >= (WORD) -256 || w <= 255)
		return(w);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}

/*
 *	check w for range -128 <= w <= 127
 *	returns w as BYTE if in range, otherwise 0 and error message
 */
BYTE chk_sbyte(WORD w)
{
	if (w >= (WORD) -128 || w <= 127)
		return(w);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}
