/*
 *	Z80 - Assembler
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

int expr(int *);

/* z80aout.c */
extern void asmerr(int);

/* z80atab.c */
extern struct sym *get_sym(char *);

/*
 *	definition of token types
 */
#define T_EMPTY		0	/* nothing */
#define T_VAL		1	/* number, string, or $ */
#define T_SYM		2	/* symbol */
#define T_SUB		3	/* - */
#define T_ADD		4	/* + */
#define T_NOT		5	/* NOT or ~ */
#define T_HIGH		6	/* HIGH */
#define T_LOW		7	/* LOW */
#define T_NUL		8	/* NUL */
#define T_TYPE		9	/* TYPE */
#define T_MUL		10	/* * */
#define T_DIV		11	/* / */
#define T_MOD		12	/* MOD */
#define T_SHR		13	/* SHR or >> */
#define T_SHL		14	/* SHL or << */
#define T_EQ		15	/* EQ, = or == */
#define T_NE		16	/* NE, <> or != */
#define T_LT		17	/* LT or < */
#define T_LE		18	/* LE or <= */
#define T_GT		19	/* GT or > */
#define T_GE		20	/* GE or >= */
#define T_AND		21	/* AND or & */
#define T_XOR		22	/* XOR */
#define T_OR		23	/* OR or | */
#define T_LPAREN	24	/* (, [ or { */
#define T_RPAREN	25	/* ), ] or } */

/*
 *	structure operator table
 */
struct opr {
	char *opr_name;		/* operator name */
	int opr_type;		/* operator token type */
};

/*
 *	table with operators
 *	must be sorted in ascending order!
 */
struct opr oprtab[] = {
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

int no_operators = sizeof(oprtab) / sizeof(struct opr);

static int tok_type;	/* token type and flags */
static int tok_val;	/* token value for T_VAL type */
static char tok_sym[MAXLINE]; /* last symbol scanned (T_VAL, T_SYM) */
static char *scan_pos;	/* current scanning position */

int is_sym_char(char c)
{
	return(isalnum((unsigned char) c) || c == '$' || c == '%' || c == '.'
					  || c == '?' || c == '@' || c == '_');
}

/*
 *	binary search in sorted table oprtab
 *
 *	Input: pointer to string with operator
 *
 *	Output: symbol for operator, T_SYM if operator not found
 */
int search_opr(char *s)
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
	return(T_SYM);
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
	register int n, m, base;

