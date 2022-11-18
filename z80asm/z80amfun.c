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
#include <limits.h>
#include "z80a.h"
#include "z80aglb.h"

/* z80amain.c */
extern void fatal(int, const char *);
extern char *strsave(char *);
extern char *next_arg(char *, int *);

/* z80anum.c */
extern WORD eval(char *);

/* z80aout.c */
extern void asmerr(int);

#define CONCAT		'&'			/* parameter concatenation */
#define LITERAL		'^'			/* literal character escape */
#define BYVALUE		'%'			/* pass by value */
#define LBRACK		'<'			/* left angle bracket */
#define RBRACK		'>'			/* right angle bracket */

#define NO_LITERAL	0			/* mac_subst lit_flag values */
#define LIT_BEFORE	1

#define NO_CONCAT	0			/* mac_subst cat_flag values */
#define CAT_BEFORE	1
#define CAT_AFTER	2

struct dum {					/* macro dummy */
	char *dum_name;				/* dummy name */
	struct dum *dum_next;			/* next dummy in list */
};

struct line {					/* macro source line */
	char *line_text;			/* source line */
	struct line *line_next;			/* next line in list */
};

struct expn;

struct mac {					/* macro */
	void (*mac_start)(struct expn *);	/* start expansion function */
	int (*mac_rept)(struct expn *);		/* repeat expansion function */
	char *mac_name;				/* macro name */
	int mac_refflg;				/* macro reference flag */
	WORD mac_nrept;				/* REPT count */
	char *mac_irp;				/* IRP, IRPC character list */
	struct dum *mac_dums, *mac_dums_last;	/* macro dummies */
	struct line *mac_lines, *mac_lines_last; /* macro body */
	struct mac *mac_prev, *mac_next;	/* prev./next macro in list */
};

struct parm {					/* expansion parameter */
	char *parm_name;			/* dummy name */
	char *parm_val;				/* parameter value */
	struct parm *parm_next;			/* next parameter in list */
};

struct loc {					/* expansion local label */
	char *loc_name;				/* local label name */
	char loc_val[8];			/* local label value ??xxxx */
	struct loc *loc_next;			/* next local label in list */
};

struct expn {					/* macro expansion */
	struct mac *expn_mac;			/* macro being expanded */
	struct parm *expn_parms, *expn_parms_last; /* macro parameters */
	struct loc *expn_locs, *expn_locs_last;	/* local labels */
	struct line *expn_line;			/* current expansion line */
	int expn_iflevel;			/* iflevel before expansion */
	int expn_act_iflevel;			/* act_iflevel before expn */
	int expn_act_elselevel;			/* act_elselevel before expn */
	WORD expn_iter;				/* curr. expansion iteration */
	char *expn_irp;				/* IRP, IRPC character list */
	struct expn *expn_next;			/* next expansion in list */
};

static struct mac *mac_table;			/* MACRO table */
static struct mac *mac_curr;			/* current macro */
static struct mac **mac_array;			/* sorted table for iterator */
static int mac_index;				/* index for iterator */
static int mac_sort;				/* sort mode for iterator */
static int mac_count;				/* number of macros defined */
static struct expn *mac_expn;			/* macro expansion stack */
static WORD mac_loc_cnt;			/* counter for LOCAL labels */
static char tmp[MAXLINE + 1];			/* temporary buffer */

/*
 *	verify that s is a legal symbol, also truncates to symlen
 *	returns 1 if legal, otherwise 0
 */
int is_symbol(char *s)
{
	register int i;

	if (!IS_FSYM(*s))
		return(0);
	s++;
	i = 1;
	while (IS_SYM(*s)) {
		if (i++ == symlen)
			*s = '\0';
		s++;
	}
	return(*s == '\0');
}

/*
 *	compare function for qsort of mac_array
 */
int mac_compare(const void *p1, const void *p2)
{
	return(strcmp((*(const struct mac **) p1)->mac_name,
		      (*(const struct mac **) p2)->mac_name));
}

