/*
 *	Z80 - Macro - Assembler - Intel-like macro implementation
 *	Copyright (C) 2022 by Thomas Eberhardt
 *
 *	History:
 *	25-OCT-2022 Intel-like macros (TE)
 */

/*
 *	processing of all macro PSEUDO ops
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

/* z80amain.c */
extern void fatal(int, const char *);
extern char *next_arg(char *, int *);

/* z80anum.c */
extern int is_first_sym_char(char);
extern int is_sym_char(char);
extern int eval(char *);

/* z80aout.c */
extern void asmerr(int);

/* z80atab.c */
extern char *strsave(char *);

#define MACNEST	16				/* max. expansion nesting */

struct dum {					/* macro dummy */
	char *dum_name;				/* dummy name */
	struct dum *dum_next;
};

struct line {					/* macro source line */
	char *line_text;			/* source line */
	struct line *line_next;
};

struct expn;

struct mac {					/* macro */
	void (*mac_start)(struct expn *);	/* start expansion function */
	int (*mac_rept)(struct expn *);		/* repeat expansion function */
	char *mac_name;				/* macro name */
	int mac_count;				/* REPT count */
	char *mac_irp;				/* IRP, IRPC character list */
	struct dum *mac_dums, *mac_dums_last;	/* macro dummies */
	struct line *mac_lines, *mac_lines_last; /* macro body */
	struct mac *mac_next;
};

struct parm {					/* expansion parameter */
	char *parm_name;			/* dummy name */
	char *parm_val;				/* parameter value */
	struct parm *parm_next;
};

struct expn {					/* macro expansion */
	struct mac *expn_mac;			/* macro being expanded */
	struct parm *expn_parms, *expn_parms_last; /* macro parameters */
	struct parm *expn_locs, *expn_locs_last; /* locals (after parms) */
	struct line *expn_line;			/* current expansion line */
	int expn_iflevel;			/* iflevel before expansion */
	int expn_iter;				/* curr. expansion iteration */
	char *expn_irp;				/* IRP, IRPC character list */
};

static struct mac *mac_table;			/* MACRO table */
static struct mac *mac_def;			/* macro being defined */
static struct mac *mac_found;			/* found macro */
static struct expn mac_expn[MACNEST];		/* macro expansion stack */
static int mac_loc_cnt;				/* counter for LOCAL labels */
static char tmp[MAXLINE];			/* temporary buffer */
static char expr[MAXLINE];			/* expression buffer (for '%') */

/*
 *	verify that p is a legal symbol
 *	return 1 if legal, otherwise 0
 */
int is_symbol(char *s)
{
	if (!is_first_sym_char(*s++))
		return(0);
	while (is_sym_char(*s))
		s++;
	return(*s == '\0');
}

/*
 *	allocate a new macro with optional name and
 *	start/repeat expansion function
 */
struct mac *mac_new(char *name, void (*start)(struct expn *),
		    int (*rept)(struct expn *))
{
	struct mac *m;

	if ((m = (struct mac *) malloc(sizeof(struct mac))) == NULL)
		fatal(F_OUTMEM, "macro");
	if (name != NULL) {
		if ((m->mac_name = strsave(name)) == NULL)
			fatal(F_OUTMEM, "macro name");
	} else
		m->mac_name = NULL;
	m->mac_start = start;
	m->mac_rept = rept;
	m->mac_count = 0;
	m->mac_irp = NULL;
	m->mac_dums_last = m->mac_dums = NULL;
	m->mac_lines_last = m->mac_lines = NULL;
	m->mac_next = NULL;
	return(m);
}

/*
 *	delete a macro
 */
void mac_delete(struct mac *m)
{
	struct dum *d, *d1;
	struct line *ln, *ln1;

	for (d = m->mac_dums; d != NULL; d = d1) {
		d1 = d->dum_next;
		free(d->dum_name);
		free(d);
	}
	for (ln = m->mac_lines; ln != NULL; ln = ln1) {
		ln1 = ln->line_next;
		free(ln->line_text);
		free(ln);
	}
	if (m->mac_irp != NULL)
		free(m->mac_irp);
	if (m->mac_name != NULL)
		free(m->mac_name);
	free(m);
}