	s = scan_pos;
	tok_sym[0] = '\0';
	tok_val = 0;
	while (isspace((unsigned char) *s))		/* skip white space */
		s++;
	if (*s == '\0') {				/* nothing there? */
		tok_type = T_EMPTY;
		goto finish;
	}
	if (*s == 'X' && *(s + 1) == '\'') {		/* X'h' hex constant */
		s += 2;
		n = 0;
		while (isxdigit((unsigned char) *s)) {
			n *= 16;
			n += toupper((unsigned char) *s)
			     - ((*s <= '9') ? '0' : '7');
			s++;
		}
		if (*s != '\'')				/* missing final ' */
			return(E_MISDEL);
		else {
			s++;
			tok_type = T_VAL;
			tok_val = n;
			goto finish;
		}
	}
	p1 = p2 = tok_sym;				/* gather symbol */
	while (is_sym_char(*s))
		*p2++ = *s++;
	*p2 = '\0';
	if (p1 != p2) {					/* a number/symbol */
		if (isdigit((unsigned char) *p1)) {	/* a number */
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
				if (isxdigit((unsigned char) *p1)) {
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
			goto finish;
		}
		if (*p1 == '$' && *(p1 + 1) == '\0') {	/* location counter */
			tok_type = T_VAL;
			tok_val = pc;
		} else {				/* look for word opr */
			tok_type = search_opr(p1);
			if (p2 - p1 > symlen)		/* trim for lookup */
				*(p1 + symlen) = '\0';
		}
		goto finish;
	}
	switch (*s) {
	case '\'':					/* character constant */
	case '"':
		p1 = s;
		n = m = 0;
		while (*++s != '\0') {
			if (*s == *p1 && *++s != *p1) {	/* double delim? */
				tok_type = T_VAL;
				tok_val = n;
				goto finish;
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
	case '&':
		tok_type = T_AND;
		break;
	case '(':
	case '[':
	case '{':
		tok_type = T_LPAREN;
		break;
	case ')':
	case ']':
	case '}':
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
finish:
	scan_pos = s;
	return(E_NOERR);
}

/*
 *	recursive descent parser & evaluator
 *
 *	inspired by the previous expression parser by Didier Derny.
 */

int factor(int *resultp)
{
	int opr_type, value, err, erru;
	struct sym *sp;

	/*
	 *	if the token is not a T_VAL, but a word operator or
	 *	T_SYM, look it up now since tok_sym gets overwritten
	 *	by the next get_token().
	 */
	sp = (tok_type != T_VAL && *tok_sym) ? get_sym(tok_sym) : NULL;
	switch (tok_type) {
	case T_VAL:
		value = tok_val;
		if ((err = get_token()))
			return(err);
		*resultp = value;
		return(E_NOERR);
	case T_SYM:
		if ((err = get_token()))
			return(err);
		if (sp) {
			*resultp = sp->sym_val;
			return(E_NOERR);
		} else
			return(E_UNDSYM);
	case T_LPAREN:
		if ((err = get_token()))
			return(err);
		if ((erru = expr(&value)) && erru != E_UNDSYM)
			return(erru);
		if (tok_type == T_RPAREN) {
			if ((err = get_token()))
				return(err);
			else if (erru)		/* E_UNDSYM */
				return(E_UNDSYM);
			else {
				*resultp = value;
				return(E_NOERR);
			}
		} else
			return(E_MISPAR);
	case T_ADD:
	case T_SUB:
	case T_NOT:
	case T_HIGH:
	case T_LOW:
	case T_NUL:
	case T_TYPE:
		opr_type = tok_type;
		if ((err = get_token()))
			return(err);
		/*
		 *	if an unary word operator, that was found in the
		 *	symbol table, is not followed by a <factor> excl.
		 *	unary + and - (which are also binary), return the
		 *	symbol value.
		 */
		if (sp != NULL && tok_type != T_VAL && tok_type != T_SYM
			       && tok_type != T_LPAREN && tok_type != T_NOT
			       && tok_type != T_HIGH && tok_type != T_LOW
			       && tok_type != T_NUL && tok_type != T_TYPE)
			*resultp = sp->sym_val;
		else if (opr_type == T_NUL) {
			*resultp = (tok_type == T_EMPTY) ? -1 : 0;
			/* short circuit to end of expression */
			while (*scan_pos != '\0')
				scan_pos++;
			tok_type = T_EMPTY;
		} else if (opr_type == T_TYPE)
			*resultp = (factor(&value) ? 0x00 : 0x20);
		else {
			if ((err = factor(&value)))
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
				*resultp = (value >> 8) & 0xff;
				break;
			case T_LOW:
				*resultp = value & 0xff;
				break;
			default:
				break;
			}
		}
		return(E_NOERR);
	default:
		/*
		 *	if the token is an unexpected word operator
		 *	that was found in the symbol table, return
		 *	the symbol value.
		 */
		if (sp) {
			if ((err = get_token()))
				return(err);
			*resultp = sp->sym_val;
			return(E_NOERR);
		} else
			return(E_INVEXP);
	}
}

int mul_term(int *resultp)
{
	int opr_type, value, err, erru;

	if ((erru = factor(resultp)) && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_MUL || tok_type == T_DIV || tok_type == T_MOD
				 || tok_type == T_SHR || tok_type == T_SHL) {
		opr_type = tok_type;
		if ((err = get_token()))
			return(err);
		if ((err = factor(&value)) && err != E_UNDSYM)
			return(err);
		if (erru || err) {		/* E_UNDSYM */
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
			*resultp = ((unsigned) *resultp) >> value;
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

int add_term(int *resultp)
{
	int opr_type, value, err, erru;

	if ((erru = mul_term(resultp)) && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_ADD || tok_type == T_SUB) {
		opr_type = tok_type;
		if ((err = get_token()))
			return(err);
		if ((err = mul_term(&value)) && err != E_UNDSYM)
			return(err);
		if (erru || err) {		/* E_UNDSYM */
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

int cmp_term(int *resultp)
{
	int opr_type, value, err, erru;

	if ((erru = add_term(resultp)) && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_EQ || tok_type == T_NE
				|| tok_type == T_LT || tok_type == T_LE
				|| tok_type == T_GT || tok_type == T_GE) {
		opr_type = tok_type;
		if ((err = get_token()))
			return(err);
		if ((err = add_term(&value)) && err != E_UNDSYM)
			return(err);
		if (erru || err) {		/* E_UNDSYM */
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

int expr(int *resultp)
{
	int opr_type, value, err, erru;

	if ((erru = cmp_term(resultp)) && erru != E_UNDSYM)
		return(erru);
	while (tok_type == T_AND || tok_type == T_XOR || tok_type == T_OR) {
		opr_type = tok_type;
		if ((err = get_token()))
			return(err);
		if ((err = cmp_term(&value)) && err != E_UNDSYM)
			return(err);
		if (erru || err) {		/* E_UNDSYM */
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

int eval(char *s)
{
	int result, err;

	if (s == NULL) {
		asmerr(E_MISOPE);
		return(0);
	}
	result = 0;
	scan_pos = s;
	if ((err = get_token()) || (err = expr(&result))) {
		asmerr(err);
		return(0);
	} else if (tok_type != T_EMPTY) {	/* leftovers, error out */
		asmerr(E_INVEXP);
		return(0);
	} else
		return(result);
}

/*
 *	check value for range -257 < value < 256
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_byte(int i)
{
	if (i >= -256 && i <= 255)
		return(i);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}

/*
 *	check value for range -129 < value < 128
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_sbyte(int i)
{
	if (i >= -128 && i <= 127)
		return(i);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}