/*
 *	get first macro name and refflg in *rp for listing
 *	sorted as specified in sort_mode
 */
char *mac_first(int sort_mode, int *rp)
{
	register struct mac *m;
	register int i;

	if (mac_count == 0)
		return(NULL);
	mac_sort = sort_mode;
	switch(sort_mode) {
	case SYM_UNSORT:
		for (m = mac_table; m->mac_next != NULL; m = m->mac_next)
			;
		mac_curr = m;
		*rp = mac_curr->mac_refflg;
		return(mac_curr->mac_name);
	case SYM_SORTN:
	case SYM_SORTA:
		mac_array = (struct mac **) malloc(sizeof(struct mac *)
						   * mac_count);
		if (mac_array == NULL)
			fatal(F_OUTMEM, "sorting macro table");
		i = 0;
		for (m = mac_table; m != NULL; m = m->mac_next)
			mac_array[i++] = m;
		qsort(mac_array, mac_count, sizeof(struct mac *), mac_compare);
		mac_index = 0;
		*rp = mac_array[mac_index]->mac_refflg;
		return(mac_array[mac_index]->mac_name);
	default:
		fatal(F_INTERN, "unknown sort mode in mac_first");
	}
	return(NULL);		/* silence compiler */
}

/*
 *	get next macro name and refflg in *rp for listing
 */
char *mac_next(int *rp)
{
	if (mac_sort == SYM_UNSORT) {
		mac_curr = mac_curr->mac_prev;
		if (mac_curr != NULL) {
			*rp = mac_curr->mac_refflg;
			return(mac_curr->mac_name);
		}
	} else if (++mac_index < mac_count) {
		*rp = mac_array[mac_index]->mac_refflg;
		return(mac_array[mac_index]->mac_name);
	}
	return(NULL);
}

/*
 *	allocate a new macro with optional name and
 *	start/repeat expansion function
 */
struct mac *mac_new(char *name, void (*start)(struct expn *),
		    int (*rept)(struct expn *))
{
	register struct mac *m;
	register int n;

	if ((m = (struct mac *) malloc(sizeof(struct mac))) == NULL)
		fatal(F_OUTMEM, "macro");
	if (name != NULL) {
		n = strlen(name);
		if ((m->mac_name = (char *) malloc(n + 1)) == NULL)
			fatal(F_OUTMEM, "macro name");
		strcpy(m->mac_name, name);
		if (n > mac_symmax)
			mac_symmax = n;
	} else
		m->mac_name = NULL;
	m->mac_start = start;
	m->mac_rept = rept;
	m->mac_refflg = 0;
	m->mac_nrept = 0;
	m->mac_irp = NULL;
	m->mac_dums_last = m->mac_dums = NULL;
	m->mac_lines_last = m->mac_lines = NULL;
	m->mac_next = m->mac_prev = NULL;
	return(m);
}

/*
 *	delete a macro
 */
void mac_delete(struct mac *m)
{
	register struct dum *d, *d1;
	register struct line *ln, *ln1;

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

	if (pass == 1)
		while (mac_table != NULL) {
			m = mac_table->mac_next;
			mac_delete(mac_table);
			mac_table = m;
			mac_count--;
		}
}

/*
 * 	add a dummy to a macro
 */
void mac_add_dum(struct mac *m, char *name)
{
	register struct dum *d;

	if ((d = (struct dum *) malloc(sizeof(struct dum))) == NULL)
		fatal(F_OUTMEM, "macro dummy");
	d->dum_name = strsave(name);
	d->dum_next = NULL;
	if (m->mac_dums == NULL)
		m->mac_dums = d;
	else
		m->mac_dums_last->dum_next = d;
	m->mac_dums_last = d;
}

/*
 * 	add a local to a macro expansion
 */
struct loc *expn_add_loc(struct expn *e, char *name)
{
	register struct loc *l;