/*
 *	initialize variables at start of pass
 */
void mac_start_pass(void)
{
	mac_loc_cnt = 0;
}

/*
 *	clean up at end of pass
 */
void mac_end_pass(void)
{
	register struct mac *m;

	while (mac_table != NULL) {
		m = mac_table->mac_next;
		mac_delete(mac_table);
		mac_table = m;
	}
}

/*
 * 	add a dummy to a macro
 */
void mac_add_dum(struct mac *m, char *name)
{
	register struct dum *d;

	d = (struct dum *) malloc(sizeof(struct dum));
	if (d == NULL || (d->dum_name = strsave(name)) == NULL)
		fatal(F_OUTMEM, "macro dummy");
	d->dum_next = NULL;
	if (m->mac_dums == NULL)
		m->mac_dums = d;
	else
		m->mac_dums_last->dum_next = d;
	m->mac_dums_last = d;
}

/*
 * 	add a parameter/local to a macro expansion
 */
struct parm *expn_add_parm(struct expn *e, char *name, int is_local)
{
	register struct parm *p;

	p = (struct parm *) malloc(sizeof(struct parm));
	if (p == NULL || (p->parm_name = strsave(name)) == NULL)
		fatal(F_OUTMEM, "macro parameter/local");
	p->parm_val = NULL;
	p->parm_next = NULL;
	if (e->expn_parms == NULL) {
		e->expn_parms = p;
		if (is_local)
			e->expn_locs = p;
	} else if (is_local) {
		if (e->expn_locs == NULL) {
			e->expn_locs = p;
			if (e->expn_parms_last != NULL)
				e->expn_parms_last->parm_next = p;
		} else
			e->expn_locs_last->parm_next = p;
	} else
		e->expn_parms_last->parm_next = p;
	if (is_local)
		e->expn_locs_last = p;
	else
		e->expn_parms_last = p;
	return(p);
}

/*
 *	start macro expansion
 *	assign values to parameters, save iflevel
 */
void mac_start_expn(struct mac *m)
{
	register struct expn *e;
	register struct dum *d;

	if (mac_exp_nest == MACNEST) {
		/* abort macro expansion */
		mac_exp_nest = 0;
		if ((iflevel = mac_expn[0].expn_iflevel) > 0)
			gencode = condnest[iflevel - 1];
		else
			gencode = pass;
		asmerr(E_MACNEST);
		return;
	}
	e = &mac_expn[mac_exp_nest];
	e->expn_mac = m;
	e->expn_parms_last = e->expn_parms = NULL;
	e->expn_locs_last = e->expn_locs = NULL;
	e->expn_line = m->mac_lines;
	e->expn_iflevel = iflevel;
	e->expn_iter = 0;
	e->expn_irp = m->mac_irp;
	for (d = m->mac_dums; d; d = d->dum_next)
		(void) expn_add_parm(e, d->dum_name, 0);
	(*m->mac_start)(e);
	mac_exp_nest++;
}

/*
 *	end macro expansion
 *	delete parameters and local labels, restore iflevel
 */
void mac_end_expn(void)
{
	register struct expn *e;
	register struct parm *p, *p1;
	register struct mac *m;

	e = &mac_expn[mac_exp_nest - 1];
	for (p = e->expn_parms; p; p = p1) {
		p1 = p->parm_next;
		if (p->parm_val != NULL)
			free(p->parm_val);
		free(p);
	}
	if ((iflevel = e->expn_iflevel) > 0)
		gencode = condnest[iflevel - 1];
	else
		gencode = pass;
	m = e->expn_mac;
	mac_exp_nest--;
	/* delete unnamed macros (IRP, IRPC, REPT) */
	if (m->mac_name == NULL)
		mac_delete(m);
}

/*
 *	repeat macro for IRP, IRPC, REPT when end reached
 *	end expansion for MACRO
 */
