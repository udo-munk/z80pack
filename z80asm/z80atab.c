/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	symbol table module
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

struct sym *look_sym(char *);
struct sym *get_sym(char *);
struct sym *new_sym(char *);
int hash(char *);
int namecmp(const void *, const void *);
int valcmp(const void *, const void *);

/* z80amain.c */
extern void fatal(int, const char *) NORETURN;

/* z80aout.c */
extern void asmerr(int);

static struct sym *symtab[HASHSIZE];	/* symbol table */
static int symcnt;			/* number of symbols defined */
static struct sym **symarray;		/* sorted symbol table */
static int symsort;			/* sort mode for iterator */
static int symidx;			/* hash table index for iterator */
static struct sym *symptr;		/* symbol pointer for iterator */

/*
 *	hash search for sym_name in symbol table symtab
 *	returns pointer to table element, or NULL if not found
 */
struct sym *look_sym(char *sym_name)
{
	register struct sym *sp;

	for (sp = symtab[hash(sym_name)]; sp != NULL; sp = sp->sym_next)
		if (strcmp(sym_name, sp->sym_name) == 0)
			return sp;
	return NULL;
}

/*
 *	hash search for sym_name in symbol table symtab
 *	set refflg when found
 *	returns pointer to table element, or NULL if not found
 */
struct sym *get_sym(char *sym_name)
{
	register struct sym *sp;

	if ((sp = look_sym(sym_name)) != NULL)
		sp->sym_refflg = TRUE;
	return sp;
}

/*
 *	add symbol sym_name to symbol table symtab
 *	returns pointer to table element
 */
struct sym *new_sym(char *sym_name)
{
	register struct sym *sp;
	register int n;
	register int hashval;

	n = strlen(sym_name);
	sp = (struct sym *) malloc(sizeof(struct sym));
	if (sp == NULL || (sp->sym_name = (char *) malloc(n + 1)) == NULL)
		fatal(F_OUTMEM, "symbols");
	strcpy(sp->sym_name, sym_name);
	hashval = hash(sym_name);
	sp->sym_next = symtab[hashval];
	symtab[hashval] = sp;
	sp->sym_refflg = FALSE;
	if (n > symmax)
		symmax = n;
	symcnt++;
	return sp;
}

/*
 *	add symbol sym_name with value sym_val to symbol table symtab,
 *	or modify existing symbol with new value and set refflg
 */
void put_sym(char *sym_name, WORD sym_val)
{
	register struct sym *sp;

	if ((sp = get_sym(sym_name)) == NULL)
		sp = new_sym(sym_name);
	sp->sym_val = sym_val;
}

/*
 *	add label to symbol table, error if symbol already exists
 *	and differs in value
 */
void put_label(void)
{
	register struct sym *sp;

	if ((sp = look_sym(label)) == NULL)
		new_sym(label)->sym_val = pc;
	else if (sp->sym_val != pc)
		asmerr(pass == 1 ? E_MULSYM : E_LBLDIF);
}

/*
 *	calculate the hash value of the string name
 *	returns hash value
 */
int hash(char *name)
{
	register unsigned h;

	for (h = 0; *name != '\0';)
		h = ((h << 5) | (h >> (sizeof(h) * 8 - 5))) ^ (BYTE) *name++;
	return h % HASHSIZE;
}

/*
 *	get first symbol for listing, sorted as specified in sort_mode
 */
struct sym *first_sym(int sort_mode)
{
	register struct sym *sp;
	register int j;
	register int i;

	if (symcnt == 0)
		return NULL;
	symsort = sort_mode;
	switch (sort_mode) {
	case SYM_UNSORT:
		symidx = 0;
		while ((symptr = symtab[symidx]) == NULL)
			if (++symidx == HASHSIZE)
				break;
		return symptr;
	case SYM_SORTN:
	case SYM_SORTA:
		symarray = (struct sym **) malloc(sizeof(struct sym *)
						  * symcnt);
		if (symarray == NULL)
			fatal(F_OUTMEM, "sorting symbol table");
		for (i = 0, j = 0; i < HASHSIZE; i++)
			for (sp = symtab[i]; sp != NULL; sp = sp->sym_next)
				symarray[j++] = sp;
		qsort(symarray, symcnt, sizeof(struct sym *),
		      sort_mode == SYM_SORTN ? namecmp : valcmp);
		symidx = 0;
		return symarray[symidx];
	default:
		fatal(F_INTERN, "unknown sort mode in first_sym");
		break;
	}
}

/*
 *	get next symbol for listing
 */
struct sym *next_sym(void)
{
	if (symsort == SYM_UNSORT) {
		if ((symptr = symptr->sym_next) == NULL) {
			do {
				if (++symidx == HASHSIZE)
					break;
				else
					symptr = symtab[symidx];
			} while (symptr == NULL);
		}
		return symptr;
	} else if (++symidx < symcnt)
		return symarray[symidx];
	return NULL;
}

/*
 *	compares two symbol names for qsort()
 */
int namecmp(const void *p1, const void *p2)
{
	return (strcmp((*(const struct sym **) p1)->sym_name,
		       (*(const struct sym **) p2)->sym_name));
}

/*
 *	compares two symbol values for qsort(), result like strcmp()
 *	if equal compares symbol names
 */
int valcmp(const void *p1, const void *p2)
{
	register WORD n1, n2;

	n1 = (*(const struct sym **) p1)->sym_val;
	n2 = (*(const struct sym **) p2)->sym_val;
	if (n1 < n2)
		return -1;
	else if (n1 > n2)
		return 1;
	else
		return namecmp(p1, p2);
}