	if ((l = (struct loc *) malloc(sizeof(struct loc))) == NULL)
		fatal(F_OUTMEM, "macro local label");
	l->loc_name = strsave(name);
	l->loc_next = NULL;
	if (e->expn_locs == NULL)
		e->expn_locs = l;
	else
		e->expn_locs_last->loc_next = l;
	e->expn_locs_last = l;
	return(l);
}

/*
 *	start macro expansion
 *	assign values to parameters, save iflevel
 */
void mac_start_expn(struct mac *m)
{
	register struct expn *e, *e1;
	register struct dum *d;
	register struct parm *p;

	if (mac_exp_nest == MACNEST) {
		/* abort macro expansion */
		for (e = mac_expn; e != NULL; e = e1) {
			if ((e1 = e->expn_next) == NULL) {
				iflevel = e->expn_iflevel;
				act_iflevel = e->expn_act_iflevel;
				act_elselevel = e->expn_act_elselevel;
				gencode = 1;
			}
			free(e);
		}
		/* delete unnamed macros (IRP, IRPC, REPT) */
		if (m->mac_name == NULL)
			mac_delete(m);
		asmerr(E_MACNST);
		return;
	}
	if ((e = (struct expn *) malloc(sizeof(struct expn))) == NULL)
		fatal(F_OUTMEM, "macro expansion");
	e->expn_mac = m;
	e->expn_parms_last = e->expn_parms = NULL;
	for (d = m->mac_dums; d != NULL; d = d->dum_next) {
		p = (struct parm *) malloc(sizeof(struct parm));
		if (p == NULL)
			fatal(F_OUTMEM, "macro parameter");
		p->parm_name = d->dum_name;
		p->parm_val = NULL;
		p->parm_next = NULL;
		if (e->expn_parms == NULL)
			e->expn_parms = p;
		else
			e->expn_parms_last->parm_next = p;
		e->expn_parms_last = p;
	}
	e->expn_locs_last = e->expn_locs = NULL;
	e->expn_line = m->mac_lines;
	e->expn_iflevel = iflevel;
	e->expn_act_iflevel = act_iflevel;
	e->expn_act_elselevel = act_elselevel;
	e->expn_iter = 0;
	e->expn_irp = m->mac_irp;
	m->mac_refflg = 1;
	(*m->mac_start)(e);
	e->expn_next = mac_expn;
	mac_expn = e;
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
	register struct loc *l, *l1;
	register struct mac *m;

	e = mac_expn;
	for (p = e->expn_parms; p != NULL; p = p1) {
		p1 = p->parm_next;
		if (p->parm_val != NULL)
			free(p->parm_val);
		free(p);
	}
	for (l = e->expn_locs; l != NULL; l = l1) {
		l1 = l->loc_next;
		free(l->loc_name);
		free(l);
	}
	iflevel = e->expn_iflevel;
	act_iflevel = e->expn_act_iflevel;
	act_elselevel = e->expn_act_elselevel;
	gencode = 1;
	m = e->expn_mac;
	mac_expn = e->expn_next;
	mac_exp_nest--;
	free(e);
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
	register struct loc *l, *l1;

	e = mac_expn;
	e->expn_iter++;
	m = e->expn_mac;
	if (*m->mac_rept != NULL && (*m->mac_rept)(e)) {
		for (l = e->expn_locs; l != NULL; l = l1) {
			l1 = l->loc_next;
			free(l->loc_name);
			free(l);
		}
		e->expn_locs_last = e->expn_locs = NULL;
		e->expn_line = m->mac_lines;
		iflevel = e->expn_iflevel;
		act_iflevel = e->expn_act_iflevel;
		act_elselevel = e->expn_act_elselevel;
		gencode = 1;
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
	if ((l = (struct line *) malloc(sizeof(struct line))) == NULL)
		fatal(F_OUTMEM, "macro body line");
	l->line_text = strsave(line);
	l->line_next = NULL;
	m = mac_curr;
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
				m = mac_curr;
				mac_curr = NULL;
				/* start expansion for IRP, IRPC, REPT */
				if (m->mac_name == NULL)
					mac_start_expn(m);
			}
		}
	}
}