int mac_rept_expn(void)
{
	register struct expn *e;
	register struct mac *m;
	register struct parm *p, *p1;

	e = &mac_expn[mac_exp_nest - 1];
	e->expn_iter++;
	m = e->expn_mac;
	if (*m->mac_rept != NULL && (*m->mac_rept)(e)) {
		for (p = e->expn_locs; p; p = p1) {
			p1 = p->parm_next;
			if (p->parm_val != NULL)
				free(p->parm_val);
			free(p);
		}
		if (e->expn_parms_last == NULL)
			e->expn_parms = NULL;
		else
			e->expn_parms_last->parm_next = NULL;
		e->expn_locs_last = e->expn_locs = NULL;
		e->expn_line = m->mac_lines;
		if ((iflevel = e->expn_iflevel) > 0)
			gencode = condnest[iflevel - 1];
		else
			gencode = pass;
		return(1);
	} else {
		mac_end_expn();
		return(0);
	}
}

/*
 *	add source line l to current macro definition
 */
void mac_add_line(struct opc *op, char *line)
{
	register struct line *l;
	register struct mac *m;

	a_mode = A_NONE;
	l = (struct line *) malloc(sizeof(struct line));
	if (l == NULL || (l->line_text = strsave(line)) == NULL)
		fatal(F_OUTMEM, "macro body line");
	l->line_next = NULL;
	m = mac_def;
	if (m->mac_lines == NULL)
		m->mac_lines = l;
	else
		m->mac_lines_last->line_next = l;
	m->mac_lines_last = l;
	if (op != NULL) {
		if (op->op_flags & OP_MDEF)
			mac_def_nest++;
		else if (op->op_flags & OP_MEND) {
			if (--mac_def_nest == 0) {
				m = mac_def;
				mac_def = NULL;
				/* start expansion for IRP, IRPC, REPT */
				if (m->mac_name == NULL)
					mac_start_expn(m);
			}
		}
	}
}

/*
 *	return value of dummy or local s, NULL if not found
 */
const char *mac_get_dummy(struct expn *e, char *s)
{
	register struct parm *p;

	for (p = e->expn_parms; p; p = p->parm_next)
		if (strcmp(p->parm_name, s) == 0)
			return(p->parm_val == NULL ? "" : p->parm_val);
	return(NULL);
}

/*
 *	substitute dummies and locals with actual values
 *	in current expansion source line and return the
 *	result
 */
char *mac_subst_dummies(struct expn *e)
{
	register char *s, *t, *t1;
	register const char *v;
	register char c;
	register int n;
	int amp_flag;	/* 0 = no &, 1 = & before, 2 = & after */
	int esc_flag;	/* 0 = no ^, 1 = ^ before */

	s = e->expn_line->line_text;
	if (*s == LINCOM)
		return(s);
	t = line;
	n = 0;
	amp_flag = esc_flag = 0;
	while (*s != '\n' && *s != '\0') {
		if (is_first_sym_char(*s)) {
			t1 = t;
			*t++ = toupper((unsigned char) *s++);
			while (is_sym_char(*s))
				*t++ = toupper((unsigned char) *s++);
			*t = '\0';
			v = mac_get_dummy(e, t1);
			if (v == NULL)
				amp_flag = 0;
			else if (esc_flag) {
				t = t1 - 1;
				while (*(t + 1) != '\0') {
					*t = *(t + 1);
					t++;
				}
			} else {
				if (amp_flag == 1)
					t1--;
				t = t1;
				while (*v != '\0')
					*t++ = *v++;
				if (*s == '&') {
					amp_flag = 2;
					s++;
				} else
					amp_flag = 0;
			}
			esc_flag = 0;
		} else if (*s == STRDEL || *s == STRDEL2) {
			*t++ = c = *s++;
			amp_flag = 0;
			while (1) {
				if (*s == '\n' || *s == '\0') {
					asmerr(E_MISDEL);
					goto done;
				} else if (*s == c) {
					amp_flag = 0;
					*t++ = *s++;
					if (*s != c)
						break;
					else {
						*t++ = *s++;
						continue;
					}
				} else if (is_first_sym_char(*s)) {
					t1 = t;
					*t++ = *s++;
					while (is_sym_char(*s))
						*t++ = *s++;
					*t = '\0';
					if (amp_flag > 0 || *s == '&') {
						v = mac_get_dummy(e, t1);
						if (v == NULL)
							amp_flag = 0;
						else {
							if (amp_flag == 1)
								t1--;
							t = t1;
							while (*v != '\0')
								*t++ = *v++;
							if (*s == '&') {
								amp_flag = 2;
								s++;
							} else
								amp_flag = 0;
						}
					}
				} else {
					amp_flag = (*s == '&');
					*t++ = *s++;
				}
			}
			esc_flag = 0;
		} else if (*s == COMMENT && n == 0) {
			if (*(s + 1) != COMMENT)
				while (*s != '\n' && *s != '\0')
					*t++ = *s++;
			goto done;
		} else {
			if (*s == '<')
				n++;
			else if (*s == '>') {
				if (n == 0) {
					asmerr(E_ILLOPE);
					goto done;
				} else
					n--;
			}
			amp_flag = (*s == '&');
			esc_flag = (*s == '^');
			*t++ = toupper((unsigned char) *s++);
		}
	}
	if (n > 0)
		asmerr(E_MISPAR);
done:
	*t++ = '\n';
	*t = '\0';
	return(line);
}