/*
 *	get value of dummy s, NULL if not found
 */
const char *mac_get_dummy(struct expn *e, char *s)
{
	register struct parm *p;

	for (p = e->expn_parms; p != NULL; p = p->parm_next)
		if (strncmp(p->parm_name, s, symlen) == 0)
			return(p->parm_val == NULL ? "" : p->parm_val);
	return(NULL);
}

/*
 *	get value of local label s, NULL if not found
 */
const char *mac_get_local(struct expn *e, char *s)
{
	register struct loc *l;

	for (l = e->expn_locs; l != NULL; l = l->loc_next)
		if (strncmp(l->loc_name, s, symlen) == 0)
			return(l->loc_val);
	return(NULL);
}

/*
 *	substitute dummies or locals with actual values in source line s
 *	returns the result in t
 */
void mac_subst(char *t, char *s, struct expn *e,
	       const char *(*getf)(struct expn *, char *))
{
	register char *s1, *t0, *t1, c;
	register const char *v;
	register int n, m;
	int cat_flag;
	int lit_flag;

	if (*s == LINCOM || (*s == LINOPT && !IS_SYM(*(s + 1)))) {
		strcpy(t, s);
		return;
	}
	t0 = t;
	n = 0;		/* angle brackets nesting level */
	cat_flag = NO_CONCAT;
	lit_flag = NO_LITERAL;
	while (*s != '\0') {
		if (IS_FSYM(*s)) {
			/* gather symbol */
			s1 = s;
			t1 = t;
			*t++ = TO_UPP(*s);
			s++;
			while (IS_SYM(*s)) {
				*t++ = TO_UPP(*s);
				s++;
			}
			*t = '\0';
			v = (*getf)(e, t1);
			/* don't substitute dummy if leading LITERAL */
			if (v == NULL || lit_flag == LIT_BEFORE) {
				t = s;
				s = s1;
				s1 = t;
				t = t1;
				/* remove leading LITERAL if dummy */
				if (v != NULL && lit_flag == LIT_BEFORE)
					t--;
				while (s < s1)
					*t++ = *s++;
				cat_flag = NO_CONCAT;
				lit_flag = NO_LITERAL;
				continue;
			}
			/* substitute dummy */
			t = t1;
			/* remove leading CONCAT */
			if (cat_flag == CAT_BEFORE)
				t--;
			m = MAXLINE - (t - t0);
			while (*v != '\0') {
				if (m-- == 0) {
					asmerr(E_MACOVF);
					goto done;
				}
				*t++ = *v++;
			}
			/* skip trailing CONCAT */
			if (*s == CONCAT) {
				cat_flag = CAT_AFTER;
				s++;
			} else
				cat_flag = NO_CONCAT;
			lit_flag = NO_LITERAL;
		} else if (*s == STRDEL || *s == STRDEL2) {
			*t++ = c = *s++;
			cat_flag = NO_CONCAT;
			while (1) {
				if (*s == '\0') {
					asmerr(E_MISDEL);
					goto done;
				} else if (*s == c) {
					cat_flag = NO_CONCAT;
					*t++ = *s++;
					if (*s != c) /* double delim? */
						break;
					else {
						*t++ = *s++;
						continue;
					}
				} else if (!IS_FSYM(*s)) {
					if (*s == CONCAT)
						cat_flag = CAT_BEFORE;
					else
						cat_flag = NO_CONCAT;
					*t++ = *s++;
					continue;
				}
				/* gather symbol */
				t1 = t;
				*t++ = *s++;
				while (IS_SYM(*s))
					*t++ = *s++;
				/* subst. poss. dummy if CONCAT before/after */
				if (cat_flag != NO_CONCAT || *s == CONCAT) {
					*t = '\0';
					v = (*getf)(e, t1);
					/* not a dummy? */
					if (v == NULL) {
						cat_flag = NO_CONCAT;
						continue;
					}
					/* substitute dummy */
					t = t1;
					/* remove leading CONCAT */
					if (cat_flag == CAT_BEFORE)
						t--;
					m = MAXLINE - (t - t0);
					while (*v != '\0') {
						if (m-- == 0) {
							asmerr(E_MACOVF);
							goto done;
						}
						*t++ = *v++;
					}
					/* skip trailing CONCAT */
					if (*s == CONCAT) {
						cat_flag = CAT_AFTER;
						s++;
					} else
						cat_flag = NO_CONCAT;
				}
			}
			lit_flag = NO_LITERAL;
		} else if (*s == COMMENT && n == 0) {
			/* don't copy double COMMENT comments */
			if (*(s + 1) == COMMENT)
				*t = '\0';
			else
				strcpy(t, s);
			return;
		} else {
			cat_flag = NO_CONCAT;
			lit_flag = NO_LITERAL;
			if (*s == LBRACK)
				n++;
			else if (*s == RBRACK) {
				if (n == 0) {
					asmerr(E_INVOPE);
					goto done;
				} else
					n--;
			} else if (*s == CONCAT)
				cat_flag = CAT_BEFORE;
			else if (*s == LITERAL)
				lit_flag = LIT_BEFORE;
			*t++ = *s++;
		}
	}
	if (n > 0)
		asmerr(E_INVOPE);
done:
	*t = '\0';
}

/*
 *	get next macro expansion line
 */
char *mac_expand(void)
{
	register struct expn *e;

	e = mac_expn;
	if (e->expn_line == NULL && !mac_rept_expn())
		return(NULL);
	/* first substitute dummies with parameter values */
	mac_subst(tmp, e->expn_line->line_text, e, mac_get_dummy);
	/* next substitute local labels with ??xxxx */
	mac_subst(line, tmp, e, mac_get_local);
	e->expn_line = e->expn_line->line_next;
	return(line);
}

/*
 *	macro lookup
 */
int mac_lookup(char *opcode)
{
	register struct mac *m;

	mac_curr = NULL;
	for (m = mac_table; m != NULL; m = m->mac_next)
		if (strncmp(m->mac_name, opcode, symlen) == 0) {
			mac_curr = m;
			return(1);
		}
	return(0);
}

/*
 *	MACRO invocation, to be called after successful mac_lookup()
 */
void mac_call(void)
{
	if (mac_curr != NULL) {
		mac_start_expn(mac_curr);
		mac_curr = NULL;
	} else
		fatal(F_INTERN, "mac_call with no macro");
}

/*
 *	get next macro parameter
 */
char *mac_next_parm(char *s)
{
	register char *t, *t1, c;
	register int n, m;
	register WORD w, v;

	t1 = t = tmp;
	n = 0;		/* angle brackets nesting level */
	while (IS_SPC(*s))
		s++;
	while (*s != '\0') {
		if (*s == STRDEL || *s == STRDEL2) {
			/* keep delimiters, but reduce double delimiters */
			*t++ = c = *s++;
			while (1) {
				if (*s == '\0') {
					asmerr(E_MISDEL);
					return(NULL);
				} else if (*s == c) {
					if (*(s + 1) != c) /* double delim? */
						break;
					else
						s++;
				}
				*t++ = *s++;
			}
			*t++ = *s++;
		} else if (*s == BYVALUE && n == 0) {
			/* pass by value */
			s++;
			t1 = t;
			while (*s != '\0' && *s != ',' && *s != COMMENT) {
				if (*s == STRDEL || *s == STRDEL2) {
					*t++ = c = *s++;
					while (1) {
						if (*s == '\0') {
							asmerr(E_MISDEL);
							return(NULL);
						} else if (*s == c) {
							*t++ = *s++;
							if (*s != c)
								break;
						}
						*t++ = *s++;
					}
				} else {
					*t++ = TO_UPP(*s);
					s++;
				}
			}
			*t = '\0';
			v = w = eval(t1);
			/* count digits in current radix */
			for (m = 1; v > radix; m++)
				v /= radix;
			if (v > 9)
				m++;
			t1 += m;
			if (t1 - tmp > MAXLINE) {
				asmerr(E_MACOVF);
				return(NULL);
			}
			/* generate digits backwards in current radix */
			for (t = t1; m > 0; m--) {
				v = w % radix;
				*--t = v + (v < 10 ? '0' : 'W');
				w /= radix;
			}
			break;
		} else if (*s == LITERAL) {
			/* literalize next character */
			s++;
			if (*s == '\0') {
				asmerr(E_INVOPE);
				return(NULL);
			} else
				*t++ = *s++;
		} else if (*s == LBRACK) {
			/* remove top level LBRACK */
			if (n > 0)
				*t++ = *s++;
			else
				s++;
			n++;
		} else if (*s == RBRACK) {
			if (n == 0) {
				asmerr(E_INVOPE);
				return(NULL);
			} else {
				/* remove top level RBRACK */
				n--;
				if (n > 0)
					*t++ = *s++;
				else
					s++;
			}
		} else if ((*s == ',' || *s == COMMENT) && n == 0)
			break;
		else
			*t++ = *s++;
		t1 = t;
		while (IS_SPC(*s))
			*t++ = *s++;
	}
	if (n > 0) {
		asmerr(E_INVOPE);
		return(NULL);
	}
	*t1 = '\0';
	return(s);
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
			e->expn_parms->parm_val = strsave(tmp);
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
		asmerr(E_INVOPE);
		return(0);
	} else {
		if ((s = mac_next_parm(s)) == NULL)
			return(0);
		e->expn_irp = s;
		if (e->expn_parms->parm_val != NULL)
			free(e->expn_parms->parm_val);
		e->expn_parms->parm_val = strsave(tmp);
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
		*e->expn_parms->parm_val = *e->expn_irp++;
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
	while (p != NULL && *s != '\0' && *s != COMMENT) {
		if ((s = mac_next_parm(s)) == NULL)
			return;
		if (*s == ',')
			s++;
		else if (*s != '\0' && *s != COMMENT) {
			asmerr(E_INVOPE);
			return;
		}
		p->parm_val = strsave(tmp);
		p = p->parm_next;
	}
}