/*
 *	get next macro expansion line
 */
char *mac_expand(void)
{
	register struct expn *e;
	register char *s;

	e = &mac_expn[mac_exp_nest - 1];
	if (e->expn_line == NULL && !mac_rept_expn())
		return(NULL);
	s = mac_subst_dummies(e);
	e->expn_line = e->expn_line->line_next;
	return(s);
}

/*
 *	macro lookup
 */
int mac_lookup(char *opcode)
{
	register struct mac *m;

	mac_found = NULL;
	for (m = mac_table; m; m = m->mac_next)
		if (strcmp(m->mac_name, opcode) == 0) {
			mac_found = m;
			return(1);
		}
	return(0);
}

/*
 *	MACRO invocation, to be called after successful mac_lookup()
 */
void mac_call(void)
{
	if (mac_found != NULL)
		mac_start_expn(mac_found);
	else
		fatal(F_INTERN, "mac_call with no macro");
}

/*
 *	get next macro parameter
 */
char *mac_next_parm(char *s)
{
	register char *t, *t1, *u, c;
	register int n;

	t1 = t = tmp;
	n = 0;
	while (isspace((unsigned char) *s))
		s++;
	while (*s != '\n' && *s != '\0') {
		if (*s == STRDEL || *s == STRDEL2) {
			/* keep delimiters, but reduce double delimiters */
			*t++ = c = *s++;
			while (1) {
				if (*s == '\n' || *s == '\0') {
					asmerr(E_MISDEL);
					return(NULL);
				} else if (*s == c) {
					if (*(s + 1) != c)
						break;
					else
						s++;
				}
				*t++ = *s++;
			}
			*t++ = *s++;
		} else if (*s == '%') {
			/* evaluate as expression */
			s++;
			u = expr;
			while (*s != '\n' && *s != '\0'
					  && *s != ',' && *s != COMMENT) {
				if (*s == STRDEL || *s == STRDEL2) {
					*u++ = c = *s++;
					while (1) {
						if (*s == '\n' || *s == '\0') {
							asmerr(E_MISDEL);
							return(NULL);
						} else if (*s == c) {
							*u++ = *s++;
							if (*s != c)
								break;
						}
						*u++ = *s++;
					}
				} else if (*s == '^') {
					/* escape next character */
					s++;
					if (*s == '\n' || *s == '\0') {
						asmerr(E_ILLOPE);
						return(NULL);
					} else
						*u++ = *s++;
				} else
					*u++ = toupper((unsigned char) *s++);
			}
			*u = '\0';
			t += sprintf(t, "%d", eval(expr));
			t1 = t;
			break;
		} else if (*s == '^') {
			/* escape next character */
			s++;
			if (*s == '\n' || *s == '\0') {
				asmerr(E_ILLOPE);
				return(NULL);
			} else
				*t++ = *s++;
		} else if (*s == '<') {
			if (n > 0)
				*t++ = *s++;
			else
				s++;
			n++;
		} else if (*s == '>') {
			if (n == 0) {
				asmerr(E_ILLOPE);
				return(NULL);
			} else {
				n--;
				if (n > 0)
					*t++ = *s++;
				else
					s++;
			}
		} else if ((*s == ',' || *s == COMMENT) && n == 0)
			break;
		else
			*t++ = toupper((unsigned char) *s++);
		t1 = t;
		while (isspace((unsigned char) *s) && *s != '\n')
			*t++ = *s++;
	}
	if (n > 0) {
		asmerr(E_MISPAR);
		return(NULL);
	}
	*t1 = '\0';
	return s;
}