/*
 *	start REPT macro expansion
 */
void mac_start_rept(struct expn *e)
{
	if (e->expn_mac->mac_nrept == 0)
		e->expn_line = NULL;
}

/*
 *	repeat REPT macro expansion
 */
int mac_rept_rept(struct expn *e)
{
	return(e->expn_iter < e->expn_mac->mac_nrept);
}

/*
 *	ENDM
 */
WORD op_endm(BYTE dummy1, BYTE dummy2)
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
WORD op_exitm(BYTE dummy1, BYTE dummy2)
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
WORD op_mcond(BYTE op_code, BYTE dummy)
{
	register char *s, *t = NULL;

	UNUSED(dummy);

	a_mode = A_NONE;
	if (iflevel == INT_MAX) {
		asmerr(E_IFNEST);
		return(0);
	}
	iflevel++;
	if (!gencode)
		return(0);
	switch(op_code) {
	case 1:				/* IFB */
	case 2:				/* IFNB */
		if ((s = mac_next_parm(operand)) == NULL)
			goto done;
		else if (*s != '\0' && *s != COMMENT) {
			asmerr(E_INVOPE);
			goto done;
		} else if (*tmp != '\0')
			gencode = 0;
		break;
	case 3:				/* IFIDN */
	case 4:				/* IFDIF */
		if ((s = mac_next_parm(operand)) == NULL)
			goto done;
		else if (*s++ != ',') {
			asmerr(E_MISOPE);
			goto done;
		}
		t = strsave(tmp);
		if ((s = mac_next_parm(s)) == NULL)
			goto done;
		else if ((*s != '\0' && *s != COMMENT)) {
			asmerr(E_INVOPE);
			goto done;
		} else if (strcmp(t, s) != 0)
			gencode = 0;
		break;
	default:
		fatal(F_INTERN, "invalid opcode for function op_mcond");
		break;
	}
	if ((op_code & 1) == 0)		/* negate for inverse IF */
		gencode = !gencode;
done:
	if (t != NULL)
		free(t);
	act_iflevel = iflevel;
	return(0);
}