/*
 *	start IRP macro expansion
 */
void mac_start_irp(struct expn *e)
{
	register char *s;

	if (*e->expn_irp != '\0') {
		if ((s = mac_next_parm(e->expn_irp)) != NULL) {
			e->expn_irp = s;
			if ((s = strsave(tmp)) == NULL)
				fatal(F_OUTMEM, "IRP character list");
			e->expn_parms->parm_val = s;
		}
	}
}

/*
 *	repeat IRP macro expansion
 */
int mac_rept_irp(struct expn *e)
{
	register char *s;

	s = e->expn_irp;
	if (*s == '\0')
		return(0);
	else if (*s++ != ',') {
		asmerr(E_ILLOPE);
		return(0);
	} else {
		if ((s = mac_next_parm(s)) == NULL)
			return(0);
		e->expn_irp = s;
		if ((s = strsave(tmp)) == NULL)
			fatal(F_OUTMEM, "IRP character list");
		if (e->expn_parms->parm_val != NULL)
			free(e->expn_parms->parm_val);
		e->expn_parms->parm_val = s;
		return(1);
	}
}

/*
 *	start IRPC macro expansion
 */
void mac_start_irpc(struct expn *e)
{
	register char *s;

	if (*e->expn_irp != '\0') {
		if ((s = (char *) malloc(2)) == NULL)
			fatal(F_OUTMEM, "IRPC character");
		*s = *e->expn_irp++;
		*(s + 1) = '\0';
		e->expn_parms->parm_val = s;
	}
}

/*
 *	repeat IRPC macro expansion
 */
int mac_rept_irpc(struct expn *e)
{
	if (*e->expn_irp != '\0') {
		e->expn_parms->parm_val[0] = *e->expn_irp++;
		return(1);
	} else
		return(0);
}

/*
 *	start MACRO macro expansion
 */
void mac_start_macro(struct expn *e)
{
	register char *s;
	register struct parm *p;

	s = operand;
	p = e->expn_parms;
	while (p != NULL && *s != '\n' && *s != '\0' && *s != COMMENT) {
		if ((s = mac_next_parm(s)) == NULL)
			return;
		if (*s == ',')
			s++;
		else if (*s != '\0' && *s != COMMENT) {
			asmerr(E_ILLOPE);
			return;
		}
		if ((p->parm_val = strsave(tmp)) == NULL)
			fatal(F_OUTMEM, "parameter assignment");
		p = p->parm_next;
	}
}

/*
 *	start REPT macro expansion
 */
void mac_start_rept(struct expn *e)
{
	if (e->expn_mac->mac_count <= 0)
		e->expn_line = NULL;
}

/*
 *	repeat REPT macro expansion
 */
int mac_rept_rept(struct expn *e)
{
	return(e->expn_iter < e->expn_mac->mac_count);
}

/*
 *	ENDM
 */
int op_endm(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	if (mac_exp_nest == 0)
		asmerr(E_NIMEXP);
	else
		(void) mac_rept_expn();
	return(0);
}

/*
 *	EXITM
 */
int op_exitm(int dummy1, int dummy2)
{
	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	if (mac_exp_nest == 0)
		asmerr(E_NIMEXP);
	else
		mac_end_expn();
	return(0);
}

/*
 *	IFB, IFNB, IFIDN, IFDIF
 */