/*
 *	IRP and IRPC
 */
WORD op_irp(BYTE op_code, BYTE dummy)
{
	register char *s, *t;
	register int i;
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
	if (!IS_FSYM(*s)) {
		asmerr(E_INVOPE);
		return(0);
	}
	*t++ = TO_UPP(*s);
	s++;
	i = 1;
	while (IS_SYM(*s)) {
		if (i++ < symlen)
			*t++ = TO_UPP(*s);
		s++;
	}
	*t = '\0';
	mac_add_dum(m, tmp);
	while (IS_SPC(*s))
		s++;
	if (*s++ != ',') {
		asmerr(E_INVOPE);
		return(0);
	}
	while (IS_SPC(*s))
		s++;
	if ((s = mac_next_parm(s)) == NULL)
		return(0);
	else if (*s != '\0' && *s != COMMENT) {
		asmerr(E_INVOPE);
		return(0);
	}
	m->mac_irp = strsave(tmp);
	mac_curr = m;
	mac_def_nest++;
	return(0);
}

/*
 *	LOCAL
 */
WORD op_local(BYTE dummy1, BYTE dummy2)
{
	register char *s, *s1, c;
	register struct expn *e;
	register struct loc *l;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	if (mac_exp_nest == 0) {
		asmerr(E_NIMEXP);
		return(0);
	}
	e = mac_expn;
	s = operand;
	while (s != NULL) {
		s1 = next_arg(s, NULL);
		if (*s != '\0') {
			if (is_symbol(s)) {
				l = expn_add_loc(e, s);
				if (mac_loc_cnt == 65535U)
					asmerr(E_OUTLCL);
				s = l->loc_val;
				*s++ = '?';
				*s++ = '?';
				c = mac_loc_cnt >> 12;
				*s++ = c + (c < 10 ? '0' : 'W');
				c = (mac_loc_cnt >> 8) & 0xf;
				*s++ = c + (c < 10 ? '0' : 'W');
				c = (mac_loc_cnt >> 4) & 0xf;
				*s++ = c + (c < 10 ? '0' : 'W');
				c = mac_loc_cnt++ & 0xf;
				*s++ = c + (c < 10 ? '0' : 'W');
				*s = '\0';
			} else
				asmerr(E_INVOPE);
		}
		s = s1;
	}
	return(0);
}

/*
 *	MACRO
 */
WORD op_macro(BYTE dummy1, BYTE dummy2)
{
	register char *s, *s1;
	register struct mac *m;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	m = mac_new(label, mac_start_macro, NULL);
	if (mac_table != NULL)
		mac_table->mac_prev = m;
	m->mac_next = mac_table;
	mac_table = m;
	mac_count++;
	s = operand;
	while (s != NULL) {
		s1 = next_arg(s, NULL);
		if (*s != '\0') {
			if (is_symbol(s))
				mac_add_dum(m, s);
			else
				asmerr(E_INVOPE);
		}
		s = s1;
	}
	mac_curr = m;
	mac_def_nest++;
	return(0);
}

/*
 *	REPT
 */
WORD op_rept(BYTE dummy1, BYTE dummy2)
{
	register struct mac *m;

	UNUSED(dummy1);
	UNUSED(dummy2);

	a_mode = A_NONE;
	m = mac_new(NULL, mac_start_rept, mac_rept_rept);
	m->mac_nrept = eval(operand);
	mac_curr = m;
	mac_def_nest++;
	return(0);
}