int op_mcond(int op_code, int dummy)
{
	register char *s, *t;

	UNUSED(dummy);

	a_mode = A_NONE;
	if (iflevel >= IFNEST) {
		asmerr(E_IFNEST);
		return(0);
	}
	condnest[iflevel++] = gencode;
	if (gencode < 0)
		return(0);
	switch(op_code) {
	case 1:				/* IFB */
	case 2:				/* IFNB */
		s = mac_next_parm(operand);
		if (*s != '\0' && *s != COMMENT) {
			asmerr(E_ILLOPE);
			return(0);
		}
		gencode = (*tmp == '\0') ? pass : -pass;
		break;
	case 3:				/* IFIDN */
	case 4:				/* IFDIF */
		s = mac_next_parm(operand);
		if (*s++ != ',') {
			asmerr(E_MISOPE);
			return(0);
		}
		if ((t = strsave(tmp)) == NULL)
			fatal(F_OUTMEM, "macro IF parameter");
		s = mac_next_parm(s);
		if (*s != '\0' && *s != COMMENT) {
			asmerr(E_ILLOPE);
			free(t);
			return(0);
		}
		gencode = (strcmp(t, s) == 0) ? pass : -pass;
		free(t);
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_mcond");
		break;
	}
	if ((op_code & 1) == 0)		/* negate for inverse IF */
		gencode = -gencode;
	return(0);
}

/*
 *	IRP and IRPC
 */
int op_irp(int op_code, int dummy)
{
	register char *s, *t;
	register struct mac *m;

	UNUSED(dummy);

	a_mode = A_NONE;
	m = NULL;			/* silence compiler */
	switch(op_code) {
	case 1:				/* IRP */
		m = mac_new(NULL, mac_start_irp, mac_rept_irp);
		break;
	case 2:				/* IRPC */
		m = mac_new(NULL, mac_start_irpc, mac_rept_irpc);
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_irp");
		break;
	}
	s = operand;
	t = tmp;
	if (!is_first_sym_char(*s)) {
		asmerr(E_ILLOPE);
		return(0);
	}
	*t++ = toupper((unsigned char) *s++);
	while (is_sym_char(*s))
		*t++ = toupper((unsigned char) *s++);
	*t = '\0';
	mac_add_dum(m, tmp);
	while (isspace((unsigned char) *s))
		s++;
	if (*s++ != ',') {
		asmerr(E_ILLOPE);
		return(0);
	}
	while (isspace((unsigned char) *s))
		s++;
	s = mac_next_parm(s);
	if (*s != '\0' && *s != COMMENT) {
		asmerr(E_ILLOPE);
		return(0);
	}
	if ((m->mac_irp = strsave(tmp)) == NULL)
		fatal(F_OUTMEM, "IRP/IRPC character list");
	mac_def = m;
	mac_def_nest++;
	return(0);
}

/*
 *	LOCAL
 */
int op_local(int dummy1, int dummy2)
{
	register char *s, *s1;
	register struct expn *e;
	register struct parm *p;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	if (mac_exp_nest == 0) {
		asmerr(E_NIMEXP);
		return(0);
	}
	e = &mac_expn[mac_exp_nest - 1];
	s = operand;
	while (s != NULL) {
		s1 = next_arg(s, NULL);
		if (*s != '\0') {
			if (is_symbol(s)) {
				p = expn_add_parm(e, s, 1);
				if (mac_loc_cnt == 10000)
					asmerr(E_OUTLCL);
				else
					mac_loc_cnt++;
				sprintf(tmp, "??%04d", mac_loc_cnt);
				if ((p->parm_val = strsave(tmp)) == NULL)
					fatal(F_OUTMEM, "local assignment");
			} else
				asmerr(E_ILLOPE);
		}
		s = s1;
	}
	return(0);
}

/*
 *	MACRO
 */
int op_macro(int dummy1, int dummy2)
{
	register char *s, *s1;
	register struct mac *m;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	m = mac_new(label, mac_start_macro, NULL);
	m->mac_next = mac_table;
	mac_table = m;
	s = operand;
	while (s != NULL) {
		s1 = next_arg(s, NULL);
		if (*s != '\0') {
			if (is_symbol(s))
				mac_add_dum(m, s);
			else
				asmerr(E_ILLOPE);
		}
		s = s1;
	}
	mac_def = m;
	mac_def_nest++;
	return(0);
}

/*
 *	REPT
 */
int op_rept(int dummy1, int dummy2)
{
	register struct mac *m;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	m = mac_new(NULL, mac_start_rept, mac_rept_rept);
	m->mac_count = eval(operand);
	mac_def = m;
	mac_def_nest++;
	return(0);
}
